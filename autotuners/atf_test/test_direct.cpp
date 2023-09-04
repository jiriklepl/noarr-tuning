#include <noarr/structures/tuning/formatters/atf_formatter.hpp>

static const char *GCC_FLAGS = "-O2 -g -std=c++20 -o tmp.bin -I../../../include " __FILE__;
#define SPECIFIC_TUNING_BEGIN(...) NOARR_TUNE_BEGIN(noarr::tuning::atf_formatter( \
		std::cout, \
		std::make_shared<noarr::tuning::direct_compile_command_builder>("g++", GCC_FLAGS), \
		std::make_shared<noarr::tuning::direct_run_command_builder>("./tmp.bin", "../matrices 1024")))

#define SPECIFIC_GET_PAR(...) NOARR_ATF_TP(__VA_ARGS__)
#define SPECIFIC_TUNING_END(...) NOARR_TUNE_END(__VA_ARGS__)

#include "../program.hpp"
