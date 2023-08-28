#ifndef NOARR_STRUCTURES_PLANNER_HPP
#define NOARR_STRUCTURES_PLANNER_HPP

#include "../base/contain.hpp"
#include "../base/signature.hpp"
#include "../base/state.hpp"
#include "../base/structs_common.hpp"
#include "../base/utility.hpp"
#include "../structs/views.hpp"
#include "../structs/setters.hpp"
#include "../extra/sig_utils.hpp"
#include "../extra/traverser.hpp"

namespace noarr {

struct planner_empty_t {
	using signature = scalar_sig<void>;

	template<class NewOrder>
	constexpr auto order(NewOrder) const noexcept {
		return *this;
	}

	template<class Planner>
	constexpr void operator()(Planner planner) = delete;
};

template<class Sig, class F>
struct planner_ending_elem_t {
	using signature = Sig;

	constexpr planner_ending_elem_t(F f) : f_(f) {}

	template<class NewOrder>
	constexpr auto order(NewOrder new_order) const noexcept {
		struct sigholder_t {
			using signature = Sig;
			static_assert(!always_false<signature>);
		};

		return planner_ending_elem_t<typename decltype(sigholder_t() ^ new_order)::signature, F>(f_);
	}

	template<class Planner>
	constexpr void operator()(Planner planner) const noexcept {
		run(planner, std::make_index_sequence<Planner::num_structs>());
	}

private:
	template<class Planner, std::size_t... Idxs>
    constexpr void run(Planner planner, std::index_sequence<Idxs...>) const noexcept
	requires (requires(F f) { f(planner.template get_struct<Idxs>()[planner.state()]...); })
	{
		f_(planner.template get_struct<Idxs>()[planner.state()]...);
	}

	template<class Planner, std::size_t... Idxs>
    constexpr void run(Planner planner, std::index_sequence<Idxs...>) const noexcept
	requires (requires(F f) { f(planner.state(), planner.template get_struct<Idxs>()[planner.state()]...); })
	{
		f_(planner.state(), planner.template get_struct<Idxs>()[planner.state()]...);
	}

	F f_;
};

template<class Sig, class F>
struct planner_ending_t {
	using signature = Sig;

	constexpr planner_ending_t(F f) : f_(f) {}

	template<class NewOrder>
	constexpr auto order(NewOrder new_order) const noexcept {
		struct sigholder_t {
			using signature = Sig;
			static_assert(!always_false<signature>);
		};

		return planner_ending_t<typename decltype(sigholder_t() ^ new_order)::signature, F>(f_);
	}

	template<class Planner>
	constexpr void operator()(Planner planner) const noexcept {
		f_(planner.state());
	}

private:
	F f_;
};

template<class Sig, class F, class Next>
struct planner_sections_t {
	using signature = Sig;
	using next = Next;


	constexpr planner_sections_t(F f, Next next) : f_(f), next_(next) {}

	template<class NewOrder>
	constexpr auto order(NewOrder new_order) const noexcept {
		struct sigholder_t {
			using signature = Sig;
			static_assert(!always_false<signature>);
		};

		return planner_sections_t<typename decltype(sigholder_t() ^ new_order)::signature, F, decltype(next_.order(new_order))>(f_, next_.order(new_order));
	}

	constexpr const next &get_next() const noexcept { return next_; }

	template<class Planner>
	constexpr void operator()(Planner planner) const noexcept {
		f_(planner.pop_ending());
	}

private:
	F f_;
	Next next_;
};

template<class Union, class Order, class Ending>
struct planner_t;

template<class... Structs, class Order, class Ending>
struct planner_t<union_t<Structs...>, Order, Ending> : contain<union_t<Structs...>, Order> {
	Ending ending_;

	using union_struct = union_t<Structs...>;
	using base = contain<union_struct, Order>;
	using base::base;

	static constexpr std::size_t num_structs = sizeof...(Structs);

	[[nodiscard]]
	constexpr planner_t(union_struct union_struct, Order order, Ending ending) : base(union_struct, order), ending_(ending) {}

	[[nodiscard]]
	constexpr union_struct get_union() const noexcept { return base::template get<0>(); }

	[[nodiscard]]
	constexpr Order get_order() const noexcept { return base::template get<1>(); }

	[[nodiscard]]
	constexpr Ending get_ending() const noexcept { return ending_; }

	template <std::size_t Idx> requires (Idx < num_structs)
	[[nodiscard]]
	constexpr auto get_struct() const noexcept { return get_union().template get<Idx>(); }

	template<class NewOrder>
	[[nodiscard("returns a new planner")]]
	constexpr auto order(NewOrder new_order) const noexcept {
		return planner_t<union_struct, decltype(get_order() ^ new_order), Ending>(get_union(), get_order() ^ new_order, get_ending());
	}

	// TODO: for_each after for_sections
	template<class F> requires (std::same_as<Ending, planner_empty_t>)
	[[nodiscard("returns a new planner")]]
	constexpr auto for_each(F f) const noexcept {
		using signature = typename decltype(get_union())::signature;
		return planner_t<union_struct, Order, planner_ending_t<signature, F>>(get_union(), get_order(), planner_ending_t<signature, F>(f));
	}

	template<class F> requires (std::same_as<Ending, planner_empty_t>)
	[[nodiscard("returns a new planner")]]
	constexpr auto for_each_elem(F f) const noexcept {
		using signature = typename decltype(get_union())::signature;
		return planner_t<union_struct, Order, planner_ending_elem_t<signature, F>>(get_union(), get_order(), planner_ending_elem_t<signature, F>(f));
	}

	[[nodiscard("returns a new planner")]]
	constexpr auto pop_ending() const noexcept {
		return planner_t<union_struct, Order, typename Ending::next>(get_union(), get_order(), get_ending().get_next());
	}

	// TODO: multiple for_sections
	template<auto... Dims, class F> requires (... && IsDim<decltype(Dims)>)
	[[nodiscard("returns a new planner")]]
	constexpr auto for_sections(F f) const noexcept {
		using union_sig = typename decltype(get_union())::signature;
		struct sigholder_t {
			using signature = union_sig;
			static_assert(!always_false<signature>);
		};

		using signature = typename decltype(sigholder_t() ^ reorder<Dims...>())::signature;

		return planner_t<union_struct, Order, planner_sections_t<signature, F, Ending>>(get_union(), get_order(), planner_sections_t<signature, F, Ending>(f, get_ending()));
	}

	constexpr void operator()() const noexcept requires (!std::same_as<Ending, planner_empty_t>) {
		using dim_tree = sig_dim_tree<typename decltype(top_struct())::signature>;
		for_each_impl(dim_tree(), empty_state);
	}

	[[nodiscard("returns the state of the planner")]]
	constexpr auto state() const noexcept {
		return state_at<union_struct>(top_struct(), empty_state);
	}

	[[nodiscard("returns the top struct of the planner")]]
	constexpr auto top_struct() const noexcept {
		return get_union() ^ get_order();
	}

private:
	template<auto Dim, class Branch, class ...Branches, std::size_t I, std::size_t... Is>
	constexpr void for_each_impl_dep(auto state, std::index_sequence<I, Is...>) const noexcept {
		for_each_impl(Branch(), state.template with<index_in<Dim>>(std::integral_constant<std::size_t, I>()));
		for_each_impl_dep<Dim, Branches...>(state, std::index_sequence<Is...>());
	}
	template<auto Dim, class F>
	constexpr void for_each_impl_dep(F, auto, std::index_sequence<>) const noexcept {}
	template<class DimTree, IsState State>
	constexpr void for_each_impl(DimTree, State state) const noexcept
	requires (IsGroundSig<typename decltype(get_ending().order(fix(order(fix(state)).state())))::signature>) {
		get_ending()(order(fix(state))); // TODO: maybe run ending on reordered planner
	}
	template<auto Dim, class ...Branches, IsState State>
	constexpr void for_each_impl(dim_tree<Dim, Branches...>, State state) const noexcept
	requires (!IsGroundSig<typename decltype(get_ending().order(fix(order(fix(state)).state())))::signature>) {
		using dim_sig = sig_find_dim<Dim, State, typename decltype(top_struct())::signature>;
		if constexpr(dim_sig::dependent) {
			for_each_impl_dep<Dim, Branches...>(state, std::index_sequence_for<Branches...>());
		} else {
			std::size_t len = top_struct().template length<Dim>(state);
			for(std::size_t i = 0; i < len; i++)
				for_each_impl(Branches()..., state.template with<index_in<Dim>>(i));
		}
	}
};

template<class... Ts>
[[nodiscard("returns a new planner")]]
constexpr auto planner(const Ts &... s) noexcept 
{ return planner(make_union(s.get_ref()...)); }

template<class... Ts>
constexpr planner_t<union_t<Ts...>, neutral_proto, planner_empty_t> planner(union_t<Ts...> u) noexcept { return planner_t<union_t<Ts...>, neutral_proto, planner_empty_t>(u, neutral_proto(), planner_empty_t()); }

} // namespace noarr

#endif // NOARR_STRUCTURES_PLANNER_HPP
