#ifndef NOARR_TUNING_HPP
#define NOARR_TUNING_HPP

#include <cstdlib>
#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>

#include "formatter.hpp"

namespace noarr::tuning {

template<class ...>
struct name_holder {};

struct begin_t {};
constexpr begin_t begin;

struct end_t {};
constexpr end_t end;

struct choice_t {};
constexpr choice_t choice;

struct constant_t {};
constexpr constant_t constant;

struct permutation_t {};
constexpr permutation_t permutation;

struct mapped_permutation_t {};
constexpr mapped_permutation_t mapped_permutation;

struct range_t {};
constexpr range_t range;

struct mapped_range_t {};
constexpr mapped_range_t mapped_range;

template<class ValueType>
concept IsDefined = requires{ ValueType::value; };

template<class Name, class Parameter, class ...Things>
struct interpret;

template<auto ...Values>
constexpr auto collect_values() {
	return std::tuple<std::integral_constant<decltype(Values), Values>...>();
}

template<class Name, class Parameter, class ...Things>
interpret(name_holder<Name>, Parameter, Things&&...) -> interpret<Name, Parameter, Things...>;

template<class Name, class T>
interpret(name_holder<Name>, range_t, T &&end) -> interpret<Name, range_t, std::integral_constant<std::remove_cvref_t<T>, 0>, std::remove_cvref_t<T>, std::integral_constant<std::remove_cvref_t<T>, 1>>;

template<class Name, class T, T Value>
interpret(name_holder<Name>, range_t, const std::integral_constant<T, Value> &end) -> interpret<Name, range_t, std::integral_constant<T, 0>, std::integral_constant<T, Value>, std::integral_constant<T, 1>>;

template<class Name, class Start, class End>
interpret(name_holder<Name>, range_t, Start &&, End &&) -> interpret<Name, range_t, std::remove_cvref_t<Start>, std::remove_cvref_t<End>, Start>;

template<class Name, class Start, Start Value, class End>
interpret(name_holder<Name>, range_t, const std::integral_constant<Start, Value> &, End &&) -> interpret<Name, range_t, std::integral_constant<Start, Value>, std::remove_cvref_t<End>, Start>;

template<class Name, class Start, class End, class Step>
interpret(name_holder<Name>, range_t, Start &&, End &&, Step &&) -> interpret<Name, range_t, std::remove_cvref_t<Start>, std::remove_cvref_t<End>, std::remove_cvref_t<Step>>;

template<class Name, class Map, class T>
interpret(name_holder<Name>, mapped_range_t, Map &&, T &&end) -> interpret<Name, mapped_range_t, std::remove_cvref_t<Map>, std::integral_constant<std::remove_cvref_t<T>, 0>, std::remove_cvref_t<T>, std::integral_constant<std::remove_cvref_t<T>, 1>>;

template<class Name, class Map, class T, T Value>
interpret(name_holder<Name>, mapped_range_t, Map &&, const std::integral_constant<T, Value> &end) -> interpret<Name, mapped_range_t, std::remove_cvref_t<Map>, std::integral_constant<T, 0>, std::integral_constant<T, Value>, std::integral_constant<T, 1>>;

template<class Name, class Map, class Start, class End>
interpret(name_holder<Name>, mapped_range_t, Map &&, Start &&, End &&) -> interpret<Name, mapped_range_t, std::remove_cvref_t<Map>, std::remove_cvref_t<Start>, std::remove_cvref_t<End>, Start>;

template<class Name, class Map, class Start, Start Value, class End>
interpret(name_holder<Name>, mapped_range_t, Map &&, const std::integral_constant<Start, Value> &, End &&) -> interpret<Name, mapped_range_t, std::remove_cvref_t<Map>, std::integral_constant<Start, Value>, std::remove_cvref_t<End>, Start>;

template<class Name, class Map, class Start, class End, class Step>
interpret(name_holder<Name>, mapped_range_t, Map &&, Start &&, End &&, Step &&) -> interpret<Name, mapped_range_t, std::remove_cvref_t<Map>, std::remove_cvref_t<Start>, std::remove_cvref_t<End>, std::remove_cvref_t<Step>>;

#ifdef NOARR_TUNE

template<class Name, class ...Args> requires (!IsDefined<Name>)
struct interpret<Name, begin_t, Args...> : std::tuple<Args...> {
	using name = Name;

	template<class ...Ts>
	constexpr interpret(name_holder<Name>, begin_t, Ts &&...args) noexcept
		: std::tuple<Args...>(std::forward<Ts>(args)...) {}

	template<class TunerFormatter>
	constexpr void generate(TunerFormatter &formatter) const {
		generate(formatter, std::index_sequence_for<Args...>());
	}

private:
	template<class TunerFormatter, std::size_t ...Is>
	constexpr void generate(TunerFormatter &formatter, std::index_sequence<Is...>) const {
		formatter.header(std::get<Is>(*this)...);
	}
};

template<class Name, class ...Args> requires (!IsDefined<Name>)
struct interpret<Name, end_t, Args...> : std::tuple<Args...> {
	using name = Name;

	template<class ...Ts>
	constexpr interpret(name_holder<Name>, end_t, Ts &&...args) noexcept
		: std::tuple<Args...>(std::forward<Ts>(args)...) {}

	template<class TunerFormatter>
	constexpr void generate(TunerFormatter &formatter) const {
		generate(formatter, std::index_sequence_for<Args...>());
	}

private:
	template<class TunerFormatter, std::size_t ...Is>
	constexpr void generate(TunerFormatter &formatter, std::index_sequence<Is...>) const {
		formatter.footer(std::get<Is>(*this)...);
		std::exit(0);
	}
};

template<class Name, class ...Choices> requires (!IsDefined<Name>)
struct interpret<Name, choice_t, Choices...> : std::tuple<Choices...>  {
	using name = Name;

	template<class ...Ts>
	constexpr interpret(name_holder<Name>, choice_t, Ts &&...choices)
		: std::tuple<Choices...>(std::forward<Ts>(choices)...) {}

	constexpr decltype(auto) operator*() const noexcept {
		return std::get<0>(*this);
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
struct interpret<Name, range_t, Start, End, Step> : std::tuple<Start> {
	using name = Name;

	template<class T>
	constexpr interpret(name_holder<Name>, range_t, T &&end)
		: std::tuple<Start>(init_value<Start>(0)), range_(init_value<Start>(0), std::forward<T>(end), init_value<Step>(1)) {}

	template<class Start_, class End_, class Step_ = Step>
	constexpr interpret(name_holder<Name>, range_t, Start_ &&begin, End_ &&end, Step_ &&step = init_value<Step>(1))
		: std::tuple<Start>(std::forward<Start_>(begin)), range_(std::forward<Start_>(begin), std::forward<End_>(end), std::forward<Step_>(step)) {}

	constexpr decltype(auto) operator*() const noexcept {
		return std::get<0>(*this);
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

template<class Name, class Map, class Start, class End, class Step> requires (!IsDefined<Name>)
struct interpret<Name, mapped_range_t, Map, Start, End, Step> : std::tuple<Map, Start> {
	using name = Name;

	template<class Map_, class End_> requires std::same_as<std::remove_cvref_t<Map_>, Map>
	constexpr interpret(name_holder<Name>, mapped_range_t, Map_ &&map, End_ &&end)
		: std::tuple<Map, Start>(std::forward<Map_>(map), init_value<Start>(0)), range_(init_value<Start>(0), std::forward<End_>(end), init_value<Step>(1)) {}

	template<class Map_, class End_> requires std::default_initializable<Map> && (!std::same_as<std::remove_cvref_t<Map_>, Map>)
	constexpr interpret(name_holder<Name>, mapped_range_t, Map_ &&, End_ &&end)
		: std::tuple<Map, Start>(Map{}, init_value<Start>(0)), range_(init_value<Start>(0), std::forward<End_>(end), init_value<Step>(1)) {}

	template<class Map_, class Start_, class End_, class Step_ = Step> requires std::same_as<std::remove_cvref_t<Map_>, Map>
	constexpr interpret(name_holder<Name>, mapped_range_t, Map_ &&map, Start_ &&begin, End_ &&end, Step_ &&step = init_value<Step>(1))
		: std::tuple<Map, Start>(std::forward<Map_>(map), std::forward<Start_>(begin)), range_(std::forward<Start_>(begin), std::forward<End_>(end), std::forward<Step_>(step)) {}
	
	template<class Map_, class Start_, class End_, class Step_ = Step> requires std::default_initializable<Map> && (!std::same_as<std::remove_cvref_t<Map_>, Map>)
	constexpr interpret(name_holder<Name>, mapped_range_t, Map_ &&, Start_ &&begin, End_ &&end, Step_ &&step = init_value<Step>(1))
		: std::tuple<Map, Start>(Map{}, std::forward<Start_>(begin)), range_(std::forward<Start_>(begin), std::forward<End_>(end), std::forward<Step_>(step)) {}

	constexpr decltype(auto) operator*() const {
		return std::get<0>(*this)(std::get<1>(*this));
	}

	template<class TunerFormatter>
	constexpr decltype(auto) generate(TunerFormatter &formatter) const {
		return formatter.format(Name::name, range_);
	}

	template<class TunerFormatter, class Constraint>
	constexpr decltype(auto) generate(TunerFormatter &formatter, Constraint &constraint) const {
		return formatter.format(Name::name, range_, constraint);
	}

private:
	range_parameter<Start, End, Step> range_;
};

template<class Name, class ...Choices> requires (!IsDefined<Name>)
struct interpret<Name, permutation_t, Choices...> : std::tuple<Choices...>  {
	using name = Name;

	template<class ...Ts>
	constexpr interpret(name_holder<Name>, permutation_t, Ts &&...choices)
		: std::tuple<Choices...>(std::forward<Ts>(choices)...) {}

	constexpr const std::tuple<Choices...> &operator*() const noexcept {
		return *this;
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
struct interpret<Name, mapped_permutation_t, Map, Choices...> : std::tuple<Map, Choices...>  {
	using name = Name;

	template<class T, class ...Ts> requires std::same_as<std::remove_cvref_t<T>, Map>
	constexpr interpret(name_holder<Name>, mapped_permutation_t, T &&map, Ts &&...choices)
		: std::tuple<Map, Choices...>(std::forward<T>(map), std::forward<Ts>(choices)...) {}

	template<class T, class ...Ts> requires std::default_initializable<Map> && (!std::same_as<std::remove_cvref_t<T>, Map>)
	constexpr interpret(name_holder<Name>, mapped_permutation_t, T &&, Ts &&...choices)
		: std::tuple<Map, Choices...>(Map{}, std::forward<Ts>(choices)...) {}

	constexpr decltype(auto) operator*() const {
		return map(std::index_sequence_for<Choices...>());
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
		return std::get<0>(*this)(std::get<Is + 1>(*this)...);
	}

	static constexpr auto permutation = permutation_parameter(sizeof...(Choices));
};

#else

template<class Name, class ...Args> requires (!IsDefined<Name>)
struct interpret<Name, begin_t, Args...> : std::tuple<> {
	using name = Name;

	template<class ...Ts>
	constexpr interpret(name_holder<Name>, begin_t, Ts &&...) noexcept : std::tuple<>() {}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept {}
};

template<class Name, class ...Args> requires (!IsDefined<Name>)
struct interpret<Name, end_t, Args...> : std::tuple<> {
	using name = Name;

	template<class ...Ts>
	constexpr interpret(name_holder<Name>, end_t, Ts &&...) noexcept : std::tuple<>() {}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept {}
};

template<class Name, class Choice, class ...Choices> requires (!IsDefined<Name>)
struct interpret<Name, choice_t, Choice, Choices...> : std::tuple<Choice> {
	using name = Name;

	template<class T, class ...Ts>
	constexpr interpret(name_holder<Name>, choice_t, T &&choice, Ts &&...)
		: std::tuple<Choice>(std::forward<T>(choice)) {}

	constexpr decltype(auto) operator*() const noexcept {
		return std::get<0>(*this);
	}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept {}
};

template<class Name, class Start, class End, class Step> requires (!IsDefined<Name>)
struct interpret<Name, range_t, Start, End, Step> : std::tuple<Start> {
	using name = Name;

	template<class T>
	constexpr interpret(name_holder<Name>, range_t, T &&)
		: std::tuple<Start>(Start{}) {}

	template<class Start_, class End_, class Step_ = Step>
	constexpr interpret(name_holder<Name>, range_t, Start_ &&begin, End_ &&, Step_ && = (Step)1)
		: std::tuple<Start>(std::forward<Start_>(begin)) {}

	constexpr decltype(auto) operator*() const noexcept {
		return std::get<0>(*this);
	}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept {}
};

template<class Name, class Map, class Start, class End, class Step> requires (!IsDefined<Name>)
struct interpret<Name, mapped_range_t, Map, Start, End, Step> : std::tuple<Map, Start> {
	using name = Name;

	template<class Map_, class End_> requires std::same_as<std::remove_cvref_t<Map_>, Map>
	constexpr interpret(name_holder<Name>, mapped_range_t, Map_ &&map, End_ &&)
		: std::tuple<Map, Start>(std::forward<Map_>(map), Start{}) {}

	template<class Map_, class End_> requires std::default_initializable<Map> && (!std::same_as<std::remove_cvref_t<Map_>, Map>)
	constexpr interpret(name_holder<Name>, mapped_range_t, Map_ &&, End_ &&)
		: std::tuple<Map, Start>(Map{}, Start{}) {}

	template<class Map_, class Start_, class End_, class Step_ = Step> requires std::same_as<std::remove_cvref_t<Map_>, Map>
	constexpr interpret(name_holder<Name>, mapped_range_t, Map_ &&map, Start_ &&begin, End_ &&, Step_ && = (Step)1)
		: std::tuple<Map, Start>(std::forward<Map_>(map), std::forward<Start_>(begin)) {}

	template<class Map_, class Start_, class End_, class Step_ = Step> requires std::default_initializable<Map> && (!std::same_as<std::remove_cvref_t<Map_>, Map>)
	constexpr interpret(name_holder<Name>, mapped_range_t, Map_ &&, Start_ &&begin, End_ &&, Step_ && = (Step)1)
		: std::tuple<Map, Start>(Map{}, std::forward<Start_>(begin)) {}

	constexpr decltype(auto) operator*() const {
		return std::get<0>(*this)(std::get<1>(*this));
	}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept {}
};

template<class Name, class ...Choices> requires (!IsDefined<Name>)
struct interpret<Name, permutation_t, Choices...> : std::tuple<Choices...>  {
	using name = Name;

	template<class ...Ts>
	constexpr interpret(name_holder<Name>, permutation_t, Ts &&...choices)
		: std::tuple<Choices...>(std::forward<Ts>(choices)...) {}

	constexpr const std::tuple<Choices...> &operator*() const noexcept {
		return *this;
	}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept {}
};

template<class Name, class Map, class ...Choices> requires (!IsDefined<Name>)
struct interpret<Name, mapped_permutation_t, Map, Choices...> : std::tuple<Map, Choices...>  {
	using name = Name;

	template<class T, class ...Ts> requires std::same_as<std::remove_cvref_t<T>, Map>
	constexpr interpret(name_holder<Name>, mapped_permutation_t, T &&map, Ts &&...choices)
		: std::tuple<Map, Choices...>(std::forward<T>(map), std::forward<Ts>(choices)...) {}

	template<class T, class ...Ts> requires std::default_initializable<Map> && (!std::same_as<std::remove_cvref_t<T>, Map>)
	constexpr interpret(name_holder<Name>, mapped_permutation_t, T &&, Ts &&...choices)
		: std::tuple<Map, Choices...>(Map{}, std::forward<Ts>(choices)...) {}

	constexpr decltype(auto) operator*() const {
		return map(std::index_sequence_for<Choices...>());
	}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept {}

private:
	template<std::size_t ...Is>
	constexpr decltype(auto) map(std::index_sequence<Is...>) const {
		return std::get<0>(*this)(std::get<Is + 1>(*this)...);
	}
};

#endif // NOARR_TUNE

template<class Name, class ...Choices> requires (IsDefined<Name>)
struct interpret<Name, choice_t, Choices...> : std::tuple<Choices...>  {
	using name = Name;

	template<class ...Ts>
	constexpr interpret(name_holder<Name>, choice_t, Ts &&...choices)
		: std::tuple<Choices...>(std::forward<Ts>(choices)...) {}

	constexpr decltype(auto) operator*() const noexcept {
		return std::get<std::get<0>(Name::value)>(*this);
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
		return std::get<0>(Name::value);
	}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept { }
};

template<class Name, class Map, class Start, class End, class Step> requires (IsDefined<Name>)
struct interpret<Name, mapped_range_t, Map, Start, End, Step> : std::tuple<Map> {
	using name = Name;

	template<class Map_, class End_> requires std::same_as<std::remove_cvref_t<Map_>, Map>
	constexpr interpret(name_holder<Name>, mapped_range_t, Map_ &&map, End_ &&)
		: std::tuple<Map>(std::forward<Map_>(map)) {}

	template<class Map_, class End_> requires std::default_initializable<Map> && (!std::same_as<std::remove_cvref_t<Map_>, Map>)
	constexpr interpret(name_holder<Name>, mapped_range_t, Map_ &&, End_ &&)
		: std::tuple<Map>(Map{}) {}

	template<class Map_, class Start_, class End_, class Step_ = Step> requires std::same_as<std::remove_cvref_t<Map_>, Map>
	constexpr interpret(name_holder<Name>, mapped_range_t, Map_ &&map, Start_ &&begin, End_ &&, Step_ && = (Step)1)
		: std::tuple<Map>(std::forward<Map_>(map)) {}
	
	template<class Map_, class Start_, class End_, class Step_ = Step> requires std::default_initializable<Map> && (!std::same_as<std::remove_cvref_t<Map_>, Map>)
	constexpr interpret(name_holder<Name>, mapped_range_t, Map_ &&, Start_ &&begin, End_ &&, Step_ && = (Step)1)
		: std::tuple<Map>(Map{}) {}

	constexpr decltype(auto) operator*() const {
		return std::get<0>(*this)(std::get<0>(Name::value));
	}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept {}
};

template<class Name, class ...Choices> requires (IsDefined<Name>)
struct interpret<Name, permutation_t, Choices...> : std::tuple<Choices...>  {
	using name = Name;

	template<class ...Ts>
	constexpr interpret(name_holder<Name>, permutation_t, Ts &&...choices)
		: std::tuple<Choices...>(std::forward<Ts>(choices)...) {}

	constexpr const std::tuple<Choices...> &operator*() const noexcept {
		return *this;
	}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept { }
};

template<class Name, class Map, class ...Choices> requires (IsDefined<Name>)
struct interpret<Name, mapped_permutation_t, Map, Choices...> : std::tuple<Map, Choices...>  {
	using name = Name;

	template<class T, class ...Ts> requires std::same_as<std::remove_cvref_t<T>, Map>
	constexpr interpret(name_holder<Name>, mapped_permutation_t, T &&map, Ts &&...choices)
		: std::tuple<Map, Choices...>(std::forward<T>(map), std::forward<Ts>(choices)...) {}

	template<class T, class ...Ts> requires std::default_initializable<Map> && (!std::same_as<std::remove_cvref_t<T>, Map>)
	constexpr interpret(name_holder<Name>, mapped_permutation_t, T &&, Ts &&...choices)
		: std::tuple<Map, Choices...>(Map{}, std::forward<Ts>(choices)...) {}

	constexpr decltype(auto) operator*() const {
		return map(std::index_sequence_for<Choices...>());
	}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept {}

private:
	template<std::size_t ...Is>
	constexpr decltype(auto) map(std::index_sequence<Is...>) const {
		return std::get<0>(*this)(std::get<Is + 1>(*this)...);
	}
};

template<class Name, class Value>
struct interpret<Name, constant_t, Value> : std::tuple<Value>  {
	using name = Name;

	template<class T>
	constexpr interpret(name_holder<Name>, constant_t, T &&value)
		: std::tuple<Value>(std::forward<T>(value)) {}

	constexpr decltype(auto) operator*() const noexcept {
		return std::get<0>(*this);
	}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept { }
};


template<class Formatter, class Parameter, class Constraint>
class definition_t {
	struct empty_t {};
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
	struct empty_t {};
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

#endif // NOARR_TUNING_HPP
