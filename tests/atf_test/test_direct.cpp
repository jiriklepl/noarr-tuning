#ifndef ATF_HOME
#error "ATF_HOME is not defined"
#endif

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

static const char *GCC_FLAGS = "-O2 -g -std=c++20 -o tmp.bin -I../../../include -I../build/_deps/noarr-src/include -I\"" STRINGIFY(ATF_HOME) "\" -DATF_HOME\"" STRINGIFY(ATF_HOME) "\" " __FILE__;

#define SPECIFIC_FORMATTER noarr/tuning/formatters/atf_formatter.hpp
#define SPECIFIC_TUNING_BEGIN(...) NOARR_TUNE_BEGIN(noarr::tuning::atf_formatter( \
		atf::evaluations(10), \
		noarr::tuning::direct_compile_command_builder("g++", GCC_FLAGS), \
		noarr::tuning::direct_run_command_builder("./tmp.bin", "../matrices 512")))

#define SPECIFIC_GET_PAR(...) NOARR_ATF_TP(__VA_ARGS__)
#define SPECIFIC_TUNING_END(...) NOARR_TUNE_END(__VA_ARGS__)

#include "../program.hpp"
