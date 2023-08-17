#ifndef NOARR_STRUCTURES_METAFORMATTER_HPP
#define NOARR_STRUCTURES_METAFORMATTER_HPP

#include <cstdint>
#include <concepts>

#include "../base/contain.hpp"

namespace noarr::tuning {

// TODO: preferred category? distribution? names?
//   OpenTuner calls it a switch
struct category_parameter {
	constexpr category_parameter(std::size_t num) noexcept : num_(num) {}

	std::size_t num_;
};

// TODO: add multiple choice parameter
struct multiple_choice_parameter {};

struct permutation_parameter {
	constexpr permutation_parameter(std::size_t num) noexcept : num_(num) {}

	std::size_t num_;
};

template<class Predicate>
struct predicate_parameter {
private:
	Predicate predicate_;

public:
	constexpr predicate_parameter(Predicate predicate) noexcept : predicate_(predicate) {}

	Predicate predicate() const noexcept { return predicate_; }

	template<class T>
	constexpr bool operator()(T &&arg) const noexcept {
		return predicate_(arg);
	}

	constexpr operator Predicate() const noexcept { return predicate_; }
};

template<class T>
concept IsTunerFormatter = requires(T t) {
	{ t.header() } -> std::same_as<void>;
	{ t.footer() } -> std::same_as<void>;

	{ t.format("name", category_parameter(0)) } -> std::same_as<void>;
	{ t.format("name", multiple_choice_parameter()) } -> std::same_as<void>; // TODO: think about this
	{ t.format("name", permutation_parameter(0)) } -> std::same_as<void>;
};

template<class T>
concept IsConstrainedTunerFormatter = IsTunerFormatter<T> && requires(T t) {
	{ t.format("name", predicate_parameter([](auto &&) { return true; })) } -> std::same_as<void>; // TODO: think about this
};

} // namespace noarr::tuning

#endif // NOARR_STRUCTURES_METAFORMATTER_HPP