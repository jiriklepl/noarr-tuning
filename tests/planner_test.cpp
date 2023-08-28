#include <catch2/catch_test_macros.hpp>

#include <noarr/structures/extra/planner.hpp>
#include <noarr/structures/interop/bag.hpp>
#include <noarr/structures_extended.hpp>

using namespace noarr;

TEST_CASE("Planner trivial", "[planner]") {
	using at = noarr::array_t<'x', 20, noarr::array_t<'y', 30, noarr::scalar<int>>>;
	using bt = noarr::array_t<'y', 30, noarr::array_t<'z', 40, noarr::scalar<int>>>;
	using ct = noarr::array_t<'x', 20, noarr::array_t<'z', 40, noarr::scalar<int>>>;

	using xt = noarr::array_t<'z', 40, noarr::array_t<'x', 20, noarr::array_t<'y', 30, noarr::scalar<int>>>>;

	using u1t = union_t<at>;
	using u2t = union_t<at, bt>;
	using u3t = union_t<at, bt, ct>;

	auto a = make_bag(at());
	auto b = make_bag(bt());
	auto c = make_bag(ct());

	u1t u1;
	u2t u2;
	u3t u3;

	REQUIRE(u1.length<'x'>(state<>()) == 20);
	REQUIRE(u1.length<'y'>(state<>()) == 30);

	REQUIRE(u2.length<'x'>(state<>()) == 20);
	REQUIRE(u2.length<'y'>(state<>()) == 30);
	REQUIRE(u2.length<'z'>(state<>()) == 40);

	REQUIRE(u3.length<'x'>(state<>()) == 20);
	REQUIRE(u3.length<'y'>(state<>()) == 30);
	REQUIRE(u3.length<'z'>(state<>()) == 40);

	REQUIRE(std::is_same_v<u1t::signature, at::signature>);
	REQUIRE(std::is_same_v<u2t::signature, xt::signature>);
	REQUIRE(std::is_same_v<u3t::signature, xt::signature>);

	int i = 0;

	auto a_plan = planner(a).for_each_elem([&i](auto state, auto &&a) {
		auto [x, y] = get_indices<'x', 'y'>(state);
		a = x == y;
	});

	auto b_plan = planner(b).for_each_elem([&i](auto &&b) {
		b = 1;
	});

	auto c_plan = planner(c).for_each_elem([&i](auto &&c) {
		c = 0;
	});

	auto abc_plan = planner(a, b, c).for_each_elem([&i](auto &&a, auto &&b, auto &&c) {
		c += a * b;
		i++;
	}).template for_sections<'x', 'z'>([](auto inner) {
		auto z = get_index<'z'>(inner.state());
		auto x = get_index<'x'>(inner.state());

		REQUIRE(z < 40);
		REQUIRE(x < 20);

		inner();
	}).order(reorder<'x', 'z', 'y'>());

	auto c_check_plan = planner(c).for_each_elem([&i](auto &&c) {
		REQUIRE(c == 1);
	}).order(hoist<'z'>());

	REQUIRE(i == 0);

	a_plan();
	b_plan();
	c_plan();
	abc_plan();
	c_check_plan();

	REQUIRE(i == 20*30*40);
}
