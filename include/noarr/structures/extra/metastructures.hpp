#ifndef NOARR_STRUCTURES_METASTRUCTURES_HPP
#define NOARR_STRUCTURES_METASTRUCTURES_HPP

#include <cstdlib>
#include <type_traits>
#include <utility>

#include "../base/structs_common.hpp"
#include "../extra/metaformatter.hpp"

namespace noarr::tuning {

template<class ...>
struct placeholder {};

struct begin_t {};
inline constexpr begin_t begin;

struct end_t {};
inline constexpr end_t end;

struct choice_t {};
inline constexpr choice_t choice;

// TODO: name
// TODO: implement
struct multi_choice_t {};
inline constexpr multi_choice_t multi_choice;

// TODO: name
// TODO: implement
struct sequence_t {};
inline constexpr sequence_t sequence;

// TODO: name
// TODO: implement
struct plain_code_t {};
inline constexpr plain_code_t plain_code;

// TODO: implement
struct value_t {};
inline constexpr value_t value;

// TODO: implement
struct permutation_t {};
inline constexpr permutation_t permutation;

// TODO: name
// TODO: implement
struct range_t {};

// TODO: implement
struct lambda_t {};

template<class ValueType>
concept IsDefined = requires{ ValueType::value; };

// TODO: name
// TODO: implement
template<class Name, class Parameter, class ...Things>
struct interpret;

template<class Name, class Parameter, class ...Things>
interpret(placeholder<Name>, Parameter, Things&&...) -> interpret<Name, Parameter, Things...>;

// TODO: choice can also contain other metastructures

template<class Name> requires (!IsDefined<Name>)
struct interpret<Name, begin_t> : contain<> {
	using name = Name;

	constexpr interpret(placeholder<Name>, begin_t) noexcept : contain<>() {};

	template<class TunerFormatter, class ...Args>
	constexpr decltype(auto) generate(TunerFormatter &formatter, const Args &...args) const {
		return formatter.header(args...);
	}
};

template<class Name, class ...Args> requires (!IsDefined<Name>)
struct interpret<Name, end_t, Args...> : contain<Args...> {
	using name = Name;

	constexpr interpret(placeholder<Name>, end_t, const Args &...args) noexcept : contain<Args...>(args...) {
		
	};

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
struct interpret<Name, choice_t, Choices...> : contain<Choices...>  {
	using name = Name;

	constexpr interpret(placeholder<Name>, choice_t, Choices &&...choices)
		: contain<Choices...>(std::forward<Choices>(choices)...) {}

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
	constexpr decltype(auto) generate(TunerFormatter &formatter, Constraint &constraint) const {
		return formatter.format(Name::name, categories_, constraint);
	}

private:
	static constexpr auto categories_ = category_parameter(sizeof...(Choices));
};

// TODO: `Name::value` -> `Name::choice` or something?
template<class Name, class ...Choices> requires (IsDefined<Name>)
struct interpret<Name, choice_t, Choices...> : contain<Choices...>  {
	using name = Name;

	constexpr interpret(placeholder<Name>, choice_t, Choices &&...choices)
		: contain<Choices...>(std::forward<Choices>(choices)...) {}

	constexpr decltype(auto) operator*() const noexcept {
		return this->template get<Name::value.template get<0>()>();
	}

	constexpr decltype(auto) operator->() const noexcept {
		return &**this;
	}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept { }
};

// TODO: implement
// TODO: add version with constants
template<class Name> requires (!IsDefined<Name>)
struct interpret<Name, range_t, std::size_t, std::size_t, std::size_t> {

};

template<class Name, class ...Choices> requires (!IsDefined<Name>)
struct interpret<Name, permutation_t, Choices...> : contain<Choices...>  {
	using name = Name;

	constexpr interpret(placeholder<Name>, permutation_t, Choices &&...choices)
		: contain<Choices...>(std::forward<Choices>(choices)...) {}

	constexpr const contain<Choices...> &operator*() const noexcept {
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

template<class Name, class ...Choices> requires (IsDefined<Name>)
struct interpret<Name, permutation_t, Choices...> : contain<Choices...>  {
	using name = Name;

	constexpr interpret(placeholder<Name>, permutation_t, Choices &&...choices)
		: contain<Choices...>(std::forward<Choices>(choices)...) {}

	constexpr const contain<Choices...> &operator*() const noexcept {
		return *this;
	}

	constexpr decltype(auto) operator->() const noexcept {
		return &**this;
	}

	template<class ...Ts>
	constexpr void generate(Ts &&...) const noexcept { }
};

template<class Formatter, class Parameter, class Constraint>
struct definition_t {
	using value_type = decltype(std::declval<Parameter>().generate((Formatter&)(std::declval<Formatter>()), (Constraint&)(std::declval<Constraint>())));

	constexpr definition_t(Formatter &formatter, Parameter &parameter, Constraint &constraint) requires (!std::is_same_v<value_type, void>)
		: value_(parameter.generate(formatter, constraint)) {}

	constexpr definition_t(Formatter &formatter, Parameter &parameter, Constraint &constraint) requires (std::is_same_v<value_type, void>) {
		parameter.generate(formatter, constraint);
	}

	constexpr decltype(auto) operator*() const noexcept {
		return value_;
	}

private:
	std::conditional_t<!std::is_same_v<value_type, void>, value_type, empty_t> value_;
};

template<class Formatter, class Parameter>
struct definition_t<Formatter, Parameter, void> {
	using value_type = decltype(std::declval<Parameter>().generate((Formatter&)(std::declval<Formatter>())));

	constexpr definition_t(Formatter &formatter, Parameter &parameter) requires (!std::is_same_v<value_type, void>)
		: value_(parameter.generate(formatter)) {}
	
	constexpr definition_t(Formatter &formatter, Parameter &parameter) requires (std::is_same_v<value_type, void>) {
		parameter.generate(formatter);
	}

	constexpr decltype(auto) operator*() const noexcept {
		return value_;
	}

private:
	std::conditional_t<!std::is_same_v<value_type, void>, value_type, empty_t> value_;
};

template<class Formatter, class Parameter, class Constraint>
definition_t(Formatter &, Parameter &, Constraint &) -> definition_t<Formatter, Parameter, Constraint>;

template<class Formatter, class Parameter>
definition_t(Formatter &, Parameter &) -> definition_t<Formatter, Parameter, void>;

} // namespace noarr::tuning

#endif // NOARR_STRUCTURES_METASTRUCTURES_HPP
