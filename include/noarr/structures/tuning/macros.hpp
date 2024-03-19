#ifndef NOARR_STRUCTURES_TUNING_MACROS_HPP
#define NOARR_STRUCTURES_TUNING_MACROS_HPP

#include "../tuning/tuning.hpp"

// TODO: pass by in-code define
// TODO: pass by code

// TODO: BEGIN and END asserts
// TODO: CONSTRAINED_PAR assert

#define NOARR_PARAMETER(name) NOARR_PARAMETER_ ## name
#define NOARR_PARAMETER_DEFINITION(name) NOARR_PARAMETER_DEFINITION_ ## name

#ifdef NOARR_PASS_BY_DEFINE

#define NOARR_TUNE_BEGIN(formatter, ...) \
	static_assert(true, "NOARR_TUNE_BEGIN() must be called at the beginning of the tuning block.")

#define NOARR_TUNE_PAR(parameter_name, ...) \
	struct NOARR_PARAMETER(parameter_name) { static constexpr const char *name= #parameter_name; static constexpr auto value = (::noarr::tuning::collect_values<NOARR_PARAMETER(VALUE_ ## parameter_name)>()); }; \
	decltype(::noarr::tuning::interpret(::noarr::tuning::name_holder<NOARR_PARAMETER(parameter_name)>(), __VA_ARGS__)) \
	parameter_name{::noarr::tuning::name_holder<NOARR_PARAMETER(parameter_name)>(), __VA_ARGS__}

#define NOARR_TUNE_CONSTRAINED_PAR(parameter_name, constraint, ...) \
	struct NOARR_PARAMETER(parameter_name) { static constexpr const char *name= #parameter_name; static constexpr auto value = (::noarr::tuning::collect_values<NOARR_PARAMETER(VALUE_ ## parameter_name)>()); }; \
	decltype(::noarr::tuning::interpret(::noarr::tuning::name_holder<NOARR_PARAMETER(parameter_name)>(), __VA_ARGS__)) \
	parameter_name{::noarr::tuning::name_holder<NOARR_PARAMETER(parameter_name)>(), __VA_ARGS__}

#define NOARR_TUNE_END(...) \
	static_assert(true, "NOARR_TUNE_END() must be called at the end of the tuning block.")

#else

#define NOARR_TUNE_PAR(parameter_name, ...) \
	struct NOARR_PARAMETER(parameter_name) { static constexpr const char *name= #parameter_name; }; \
	decltype(::noarr::tuning::interpret(::noarr::tuning::name_holder<NOARR_PARAMETER(parameter_name)>(), __VA_ARGS__)) \
	parameter_name{::noarr::tuning::name_holder<NOARR_PARAMETER(parameter_name)>(), __VA_ARGS__}; \
	decltype(::noarr::tuning::definition_t(formatter, parameter_name)) \
	NOARR_PARAMETER_DEFINITION(parameter_name){formatter, parameter_name}

#define NOARR_TUNE_CONSTRAINED_PAR(parameter_name, constraint, ...) \
	struct NOARR_PARAMETER(parameter_name) { static constexpr const char *name= #parameter_name; }; \
	decltype(::noarr::tuning::interpret(::noarr::tuning::name_holder<NOARR_PARAMETER(parameter_name)>(), __VA_ARGS__)) \
	parameter_name{::noarr::tuning::name_holder<NOARR_PARAMETER(parameter_name)>(), __VA_ARGS__}; \
	decltype(::noarr::tuning::definition_t(formatter, parameter_name, constraint)) \
	NOARR_PARAMETER_DEFINITION(parameter_name){formatter, parameter_name, constraint}

#define NOARR_TUNE_BEGIN(formatter_init, ...) \
	decltype(formatter_init) \
	formatter = formatter_init; \
	NOARR_TUNE_PAR(NOARR_header, ::noarr::tuning::begin __VA_OPT__(,) __VA_ARGS__)

#define NOARR_TUNE_END(...) \
	NOARR_TUNE_PAR(NOARR_footer, ::noarr::tuning::end __VA_OPT__(,) __VA_ARGS__)

#endif // NOARR_PASS_BY_DEFINE

#define NOARR_TUNE_CONST(parameter_name, ...) \
	struct NOARR_PARAMETER(parameter_name) { static constexpr const char *name= #parameter_name; }; \
	decltype(::noarr::tuning::interpret(::noarr::tuning::name_holder<NOARR_PARAMETER(parameter_name)>(), ::noarr::tuning::constant, __VA_ARGS__)) \
	parameter_name{::noarr::tuning::name_holder<NOARR_PARAMETER(parameter_name)>(), ::noarr::tuning::constant, __VA_ARGS__}

#endif // NOARR_STRUCTURES_TUNING_MACROS_HPP
