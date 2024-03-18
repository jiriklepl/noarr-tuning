#ifndef NOARR_STRUCTURES_TUNING_HPP
#define NOARR_STRUCTURES_TUNING_HPP

#include <cstdlib>
#include <type_traits>
#include <utility>

#include "../base/structs_common.hpp"
#include "../tuning/formatter.hpp"

namespace noarr::tuning {

template<class ...>
struct name_holder {};

struct begin_t {};
inline constexpr begin_t begin;

struct end_t {};
inline constexpr end_t end;

struct choice_t {};
inline constexpr choice_t choice;

struct constant_t {};
inline constexpr constant_t constant;

struct permutation_t {};
inline constexpr permutation_t permutation;

struct mapped_permutation_t {};
inline constexpr mapped_permutation_t mapped_permutation;

struct range_t {};
inline constexpr range_t range;

template<class ValueType>
concept IsDefined = requires{ ValueType::value; };

template<class Name, class Parameter, class ...Things>
struct interpret;

template<class Name, class Parameter, class ...Things>
interpret(name_holder<Name>, Parameter, Things&&...) -> interpret<Name, Parameter, Things...>;

template<class Name, class T>
interpret(name_holder<Name>, range_t, T &&end) -> interpret<Name, range_t, std::integral_constant<std::remove_cvref_t<T>, 0>, std::remove_cvref_t<T>, std::integral_constant<std::remove_cvref_t<T>, 1>>;

template<class Name, class T, T Value>
interpret(name_holder<Name>, range_t, const std::integral_constant<T, Value> &end) -> interpret<Name, range_t, std::integral_constant<T, 0>, std::integral_constant<T, Value>, std::integral_constant<T, 1>>;

template<class Name, std::size_t Value>
interpret(name_holder<Name>, range_t, const noarr::lit_t<Value> &end) -> interpret<Name, range_t, noarr::lit_t<0>, noarr::lit_t<Value>, noarr::lit_t<1>>;

template<class Name, class Start, class End>
interpret(name_holder<Name>, range_t, Start &&, End &&) -> interpret<Name, range_t, std::remove_cvref_t<Start>, std::remove_cvref_t<End>, Start>;

template<class Name, class Start, Start Value, class End>
interpret(name_holder<Name>, range_t, const std::integral_constant<Start, Value> &, End &&) -> interpret<Name, range_t, std::integral_constant<Start, Value>, std::remove_cvref_t<End>, Start>;

template<class Name, std::size_t Value, class End>
interpret(name_holder<Name>, range_t, const noarr::lit_t<Value> &, End &&) -> interpret<Name, range_t, noarr::lit_t<Value>, std::remove_cvref_t<End>, std::size_t>;

template<class Name, class Start, class End, class Step>
interpret(name_holder<Name>, range_t, Start &&, End &&, Step &&) -> interpret<Name, range_t, std::remove_cvref_t<Start>, std::remove_cvref_t<End>, std::remove_cvref_t<Step>>;

#ifdef NOARR_TUNE

template<class Name, class ...Args> requires (!IsDefined<Name>)
struct interpret<Name, begin_t, Args...> : flexible_contain<Args...> {
	using name = Name;

	template<class ...Ts>
	constexpr interpret(name_holder<Name>, begin_t, Ts &&...args) noexcept
		: flexible_contain<Args...>(std::forward<Ts>(args)...) {}

	template<class TunerFormatter>
	constexpr void generate(TunerFormatter &formatter) const {
		generate(formatter, std::index_sequence_for<Args...>());
	}

private:
	template<class TunerFormatter, std::size_t ...Is>
	constexpr void generate(TunerFormatter &formatter, std::index_sequence<Is...>) const {
		formatter.header(this->template get<Is>()...);
	}
};

template<class Name, class ...Args> requires (!IsDefined<Name>)
struct interpret<Name, end_t, Args...> : flexible_contain<Args...> {
	using name = Name;

	template<class ...Ts>
	constexpr interpret(name_holder<Name>, end_t, Ts &&...args) noexcept
		: flexible_contain<Args...>(std::forward<Ts>(args)...) {}

	template<class TunerFormatter>
	constexpr void generate(TunerFormatter &formatter) const {
		generate(formatter, std::index_sequence_for<Args...>());
	}

private:
	template<class TunerFormatter, std::size_t ...Is>
	constexpr void generate(TunerFormatter &formatter, std::index_sequence<Is...>) const {
		formatter.footer(this->template get<Is>()...);
		std::exit(0);
	}
};

template<class Name, class ...Choices> requires (!IsDefined<Name>)
struct interpret<Name, choice_t, Choices...> : flexible_contain<Choices...>  {
	using name = Name;

	template<class ...Ts>
	constexpr interpret(name_holder<Name>, choice_t, Ts &&...choices)
		: flexible_contain<Choices...>(std::forward<Ts>(choices)...) {}

	constexpr decltype(auto) operator*() const noexcept {
		return this->template get<0>();
	}

	constexpr decltype(auto) operator->() const noexcept {
		return &**this;
	}

	template<class TunerFormatter>
	constexpr decltype(auto) generate(TunerFormatter &formatter) const {
		return formatter.format(Name::name, categories);
	}

	template<class TunerFormatter, class Constraint>
	constexpr decltype(auto) generate(TunerFormatter &formatter, Constraint &&constraint) const {
		return formatter.format(Name::name, categories, std::forward<Constraint>(constraint));
	}

private:
	static constexpr auto categories = category_parameter(sizeof...(Choices));
};

template<class T>
constexpr T init_value(auto &&value) requires requires() { T{std::forward<decltype(value)>(value)}; } {
	return {std::forward<decltype(value)>(value)};
}

template<class T>
constexpr T init_value(auto &&value) {
	return {};
}

template<class Name, class Start, class End, class Step> requires (!IsDefined<Name>)
struct interpret<Name, range_t, Start, End, Step> : flexible_contain<Start> {
	using name = Name;

	template<class T>
	constexpr interpret(name_holder<Name>, range_t, T &&end)
		: flexible_contain<Start>(init_value<Start>(0)), range_(init_value<Start>(0), std::forward<T>(end), init_value<Step>(1)) {}

	template<class Start_, class End_, class Step_ = Step>
	constexpr interpret(name_holder<Name>, range_t, Start_ &&begin, End_ &&end, Step_ &&step = init_value<Step>(1))
		: flexible_contain<Start>(std::forward<Start_>(begin)), range_(std::forward<Start_>(begin), std::forward<End_>(end), std::forward<Step_>(step)) {}

	constexpr decltype(auto) operator*() const noexcept {
		return this->template get<0>();
	}

	constexpr decltype(auto) operator->() const noexcept {
		return &**this;
	}

	template<class TunerFormatter>	
	constexpr decltype(auto) generate(TunerFormatter &formatter) const {
		return formatter.format(Name::name, range_);
	}

	template<class TunerFormatter, class Constraint>
	constexpr decltype(auto) generate(TunerFormatter &formatter, Constraint &&constraint) const {
		return formatter.format(Name::name, range_, std::forward<Constraint>(constraint));
	}

private:
	range_parameter<Start, End, Step> range_;
};

template<class Name, class ...Choices> requires (!IsDefined<Name>)
struct interpret<Name, permutation_t, Choices...> : flexible_contain<Choices...>  {
	using name = Name;

	template<class ...Ts>
	constexpr interpret(name_holder<Name>, permutation_t, Ts &&...choices)
		: flexible_contain<Choices...>(std::forward<Ts>(choices)...) {}

	constexpr const flexible_contain<Choices...> &operator*() const noexcept {
		return *this;
	}

	constexpr decltype(auto) operator->() const noexcept {
		return &**this;
	}

	template<class TunerFormatter>
	constexpr decltype(auto) generate(TunerFormatter &formatter) const {
		return formatter.format(Name::name, permutation);
	}

	template<class TunerFormatter, class Constraint>
	constexpr decltype(auto) generate(TunerFormatter &formatter, Constraint &constraint) const {
		return formatter.format(Name::name, permutation, constraint);
	}

private:
	static constexpr auto permutation = permutation_parameter(sizeof...(Choices));
};

template<class Name, class Map, class ...Choices> requires (!IsDefined<Name>)
struct interpret<Name, mapped_permutation_t, Map, Choices...> : flexible_contain<Map, Choices...>  {
	using name = Name;

	template<class T, class ...Ts>
	constexpr interpret(name_holder<Name>, mapped_permutation_t, T &&map, Ts &&...choices)
		: flexible_contain<Map, Choices...>(std::forward<T>(map), std::forward<Ts>(choices)...) {}

	constexpr auto operator*() const {
		return map(std::index_sequence_for<Choices...>());
	}
	
	constexpr decltype(auto) operator->() const noexcept {
		return &**this;
	}

	template<class TunerFormatter>
	constexpr decltype(auto) generate(TunerFormatter &formatter) const {
		return formatter.format(Name::name, permutation);
	}

	template<class TunerFormatter, class Constraint>
	constexpr decltype(auto) generate(TunerFormatter &formatter, Constraint &constraint) const {
		return formatter.format(Name::name, permutation, constraint);
	}

private:
	template<std::size_t ...Is>
	constexpr decltype(auto) map(std::index_sequence<Is...>) const {
		return this->template get<0>()(this->template get<Is + 1>()...);
	}

	static constexpr auto permutation = permutation_parameter(sizeof...(Choices));
};

#else

template<class Name, class ...Args> requires (!IsDefined<Name>)
struct interpret<Name, begin_t, Args...> : flexible_contain<> {
	using name = Name;

	template<class ...Ts>
	constexpr interpret(name_holder<Name>, begin_t, Ts &&...) noexcept : flexible_contain<>() {}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept {}
};

template<class Name, class ...Args> requires (!IsDefined<Name>)
struct interpret<Name, end_t, Args...> : flexible_contain<> {
	using name = Name;

	template<class ...Ts>
	constexpr interpret(name_holder<Name>, end_t, Ts &&...) noexcept : flexible_contain<>() {}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept {}
};

template<class Name, class Choice, class ...Choices> requires (!IsDefined<Name>)
struct interpret<Name, choice_t, Choice, Choices...> : flexible_contain<Choice> {
	using name = Name;

	template<class T, class ...Ts>
	constexpr interpret(name_holder<Name>, choice_t, T &&choice, Ts &&...)
		: flexible_contain<Choice>(std::forward<T>(choice)) {}

	constexpr decltype(auto) operator*() const noexcept {
		return this->get();
	}

	constexpr decltype(auto) operator->() const noexcept {
		return &**this;
	}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept {}
};

template<class Name, class Start, class End, class Step> requires (!IsDefined<Name>)
struct interpret<Name, range_t, Start, End, Step> : flexible_contain<Start> {
	using name = Name;

	template<class T>
	constexpr interpret(name_holder<Name>, range_t, T &&)
		: flexible_contain<Start>(Start{}) {}

	template<class Start_, class End_, class Step_ = Step>
	constexpr interpret(name_holder<Name>, range_t, Start_ &&begin, End_ &&, Step_ && = (Step)1)
		: flexible_contain<Start>(std::forward<Start_>(begin)) {}

	constexpr decltype(auto) operator*() const noexcept {
		return this->get();
	}

	constexpr decltype(auto) operator->() const noexcept {
		return &**this;
	}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept {}
};

template<class Name, class ...Choices> requires (!IsDefined<Name>)
struct interpret<Name, permutation_t, Choices...> : flexible_contain<Choices...>  {
	using name = Name;

	template<class ...Ts>
	constexpr interpret(name_holder<Name>, permutation_t, Ts &&...choices)
		: flexible_contain<Choices...>(std::forward<Ts>(choices)...) {}

	constexpr const flexible_contain<Choices...> &operator*() const noexcept {
		return *this;
	}

	constexpr decltype(auto) operator->() const noexcept {
		return &**this;
	}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept {}
};

template<class Name, class Map, class ...Choices> requires (!IsDefined<Name>)
struct interpret<Name, mapped_permutation_t, Map, Choices...> : flexible_contain<Map, Choices...>  {
	using name = Name;

	template<class T, class ...Ts>
	constexpr interpret(name_holder<Name>, mapped_permutation_t, T &&map, Ts &&...choices)
		: flexible_contain<Map, Choices...>(std::forward<T>(map), std::forward<Ts>(choices)...) {}

	constexpr auto operator*() const {
		return map(std::index_sequence_for<Choices...>());
	}
	
	constexpr decltype(auto) operator->() const noexcept {
		return &**this;
	}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept {}

private:
	template<std::size_t ...Is>
	constexpr decltype(auto) map(std::index_sequence<Is...>) const {
		return this->template get<0>()(this->template get<Is + 1>()...);
	}
};

#endif // NOARR_TUNE

template<class Name, class ...Choices> requires (IsDefined<Name>)
struct interpret<Name, choice_t, Choices...> : flexible_contain<Choices...>  {
	using name = Name;

	template<class ...Ts>
	constexpr interpret(name_holder<Name>, choice_t, Ts &&...choices)
		: flexible_contain<Choices...>(std::forward<Ts>(choices)...) {}

	constexpr decltype(auto) operator*() const noexcept {
		return this->template get<Name::value.template get<0>()>();
	}

	constexpr decltype(auto) operator->() const noexcept {
		return &**this;
	}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept { }
};

template<class Name, class Start, class End, class Step> requires (IsDefined<Name>)
struct interpret<Name, range_t, Start, End, Step> {
	using name = Name;

	template<class T>
	constexpr interpret(name_holder<Name>, range_t, T &&)
	{}

	template<class Start_, class End_, class Step_>
	constexpr interpret(name_holder<Name>, range_t, Start_ &&, End_ &&, Step_ && = (Step)1)
	{}

	constexpr decltype(auto) operator*() const noexcept {
		return Name::value.template get<0>();
	}

	constexpr decltype(auto) operator->() const noexcept {
		return &**this;
	}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept { }
};

template<class Name, class ...Choices> requires (IsDefined<Name>)
struct interpret<Name, permutation_t, Choices...> : flexible_contain<Choices...>  {
	using name = Name;

	template<class ...Ts>
	constexpr interpret(name_holder<Name>, permutation_t, Ts &&...choices)
		: flexible_contain<Choices...>(std::forward<Ts>(choices)...) {}

	constexpr const flexible_contain<Choices...> &operator*() const noexcept {
		return *this;
	}

	constexpr decltype(auto) operator->() const noexcept {
		return &**this;
	}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept { }
};

template<class Name, class Map, class ...Choices> requires (IsDefined<Name>)
struct interpret<Name, mapped_permutation_t, Map, Choices...> : flexible_contain<Map, Choices...>  {
	using name = Name;

	template<class T, class ...Ts>
	constexpr interpret(name_holder<Name>, mapped_permutation_t, T &&map, Ts &&...choices)
		: flexible_contain<Map, Choices...>(std::forward<T>(map), std::forward<Ts>(choices)...) {}

	constexpr auto operator*() const {
		return map(std::index_sequence_for<Choices...>());
	}
	
	constexpr decltype(auto) operator->() const noexcept {
		return &**this;
	}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept {}

private:
	template<std::size_t ...Is>
	constexpr decltype(auto) map(std::index_sequence<Is...>) const {
		return this->template get<0>()(this->template get<Is + 1>()...);
	}
};

template<class Name, class Value>
struct interpret<Name, constant_t, Value> : flexible_contain<Value>  {
	using name = Name;

	template<class T>
	constexpr interpret(name_holder<Name>, constant_t, T &&value)
		: flexible_contain<Value>(std::forward<T>(value)) {}

	constexpr decltype(auto) operator*() const noexcept {
		return this->template get<0>();
	}

	constexpr decltype(auto) operator->() const noexcept {
		return &**this;
	}

	template<class TunerFormatter, class ...Ts>
	constexpr void generate(TunerFormatter &formatter, Ts &&...) const noexcept {
		formatter.format(Name::name, categories);
	}

private:
	static constexpr auto categories = category_parameter(1);
};


template<class Formatter, class Parameter, class Constraint>
class definition_t {
public:
	using return_type = decltype(std::declval<Parameter>().generate((Formatter&)(std::declval<Formatter>()), (Constraint&)(std::declval<Constraint>())));
	using value_type = std::conditional_t<!std::is_same_v<return_type, void>, return_type, empty_t> ;

	constexpr definition_t(Formatter &formatter, Parameter &parameter, Constraint &&constraint) requires (!std::is_same_v<return_type, void>)
		: value_(parameter.generate(formatter, std::forward<Constraint>(constraint))) {}

	constexpr definition_t(Formatter &formatter, Parameter &parameter, Constraint &&constraint) requires (std::is_same_v<return_type, void>) {
		parameter.generate(formatter, std::forward<Constraint>(constraint));
	}

	constexpr const value_type &operator*() const noexcept {
		return value_;
	}

	constexpr value_type &operator*() noexcept {
		return value_;
	}

private:
	value_type value_;
};

template<class Formatter, class Parameter>
class definition_t<Formatter, Parameter, void> {
public:
	using return_type = std::remove_reference_t<decltype(std::declval<Parameter>().generate((Formatter&)(std::declval<Formatter>())))>;
	using value_type = std::conditional_t<!std::is_same_v<return_type, void>, return_type, empty_t> ;

	constexpr definition_t(Formatter &formatter, Parameter &parameter) requires (!std::is_same_v<return_type, void>)
		: value_(parameter.generate(formatter)) {}
	
	constexpr definition_t(Formatter &formatter, Parameter &parameter) requires (std::is_same_v<return_type, void>) {
		parameter.generate(formatter);
	}

	constexpr const value_type &operator*() const noexcept {
		return value_;
	}

	constexpr value_type &operator*() noexcept {
		return value_;
	}

private:
	value_type value_;
};

template<class Formatter, class Parameter, class Constraint>
definition_t(Formatter &, Parameter &, Constraint &&) -> definition_t<Formatter, Parameter, Constraint>;

template<class Formatter, class Parameter>
definition_t(Formatter &, Parameter &) -> definition_t<Formatter, Parameter, void>;

} // namespace noarr::tuning

#endif // NOARR_STRUCTURES_TUNING_HPP
