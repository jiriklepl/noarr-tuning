#ifndef NOARR_TUNING_MACROS_HPP
#define NOARR_TUNING_MACROS_HPP

#include "tuning.hpp"

// TODO: BEGIN and END asserts
// TODO: CONSTRAINED_PAR assert

#define NOARR_TUNING_STRINGIFY2(x) #x
#define NOARR_TUNING_STRINGIFY(x) NOARR_TUNING_STRINGIFY2(x)

#define NOARR_TUNING_PARAMETER_NAME(name) NOARR_TUNING_PARAMETER_ ## name
#define NOARR_TUNING_PARAMETER_DEFINITION_NAME(name) NOARR_TUNING_PARAMETER_DEFINITION_ ## name
#define NOARR_TUNING_PARAMETER_MEMBER(name, ...) \
	decltype(::noarr::tuning::interpret(::noarr::tuning::name_holder<NOARR_TUNING_PARAMETER_NAME(name)>() __VA_OPT__(,) __VA_ARGS__)) \
	name{::noarr::tuning::name_holder<NOARR_TUNING_PARAMETER_NAME(name)>() __VA_OPT__(,) __VA_ARGS__}

#define NOARR_TUNING_PARAMETER_STRUCT_PASS_BY_DEFINE(parameter_name) \
	struct NOARR_TUNING_PARAMETER_NAME(parameter_name) { static constexpr const char *name= #parameter_name; static constexpr auto value = (::noarr::tuning::collect_values<NOARR_TUNING_PARAMETER_NAME(VALUE_ ## parameter_name)>()); }

#define NOARR_TUNING_PARAMETER_STRUCT_CONST(parameter_name) \
	struct NOARR_TUNING_PARAMETER_NAME(parameter_name) { static constexpr const char *name= #parameter_name; }

#if defined(NOARR_PASS_BY_DEFINE) && !defined(NOARR_TUNE)
#define NOARR_TUNING_PARAMETER_STRUCT(name) \
	NOARR_TUNING_PARAMETER_STRUCT_PASS_BY_DEFINE(name)
#else
#define NOARR_TUNING_PARAMETER_STRUCT(name) \
	NOARR_TUNING_PARAMETER_STRUCT_CONST(name)
#endif

#define NOARR_TUNING_PARAMETER_COMMON(name, ...) \
	NOARR_TUNING_PARAMETER_STRUCT(name); \
	NOARR_TUNING_PARAMETER_MEMBER(name __VA_OPT__(,) __VA_ARGS__)

#define NOARR_TUNING_PARAMETER_CONST(name, ...) \
	NOARR_TUNING_PARAMETER_STRUCT_CONST(name); \
	NOARR_TUNING_PARAMETER_MEMBER(name __VA_OPT__(,) __VA_ARGS__)

#ifndef NOARR_TUNE

#define NOARR_TUNE_BEGIN(formatter, ...) \
	static_assert(true, "NOARR_TUNE_BEGIN() must be called at the beginning of the tuning block.")

#define NOARR_TUNE_PAR(name, ...) \
	NOARR_TUNING_PARAMETER_COMMON(name __VA_OPT__(,) __VA_ARGS__)

#define NOARR_TUNE_CONSTRAINED_PAR(name, constraint, ...) \
	NOARR_TUNING_PARAMETER_COMMON(name __VA_OPT__(,) __VA_ARGS__)

#define NOARR_TUNE_END(...) \
	static_assert(true, "NOARR_TUNE_END() must be called at the end of the tuning block.")

#else

#define NOARR_TUNING_PARAMETER_DEFINITION(name) \
	decltype(::noarr::tuning::definition_t(formatter, name)) \
	NOARR_TUNING_PARAMETER_DEFINITION_NAME(name){formatter, name}

#define NOARR_TUNE_PAR(name, ...) \
	NOARR_TUNING_PARAMETER_COMMON(name __VA_OPT__(,) __VA_ARGS__); \
	NOARR_TUNING_PARAMETER_DEFINITION(name)

#define NOARR_TUNE_CONSTRAINED_PAR(name, constraint, ...) \
	NOARR_TUNING_PARAMETER_COMMON(name __VA_OPT__(,) __VA_ARGS__); \
	decltype(::noarr::tuning::definition_t(formatter, name, constraint)) \
	NOARR_TUNING_PARAMETER_DEFINITION(name){formatter, name, constraint}

#define NOARR_TUNE_BEGIN(formatter_init, ...) \
	decltype(formatter_init) formatter = formatter_init; \
	NOARR_TUNING_PARAMETER_CONST(NOARR_TUNING_header, ::noarr::tuning::begin __VA_OPT__(,) __VA_ARGS__); \
	NOARR_TUNING_PARAMETER_DEFINITION(NOARR_TUNING_header)

#define NOARR_TUNE_END(...) \
	NOARR_TUNING_PARAMETER_CONST(NOARR_TUNING_footer, ::noarr::tuning::end __VA_OPT__(,) __VA_ARGS__); \
	NOARR_TUNING_PARAMETER_DEFINITION(NOARR_TUNING_footer)

#endif // NOARR_TUNE

#define NOARR_TUNE_CONST(parameter_name, ...) \
	NOARR_TUNING_PARAMETER_CONST(parameter_name, ::noarr::tuning::constant __VA_OPT__(,) __VA_ARGS__)

#endif // NOARR_TUNING_MACROS_HPP
