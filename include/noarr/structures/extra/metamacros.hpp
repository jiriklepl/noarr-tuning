#ifndef NOARR_STRUCTURES_METAMACROS_HPP
#define NOARR_STRUCTURES_METAMACROS_HPP

#include "noarr/structures/extra/metastructures.hpp"

// TODO: pass by in-code define
// TODO: pass by code
// TODO: eager

#define NOARR_PARAMETER(name) NOARR_PARAMETER_ ## name
#define NOARR_PARAMETER_ENQUEUE(name) NOARR_PARAMETER_ENQUEUE_ ## name

#ifdef NOARR_PASS_BY_DEFINE

#define NOARR_TUNE_BEGIN(formatter) \
	static_assert(true, "NOARR_TUNE_BEGIN() must be called at the beginning of the tuning block.")

#define NOARR_TUNE_PAR(parameter_name, ...) \
	struct NOARR_PARAMETER(parameter_name) { static constexpr const char *name= #parameter_name; static constexpr auto value = noarr::contain(NOARR_PARAMETER(VALUE_ ## parameter_name)); }; \
	decltype(::noarr::tuning::interpret(::noarr::tuning::placeholder<NOARR_PARAMETER(parameter_name)>(), __VA_ARGS__)) \
	parameter_name{::noarr::tuning::placeholder<NOARR_PARAMETER(parameter_name)>(), __VA_ARGS__} \

#define NOARR_TUNE_END() \
	static_assert(true, "NOARR_TUNE_END() must be called at the end of the tuning block.")

#else

#define NOARR_TUNE_PAR(parameter_name, ...) \
	struct NOARR_PARAMETER(parameter_name) { static constexpr const char *name= #parameter_name; }; \
	decltype(::noarr::tuning::interpret(::noarr::tuning::placeholder<NOARR_PARAMETER(parameter_name)>(), __VA_ARGS__)) \
	parameter_name{::noarr::tuning::placeholder<NOARR_PARAMETER(parameter_name)>(), __VA_ARGS__}; \
	::noarr::tuning::enqueue_t \
	NOARR_PARAMETER_ENQUEUE(parameter_name){formatter, parameter_name}

#define NOARR_TUNE_BEGIN(formatter_init) \
	decltype(formatter_init) \
	formatter = formatter_init; \
	NOARR_TUNE_PAR(NOARR_header, ::noarr::tuning::begin)

#define NOARR_TUNE_END() \
	NOARR_TUNE_PAR(NOARR_footer, ::noarr::tuning::end)

#endif // NOARR_PASS_BY_DEFINE


#endif // NOARR_STRUCTURES_METAMACROS_HPP
