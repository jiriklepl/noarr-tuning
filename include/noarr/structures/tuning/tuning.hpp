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

struct value_t {};
inline constexpr value_t value;

struct permutation_t {};
inline constexpr permutation_t permutation;

struct range_t {};
inline constexpr range_t range;

template<class ValueType>
concept IsDefined = requires{ ValueType::value; };

template<class Name, class Parameter, class ...Things>
struct interpret;

template<class Name, class Parameter, class ...Things>
interpret(name_holder<Name>, Parameter, Things&&...) -> interpret<Name, Parameter, Things...>;

template<class Name, class T> 
interpret(name_holder<Name>, range_t, T &&) -> interpret<Name, range_t, std::remove_cvref_t<T>, std::remove_cvref_t<T>, std::remove_cvref_t<T>>;

template<class Name, class T>
interpret(name_holder<Name>, range_t, T &&, T &&) -> interpret<Name, range_t, std::remove_cvref_t<T>, std::remove_cvref_t<T>, std::remove_cvref_t<T>>;

template<class Name, class T>
interpret(name_holder<Name>, range_t, T &&, T &&, T &&) -> interpret<Name, range_t, std::remove_cvref_t<T>, std::remove_cvref_t<T>, std::remove_cvref_t<T>>;

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
		return formatter.format(Name::name, categories_);
	}

	template<class TunerFormatter, class Constraint>
	constexpr decltype(auto) generate(TunerFormatter &formatter, Constraint &&constraint) const {
		return formatter.format(Name::name, categories_, std::forward<Constraint>(constraint));
	}

private:
	static constexpr auto categories_ = category_parameter(sizeof...(Choices));
};

// TODO: add version with constants
template<class Name, class Start, class End, class Step> requires (!IsDefined<Name>)
struct interpret<Name, range_t, Start, End, Step> : flexible_contain<Start, End, Step> {
	using name = Name;

	template<class T>
	constexpr interpret(name_holder<Name>, range_t, T &&end)
		: flexible_contain<std::remove_cvref_t<T>, std::remove_cvref_t<T>, std::remove_cvref_t<T>>(0, std::forward<T>(end), 1) {}

	template<class Start_, class End_, class Step_>
	constexpr interpret(name_holder<Name>, range_t, Start_ &&begin, End_ &&end, Step_ &&step = (Step)1)
		: flexible_contain<Start, End, Step>(std::forward<Start>(begin), std::forward<End>(end), std::forward<Step>(step)) {}

	constexpr decltype(auto) operator*() const noexcept {
		return this->template get<0>();
	}

	constexpr decltype(auto) operator->() const noexcept {
		return &**this;
	}

	template<class TunerFormatter>	
	constexpr decltype(auto) generate(TunerFormatter &formatter) const {
		return formatter.format(Name::name, range_parameter(this->template get<0>(), this->template get<1>(), this->template get<2>()));
	}

	template<class TunerFormatter, class Constraint>
	constexpr decltype(auto) generate(TunerFormatter &formatter, Constraint &&constraint) const {
		return formatter.format(Name::name, range_parameter(this->template get<0>(), this->template get<1>(), this->template get<2>()), std::forward<Constraint>(constraint));
	}
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
		return formatter.format(Name::name, permutation_);
	}

	template<class TunerFormatter, class Constraint>
	constexpr decltype(auto) generate(TunerFormatter &formatter, Constraint &constraint) const {
		return formatter.format(Name::name, permutation_, constraint);
	}

private:
	static constexpr auto permutation_ = permutation_parameter(sizeof...(Choices));
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

// TODO: add version with constants
template<class Name, class Start, class End, class Step> requires (!IsDefined<Name>)
struct interpret<Name, range_t, Start, End, Step> : flexible_contain<Start> {
	using name = Name;

	template<class T>
	constexpr interpret(name_holder<Name>, range_t, T &&)
		: flexible_contain<Start>((Start)0) {}

	template<class Start_, class End_, class Step_>
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

template<class Name, class Value>
struct interpret<Name, value_t, Value> : flexible_contain<Value>  {
	using name = Name;

	template<class T>
	constexpr interpret(name_holder<Name>, value_t, T &&value)
		: flexible_contain<Value>(std::forward<T>(value)) {}

	constexpr decltype(auto) operator*() const noexcept {
		return this->template get<0>();
	}

	constexpr decltype(auto) operator->() const noexcept {
		return &**this;
	}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept { }
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
