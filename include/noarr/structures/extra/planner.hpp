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

template <class Ending>
struct is_activated;

template <class Ending>
constexpr bool is_activated_v = is_activated<Ending>::value;

struct planner_empty_t {
	using signature = scalar_sig<void>;

	template<class NewOrder>
	[[nodiscard]]
	constexpr auto order(NewOrder) const noexcept {
		return *this;
	}

	template<class Planner>
	constexpr void operator()(Planner planner) = delete;
};

template<>
struct is_activated<planner_empty_t> : std::false_type {};

template<class Sig, class F>
struct planner_ending_elem_t : flexible_contain<F> {
	using signature = Sig;
	static constexpr bool activated = IsGroundSig<signature>;

	using flexible_contain<F>::flexible_contain;

	template<class NewOrder>
	[[nodiscard]]
	constexpr auto order(NewOrder new_order) const noexcept {
		struct sigholder_t {
			using signature = Sig;
			static_assert(!always_false<signature>);
		};

		return planner_ending_elem_t<typename decltype(sigholder_t() ^ new_order)::signature, F>(flexible_contain<F>::get());
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
		flexible_contain<F>::get()(planner.template get_struct<Idxs>()[planner.state()]...);
	}

	template<class Planner, std::size_t... Idxs>
    constexpr void run(Planner planner, std::index_sequence<Idxs...>) const noexcept
	requires (requires(F f) { f(planner.state(), planner.template get_struct<Idxs>()[planner.state()]...); })
	{
		flexible_contain<F>::get()(planner.state(), planner.template get_struct<Idxs>()[planner.state()]...);
	}
};

template<class Sig, class F>
struct is_activated<planner_ending_elem_t<Sig, F>> : std::integral_constant<bool, planner_ending_elem_t<Sig, F>::activated> {};

template<class Sig, class F>
struct planner_ending_t : flexible_contain<F> {
	using signature = Sig;
	static constexpr bool activated = IsGroundSig<signature>;

	using flexible_contain<F>::flexible_contain;

	template<class NewOrder>
	[[nodiscard]]
	constexpr auto order(NewOrder new_order) const noexcept {
		struct sigholder_t {
			using signature = Sig;
			static_assert(!always_false<signature>);
		};

		return planner_ending_t<typename decltype(sigholder_t() ^ new_order)::signature, F>(flexible_contain<F>::get());
	}

	template<class Planner>
	constexpr void operator()(Planner planner) const noexcept {
		flexible_contain<F>::get()(planner.state());
	}
};

template<class Sig, class F>
struct is_activated<planner_ending_t<Sig, F>> : std::integral_constant<bool, planner_ending_t<Sig, F>::activated> {};

template<class Sig, class F, class Next>
struct planner_sections_t : flexible_contain<F, Next> {
	using signature = Sig;
	using next = Next;
	static_assert(!(IsGroundSig<signature> && is_activated_v<Next>), "ambiguous activation of planner endings");
	static constexpr bool activated = IsGroundSig<signature> || is_activated_v<Next>;

	using flexible_contain<F, Next>::flexible_contain;

	template<class NewOrder>
	[[nodiscard]]
	constexpr auto order(NewOrder new_order) const noexcept {
		struct sigholder_t {
			using signature = Sig;
			static_assert(!always_false<signature>);
		};

		return planner_sections_t<typename decltype(sigholder_t() ^ new_order)::signature, F, std::remove_cvref_t<decltype(this->template get<1>().order(new_order))>>(flexible_contain<F, Next>::template get<0>(), flexible_contain<F, Next>::template get<1>().order(new_order));
	}

	template<class Order>
	[[nodiscard]]
	constexpr decltype(auto) get_next(Order order) const noexcept requires is_activated_v<decltype(this->order(order))> {
		if constexpr(IsGroundSig<typename decltype(this->order(order))::signature>)
			return flexible_contain<F, Next>::template get<1>();
		else
			return planner_sections_t<Sig, F, std::remove_cvref_t<decltype(this->template get<1>().get_next(order))>>(flexible_contain<F, Next>::template get<0>(),flexible_contain<F, Next>::template get<1>().get_next(order));
	}

	template<class Planner>
	constexpr void operator()(Planner planner) const noexcept {
		if constexpr (IsGroundSig<typename decltype(this->order(fix(state_at<typename Planner::union_struct>(planner.top_struct(), empty_state))))::signature>)
			flexible_contain<F, Next>::template get<0>()(planner.pop_ending());
		else
			flexible_contain<F, Next>::template get<1>()(planner);
	}
};

template<class Sig, class F, class Next>
struct is_activated<planner_sections_t<Sig, F, Next>> : std::integral_constant<bool, planner_sections_t<Sig, F, Next>::activated> {};

template<class Union, class Order, class Ending>
struct planner_t;

template<class... Structs, class Order, class Ending>
struct planner_t<union_t<Structs...>, Order, Ending> : flexible_contain<union_t<Structs...>, Order, Ending> {
	using union_struct = union_t<Structs...>;
	using base = flexible_contain<union_struct, Order, Ending>;
	using base::base;

	static constexpr std::size_t num_structs = sizeof...(Structs);

	template<class Union, class Order_, class Ending_>
	[[nodiscard]]
	constexpr planner_t(Union &&union_struct, Order_ &&order, Ending_ &&ending)
		: base(std::forward<Union>(union_struct), std::forward<Order_>(order), std::forward<Ending_>(ending)) {}

	[[nodiscard]]
	constexpr union_struct get_union() const noexcept { return base::template get<0>(); }

	[[nodiscard]]
	constexpr Order get_order() const noexcept { return base::template get<1>(); }

	[[nodiscard]]
	constexpr Ending get_ending() const noexcept { return base::template get<2>(); }

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
		return planner_t<union_struct, Order, std::remove_cvref_t<decltype(get_ending().get_next(fix(state_at<union_struct>(top_struct(), empty_state))))>>(get_union(), get_order(), get_ending().get_next(fix(state_at<union_struct>(top_struct(), empty_state))));
	}

	// TODO: multiple for_sections
	template<auto... Dims, class F> requires (... && IsDim<decltype(Dims)>)
	[[nodiscard("returns a new planner")]]
	constexpr auto for_sections(F f) const noexcept {
		using union_sig = typename decltype(get_union())::signature;
		struct sigholder_t {
			using signature = union_sig;
			static_assert(!always_false<signature>); // suppresses warnings
		};

		using signature = typename decltype(sigholder_t() ^ reorder<Dims...>())::signature;

		return planner_t<union_struct, Order, planner_sections_t<signature, F, Ending>>(get_union(), get_order(), planner_sections_t<signature, F, Ending>(f, get_ending()));
	}

	constexpr void execute() const noexcept requires (!std::same_as<Ending, planner_empty_t>) {
		using dim_tree = sig_dim_tree<typename decltype(top_struct())::signature>;
		for_each_impl(dim_tree(), empty_state);
	}

	constexpr void operator()() const noexcept requires (!std::same_as<Ending, planner_empty_t>) {
		execute();
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
	template<auto Dim, class ...Branches, IsState State>
	constexpr void for_each_impl(dim_tree<Dim, Branches...>, State state) const noexcept {
		if constexpr (is_activated_v<decltype(get_ending().order(fix(state_at<union_struct>(top_struct(), state))))>) {
			get_ending()(order(fix(state)));
		} else {
			using dim_sig = sig_find_dim<Dim, State, typename decltype(top_struct())::signature>;
			if constexpr(dim_sig::dependent) {
				for_each_impl_dep<Dim, Branches...>(state, std::index_sequence_for<Branches...>());
			} else {
				std::size_t len = top_struct().template length<Dim>(state);
				for(std::size_t i = 0; i < len; i++)
					for_each_impl(Branches()..., state.template with<index_in<Dim>>(i));
			}
		}
	}
	template<IsState State>
	constexpr void for_each_impl(dim_sequence<>, State state) const noexcept {
		static_assert(is_activated_v<decltype(get_ending().order(fix(state_at<union_struct>(top_struct(), state))))>);
		get_ending()(order(fix(state)));
	}
};

template<class Union, class Order, class Ending>
planner_t(Union &&, Order &&, Ending &&) -> planner_t<std::remove_cvref_t<Union>, std::remove_cvref_t<Order>, std::remove_cvref_t<Ending>>;

template<class... Ts>
[[nodiscard("returns a new planner")]]
constexpr auto planner(const Ts &... s) noexcept
{ return planner(make_union(s.get_ref()...)); }

template<class... Ts>
constexpr planner_t<union_t<Ts...>, neutral_proto, planner_empty_t> planner(union_t<Ts...> u) noexcept { return planner_t<union_t<Ts...>, neutral_proto, planner_empty_t>(u, neutral_proto(), planner_empty_t()); }

} // namespace noarr

#endif // NOARR_STRUCTURES_PLANNER_HPP
