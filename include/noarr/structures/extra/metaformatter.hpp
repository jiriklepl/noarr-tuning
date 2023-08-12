#ifndef NOARR_STRUCTURES_METAFORMATTER_HPP
#define NOARR_STRUCTURES_METAFORMATTER_HPP

#include <cstdint>
#include <ostream>
#include <memory>

#include "../base/contain.hpp"

namespace noarr::tuning {

// TODO: add type of the ...
// TODO: rename from "parameter"
struct begin_parameter {};

// TODO: rename from "parameter"
struct end_parameter {};

struct name_parameter {
	constexpr name_parameter(const char *name) noexcept : name_(name) {};

	const char *name_;
};

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


template<class T>
concept IsTunerFormatter = requires(T t) {
	{ t.header() } -> std::same_as<void>;
	{ t.footer() } -> std::same_as<void>;

	{ t.format(begin_parameter()) } -> std::same_as<void>;
	{ t.format(end_parameter()) } -> std::same_as<void>;
	{ t.format(name_parameter("name")) } -> std::same_as<void>;
	{ t.format(category_parameter(0)) } -> std::same_as<void>;
	{ t.format(multiple_choice_parameter()) } -> std::same_as<void>;
	{ t.format(permutation_parameter(0)) } -> std::same_as<void>;
};

} // namespace noarr::tuning

#endif // NOARR_STRUCTURES_METAFORMATTER_HPP