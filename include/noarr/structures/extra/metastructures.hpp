#ifndef NOARR_STRUCTURES_METASTRUCTURES_HPP
#define NOARR_STRUCTURES_METASTRUCTURES_HPP

#include "../base/contain.hpp"
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

// TODO: implement
struct clone_t {};

template<class ValueType>
concept IsDefined = requires{ ValueType::value; };

// TODO: name
// TODO: implement
template<class Name, class Parameter, class ...Things>
struct interpret;

template<class Name, class Parameter, class ...Things>
interpret(placeholder<Name>, Parameter, Things &&...) -> interpret<Name, Parameter, Things...>;

// TODO: choice can also contain other metastructures

template<class Name> requires (!IsDefined<Name>)
struct interpret<Name, begin_t> : contain<> {
	constexpr interpret(placeholder<Name>, begin_t) noexcept : contain<>() {};

	template<class TunerFormatter>
	void generate(TunerFormatter &formatter) const {
		formatter.header();
	}
};

template<class Name> requires (!IsDefined<Name>)
struct interpret<Name, end_t> : contain<> {
	constexpr interpret(placeholder<Name>, end_t) noexcept : contain<>() {};

	template<class TunerFormatter>
	void generate(TunerFormatter &formatter) const {
		formatter.footer();
		std::exit(0);
	}
};

template<class Name, class ...Choices> requires (!IsDefined<Name>)
struct interpret<Name, choice_t, Choices...> : contain<Choices...>  {
	using base = contain<Choices...>;

	constexpr interpret(placeholder<Name>, choice_t, Choices &&...choices)
		: base(std::forward<Choices>(choices)...) {}

	template<class TunerFormatter>
	void generate(TunerFormatter &formatter) const {
		formatter.format(begin);
		formatter.format(name);
		formatter.format(categories);
		formatter.format(end);
	}

	constexpr decltype(auto) operator*() const noexcept {
		return base::template get<0>();
	}

private:
	static constexpr auto begin = begin_parameter();
	static constexpr auto end = end_parameter();
	static constexpr auto name = name_parameter(Name::name);
	static constexpr auto categories = category_parameter(sizeof...(Choices));
};

// TODO: `Name::value` -> `Name::choice` or something?
template<class Name, class ...Choices> requires (IsDefined<Name>)
struct interpret<Name, choice_t, Choices...> : contain<Choices...>  {
	using base = contain<Choices...>;

	constexpr interpret(placeholder<Name>, choice_t, Choices &&...choices)
		: base(std::forward<Choices>(choices)...) {}

	constexpr decltype(auto) operator*() const noexcept {
		return base::template get<Name::value.template get<0>()>();
	}

	template<class T>
	constexpr void generate(T &&) const noexcept { }
};

// TODO: implement
// TODO: add version with constants
template<class Name> requires (!IsDefined<Name>)
struct interpret<Name, range_t, std::size_t, std::size_t, std::size_t> {

};


template<class Formatter, class Parameter>
struct enqueuer_type {
	enqueuer_type(Formatter &first, Parameter &second) {
		second.generate(first);
	}
};

template<class Name, class ...Choices> requires (!IsDefined<Name>)
struct interpret<Name, permutation_t, Choices...> : contain<Choices...>  {
	using base = contain<Choices...>;

	constexpr interpret(placeholder<Name>, permutation_t, Choices &&...choices)
		: base(std::forward<Choices>(choices)...) {}

	template<class TunerFormatter>
	void generate(TunerFormatter &formatter) const {
		formatter.format(begin);
		formatter.format(name);
		formatter.format(permutation);
		formatter.format(end);
	}

	constexpr const base &operator*() const noexcept {
		return *this;
	}

private:
	static constexpr auto begin = begin_parameter();
	static constexpr auto end = end_parameter();
	static constexpr auto name = name_parameter(Name::name);
	static constexpr auto permutation = permutation_parameter(sizeof...(Choices));
};

template<class Name, class ...Choices> requires (IsDefined<Name>)
struct interpret<Name, permutation_t, Choices...> : contain<Choices...>  {
	using base = contain<Choices...>;

	constexpr interpret(placeholder<Name>, permutation_t, Choices &&...choices)
		: base(std::forward<Choices>(choices)...) {}

	constexpr const base &operator*() const noexcept {
		return *this;
	}

	template<class T>
	constexpr void generate(T &&) const noexcept { }
};

} // namespace noarr::tuning

#endif // NOARR_STRUCTURES_METASTRUCTURES_HPP
