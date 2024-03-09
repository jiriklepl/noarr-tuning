#ifndef NOARR_STRUCTURES_SLICE_HPP
#define NOARR_STRUCTURES_SLICE_HPP

#include <cstddef>
#include <type_traits>
#include <utility>

#include "../base/contain.hpp"
#include "../base/signature.hpp"
#include "../base/state.hpp"
#include "../base/structs_common.hpp"
#include "../base/utility.hpp"

namespace noarr {

template<IsDim auto Dim, class T, class StartT>
struct shift_t : strict_contain<T, StartT> {
	using strict_contain<T, StartT>::strict_contain;

	static constexpr char name[] = "shift_t";
	using params = struct_params<
		dim_param<Dim>,
		structure_param<T>,
		type_param<StartT>>;

	constexpr T sub_structure() const noexcept { return this->template get<0>(); }
	constexpr StartT start() const noexcept { return this->template get<1>(); }

private:
	template<class Original>
	struct dim_replacement;
	template<class ArgLength, class RetSig>
	struct dim_replacement<function_sig<Dim, ArgLength, RetSig>> {
		template<class L, class S>
		struct subtract { using type = dynamic_arg_length; };
		template<class S>
		struct subtract<unknown_arg_length, S> { using type = unknown_arg_length; };
		template<std::size_t L, std::size_t S>
		struct subtract<static_arg_length<L>, std::integral_constant<std::size_t, S>> { using type = static_arg_length<L-S>; };
		using type = function_sig<Dim, typename subtract<ArgLength, StartT>::type, RetSig>;
	};
	template<class ...RetSigs>
	struct dim_replacement<dep_function_sig<Dim, RetSigs...>> {
		using original = dep_function_sig<Dim, RetSigs...>;
		static_assert(StartT::value || true, "Cannot shift a tuple dimension dynamically");
		static constexpr std::size_t start = StartT::value;
		static constexpr std::size_t len = sizeof...(RetSigs) - start;

		template<class Indices = std::make_index_sequence<len>>
		struct pack_helper;
		template<std::size_t ...Indices>
		struct pack_helper<std::index_sequence<Indices...>> { using type = dep_function_sig<Dim, typename original::template ret_sig<Indices+start>...>; };

		using type = typename pack_helper<>::type;
	};
public:
	using signature = typename T::signature::template replace<dim_replacement, Dim>;

	template<IsState State>
	constexpr auto sub_state(State state) const noexcept {
		using namespace constexpr_arithmetic;
		const auto tmp_state = state.template remove<index_in<Dim>, length_in<Dim>>();
		if constexpr(State::template contains<index_in<Dim>>)
			if constexpr(State::template contains<length_in<Dim>>)
				return tmp_state.template with<index_in<Dim>, length_in<Dim>>(state.template get<index_in<Dim>>() + start(), state.template get<length_in<Dim>>() + start());
			else
				return tmp_state.template with<index_in<Dim>>(state.template get<index_in<Dim>>() + start());
		else
			if constexpr(State::template contains<length_in<Dim>>)
				return tmp_state.template with<length_in<Dim>>(state.template get<length_in<Dim>>() + start());
			else
				return tmp_state;
	}

	constexpr auto size(IsState auto state) const noexcept {
		return sub_structure().size(sub_state(state));
	}

	template<class Sub>
	constexpr auto strict_offset_of(IsState auto state) const noexcept {
		return offset_of<Sub>(sub_structure(), sub_state(state));
	}

	template<IsDim auto QDim, IsState State> requires (QDim != Dim || HasNotSetIndex<State, QDim>)
	constexpr auto length(State state) const noexcept {
		using namespace constexpr_arithmetic;
		if constexpr(QDim == Dim) {
			if constexpr(State::template contains<length_in<Dim>>) {
				return state.template get<length_in<Dim>>();
			} else {
				return sub_structure().template length<Dim>(sub_state(state)) - start();
			}
		} else {
			return sub_structure().template length<QDim>(sub_state(state));
		}
	}

	template<class Sub>
	constexpr auto strict_state_at(IsState auto state) const noexcept {
		return state_at<Sub>(sub_structure(), sub_state(state));
	}
};

template<IsDim auto Dim, class StartT>
struct shift_proto : strict_contain<StartT> {
	using strict_contain<StartT>::strict_contain;

	static constexpr bool proto_preserves_layout = true;

	template<class Struct>
	constexpr auto instantiate_and_construct(Struct s) const noexcept { return shift_t<Dim, Struct, StartT>(s, this->get()); }
};

/**
 * @brief shifts an index (or indices) given by dimension name(s) in a structure
 *
 * @tparam Dim: the dimension names
 * @param start: parameters for shifting the indices
 */
template<auto ...Dims, class ...StartT> requires IsDimPack<decltype(Dims)...>
constexpr auto shift(StartT ...start) noexcept { return (... ^ shift_proto<Dims, good_index_t<StartT>>(start)); }

template<>
constexpr auto shift<>() noexcept { return neutral_proto(); }

template<IsDim auto Dim, class T, class StartT, class LenT>
struct slice_t : strict_contain<T, StartT, LenT> {
	using strict_contain<T, StartT, LenT>::strict_contain;

	static constexpr char name[] = "slice_t";
	using params = struct_params<
		dim_param<Dim>,
		structure_param<T>,
		type_param<StartT>,
		type_param<LenT>>;

	constexpr T sub_structure() const noexcept { return this->template get<0>(); }
	constexpr StartT start() const noexcept { return this->template get<1>(); }
	constexpr LenT len() const noexcept { return this->template get<2>(); }

private:
	template<class Original>
	struct dim_replacement;
	template<class ArgLength, class RetSig>
	struct dim_replacement<function_sig<Dim, ArgLength, RetSig>> { using type = function_sig<Dim, arg_length_from_t<LenT>, RetSig>; };
	template<class ...RetSigs>
	struct dim_replacement<dep_function_sig<Dim, RetSigs...>> {
		using original = dep_function_sig<Dim, RetSigs...>;
		static_assert(StartT::value || true, "Cannot slice a tuple dimension dynamically");
		static_assert(LenT::value || true, "Cannot slice a tuple dimension dynamically");
		static constexpr std::size_t start = StartT::value;
		static constexpr std::size_t len = LenT::value;

		template<class Indices = std::make_index_sequence<len>>
		struct pack_helper;
		template<std::size_t ...Indices>
		struct pack_helper<std::index_sequence<Indices...>> { using type = dep_function_sig<Dim, typename original::template ret_sig<Indices+start>...>; };

		using type = typename pack_helper<>::type;
	};
public:
	using signature = typename T::signature::template replace<dim_replacement, Dim>;
	template<IsState State>
	constexpr auto sub_state(State state) const noexcept {
		using namespace constexpr_arithmetic;
		if constexpr(State::template contains<index_in<Dim>>)
			return state.template with<index_in<Dim>>(state.template get<index_in<Dim>>() + start());
		else
			return state;
	}
	template<IsState State>
	constexpr auto size(State state) const noexcept {
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set slice length");
		return sub_structure().size(sub_state(state));
	}

	template<class Sub, IsState State>
	constexpr auto strict_offset_of(State state) const noexcept {
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set slice length");
		return offset_of<Sub>(sub_structure(), sub_state(state));
	}

	template<IsDim auto QDim, IsState State> requires (QDim != Dim || HasNotSetIndex<State, QDim>)
	constexpr auto length(State state) const noexcept {
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set slice length");
		if constexpr(QDim == Dim) {
			return len();
		} else {
			return sub_structure().template length<QDim>(sub_state(state));
		}
	}

	template<class Sub, IsState State>
	constexpr auto strict_state_at(State state) const noexcept {
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set slice length");
		return state_at<Sub>(sub_structure(), sub_state(state));
	}
};

template<IsDim auto Dim, class StartT, class LenT>
struct slice_proto : strict_contain<StartT, LenT> {
	using strict_contain<StartT, LenT>::strict_contain;

	static constexpr bool proto_preserves_layout = true;

	template<class Struct>
	constexpr auto instantiate_and_construct(Struct s) const noexcept { return slice_t<Dim, Struct, StartT, LenT>(s, this->template get<0>(), this->template get<1>()); }
};

template<IsDim auto Dim, class StartT, class LenT>
constexpr auto slice(StartT start, LenT len) noexcept { return slice_proto<Dim, good_index_t<StartT>, good_index_t<LenT>>(start, len); }

// TODO add tests
template<IsDim auto Dim, class T, class StartT, class EndT>
struct span_t : strict_contain<T, StartT, EndT> {
	using strict_contain<T, StartT, EndT>::strict_contain;

	static constexpr char name[] = "span_t";
	using params = struct_params<
		dim_param<Dim>,
		structure_param<T>,
		type_param<StartT>,
		type_param<EndT>>;

	constexpr T sub_structure() const noexcept { return this->template get<0>(); }
	constexpr StartT start() const noexcept { return this->template get<1>(); }
	constexpr EndT end() const noexcept { return this->template get<2>(); }

private:
	template<class Original>
	struct dim_replacement;
	template<class ArgLength, class RetSig>
	struct dim_replacement<function_sig<Dim, ArgLength, RetSig>> { using type = function_sig<Dim, arg_length_from_t<EndT>, RetSig>; };
	template<class ...RetSigs>
	struct dim_replacement<dep_function_sig<Dim, RetSigs...>> {
		using original = dep_function_sig<Dim, RetSigs...>;
		static_assert(StartT::value || true, "Cannot span a tuple dimension dynamically");
		static_assert(EndT::value || true, "Cannot span a tuple dimension dynamically");
		static constexpr std::size_t start = StartT::value;
		static constexpr std::size_t end = EndT::value;

		template<class Indices = std::make_index_sequence<end - start>>
		struct pack_helper;
		template<std::size_t ...Indices>
		struct pack_helper<std::index_sequence<Indices...>> { using type = dep_function_sig<Dim, typename original::template ret_sig<Indices+start>...>; };

		using type = typename pack_helper<>::type;
	};
public:
	using signature = typename T::signature::template replace<dim_replacement, Dim>;

	template<IsState State>
	constexpr auto sub_state(State state) const noexcept {
		using namespace constexpr_arithmetic;
		if constexpr(State::template contains<index_in<Dim>>)
			return state.template with<index_in<Dim>>(state.template get<index_in<Dim>>() + start());
		else
			return state;
	}

	template<IsState State>
	constexpr auto size(State state) const noexcept {
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set span length");
		return sub_structure().size(sub_state(state));
	}

	template<class Sub, IsState State>
	constexpr auto strict_offset_of(State state) const noexcept {
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set span length");
		return offset_of<Sub>(sub_structure(), sub_state(state));
	}

	template<IsDim auto QDim, IsState State> requires (QDim != Dim || HasNotSetIndex<State, QDim>)
	constexpr auto length(State state) const noexcept {
		using namespace constexpr_arithmetic;
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set span length");
		if constexpr(QDim == Dim) {
			return end() - start();
		} else {
			return sub_structure().template length<QDim>(sub_state(state));
		}
	}

	template<class Sub, IsState State>
	constexpr auto strict_state_at(State state) const noexcept {
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set span length");
		return state_at<Sub>(sub_structure(), sub_state(state));
	}
};

template<IsDim auto Dim, class StartT, class EndT>
struct span_proto : strict_contain<StartT, EndT> {
	using strict_contain<StartT, EndT>::strict_contain;

	static constexpr bool proto_preserves_layout = true;

	template<class Struct>
	constexpr auto instantiate_and_construct(Struct s) const noexcept { return span_t<Dim, Struct, StartT, EndT>(s, this->template get<0>(), this->template get<1>()); }
};

template<IsDim auto Dim, class StartT, class EndT>
constexpr auto span(StartT start, EndT end) noexcept { return span_proto<Dim, good_index_t<StartT>, good_index_t<EndT>>(start, end); }

template<IsDim auto Dim, class T, class StartT, class StrideT>
struct step_t : strict_contain<T, StartT, StrideT> {
	using strict_contain<T, StartT, StrideT>::strict_contain;

	static constexpr char name[] = "step_t";
	using params = struct_params<
		dim_param<Dim>,
		structure_param<T>,
		type_param<StartT>,
		type_param<StrideT>>;

	constexpr T sub_structure() const noexcept { return this->template get<0>(); }
	constexpr StartT start() const noexcept { return this->template get<1>(); }
	constexpr StrideT stride() const noexcept { return this->template get<2>(); }

private:
	template<class Original>
	struct dim_replacement;
	template<class ArgLength, class RetSig>
	struct dim_replacement<function_sig<Dim, ArgLength, RetSig>> { using type = function_sig<Dim, arg_length_from_t<StrideT>, RetSig>; };
	template<class ...RetSigs>
	struct dim_replacement<dep_function_sig<Dim, RetSigs...>> {
		using original = dep_function_sig<Dim, RetSigs...>;
		static_assert(StartT::value || true, "Cannot slice a tuple dimension dynamically");
		static_assert(StrideT::value || true, "Cannot slice a tuple dimension dynamically");
		static constexpr std::size_t start = StartT::value;
		static constexpr std::size_t stride = StrideT::value;
		static constexpr std::size_t sub_length = sizeof...(RetSigs);

		template<class Indices = std::make_index_sequence<(sub_length + stride - start - 1) / stride>>
		struct pack_helper;
		template<std::size_t ...Indices>
		struct pack_helper<std::index_sequence<Indices...>> { using type = dep_function_sig<Dim, typename original::template ret_sig<Indices*stride+start>...>; };

		using type = typename pack_helper<>::type;
	};
public:
	using signature = typename T::signature::template replace<dim_replacement, Dim>;

	template<IsState State>
	constexpr auto sub_state(State state) const noexcept {
		using namespace constexpr_arithmetic;
		if constexpr(State::template contains<index_in<Dim>>)
			return state.template with<index_in<Dim>>(state.template get<index_in<Dim>>() * stride() + start());
		else
			return state;
	}

	template<IsState State>
	constexpr auto size(State state) const noexcept {
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set length after step");
		return sub_structure().size(sub_state(state));
	}

	template<class Sub, IsState State>
	constexpr auto strict_offset_of(State state) const noexcept {
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set length after step");
		return offset_of<Sub>(sub_structure(), sub_state(state));
	}

	template<IsDim auto QDim, IsState State> requires (QDim != Dim || HasNotSetIndex<State, QDim>)
	constexpr auto length(State state) const noexcept {
		using namespace constexpr_arithmetic;
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set length after step");
		if constexpr(QDim == Dim) {
			const auto sub_length = sub_structure().template length<Dim>(state);
			return (sub_length + stride() - start() - make_const<1>()) / stride();
		} else {
			return sub_structure().template length<QDim>(sub_state(state));
		}
	}

	template<class Sub, IsState State>
	constexpr auto strict_state_at(State state) const noexcept {
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set length after step");
		return state_at<Sub>(sub_structure(), sub_state(state));
	}
};

template<IsDim auto Dim, class StartT, class StrideT>
struct step_proto : strict_contain<StartT, StrideT> {
	using strict_contain<StartT, StrideT>::strict_contain;

	static constexpr bool proto_preserves_layout = true;

	template<class Struct>
	constexpr auto instantiate_and_construct(Struct s) const noexcept { return step_t<Dim, Struct, StartT, StrideT>(s, this->template get<0>(), this->template get<1>()); }
};

template<IsDim auto Dim, class StartT, class StrideT>
constexpr auto step(StartT start, StrideT stride) noexcept { return step_proto<Dim, good_index_t<StartT>, good_index_t<StrideT>>(start, stride); }

template<class StartT, class StrideT>
struct auto_step_proto : strict_contain<StartT, StrideT> {
	using strict_contain<StartT, StrideT>::strict_contain;

	static constexpr bool proto_preserves_layout = true;

	template<class Struct>
	constexpr auto instantiate_and_construct(Struct s) const noexcept {
		static_assert(!Struct::signature::dependent, "Add a dimension name as the first parameter to step, or use a structure with a dynamic topmost dimension");
		constexpr auto dim = Struct::signature::dim;
		return step_t<dim, Struct, StartT, StrideT>(s, this->template get<0>(), this->template get<1>());
	}
};

template<class StartT, class StrideT>
constexpr auto step(StartT start, StrideT stride) noexcept { return auto_step_proto<good_index_t<StartT>, good_index_t<StrideT>>(start, stride); }

template<IsDim auto Dim, class T>
struct reverse_t : strict_contain<T> {
	using strict_contain<T>::strict_contain;

	static constexpr char name[] = "reverse_t";
	using params = struct_params<
		dim_param<Dim>,
		structure_param<T>>;

	constexpr T sub_structure() const noexcept { return this->get(); }

private:
	template<class Original>
	struct dim_replacement {
		using type = Original;
	};
	template<class ...RetSigs>
	struct dim_replacement<dep_function_sig<Dim, RetSigs...>> {
		using original = dep_function_sig<Dim, RetSigs...>;
		static constexpr std::size_t len = sizeof...(RetSigs);

		template<class Indices = std::make_index_sequence<len>>
		struct pack_helper;
		template<std::size_t ...Indices>
		struct pack_helper<std::index_sequence<Indices...>> { using type = dep_function_sig<Dim, typename original::template ret_sig<len-1-Indices>...>; };

		using type = typename pack_helper<>::type;
	};
public:
	using signature = typename T::signature::template replace<dim_replacement, Dim>;

	template<IsState State>
	constexpr auto sub_state(State state) const noexcept {
		using namespace constexpr_arithmetic;
		if constexpr(State::template contains<index_in<Dim>>) {
			const auto tmp_state = state.template remove<index_in<Dim>>();
			return tmp_state.template with<index_in<Dim>>(sub_structure().template length<Dim>(tmp_state) - make_const<1>() - state.template get<index_in<Dim>>());
		} else {
			return state;
		}
	}

	constexpr auto size(IsState auto state) const noexcept {
		return sub_structure().size(sub_state(state));
	}

	template<class Sub>
	constexpr auto strict_offset_of(IsState auto state) const noexcept {
		return offset_of<Sub>(sub_structure(), sub_state(state));
	}

	template<IsDim auto QDim, IsState State> requires (QDim != Dim || HasNotSetIndex<State, QDim>)
	constexpr auto length(State state) const noexcept {
		return sub_structure().template length<QDim>(state);
	}

	template<class Sub>
	constexpr auto strict_state_at(IsState auto state) const noexcept {
		return state_at<Sub>(sub_structure(), sub_state(state));
	}
};

template<IsDim auto Dim>
struct reverse_proto {
	static constexpr bool proto_preserves_layout = true;

	template<class Struct>
	constexpr auto instantiate_and_construct(Struct s) const noexcept { return reverse_t<Dim, Struct>(s); }
};

/**
 * @brief reverses an index (or indices) given by dimension name(s) in a structure
 *
 * @tparam Dim: the dimension names
 */
template<auto ...Dims> requires IsDimPack<decltype(Dims)...>
constexpr auto reverse() noexcept { return (... ^ reverse_proto<Dims>()); }

template<>
constexpr auto reverse<>() noexcept { return neutral_proto(); }

} // namespace noarr

#endif // NOARR_STRUCTURES_SLICE_HPP
