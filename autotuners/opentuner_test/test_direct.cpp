#include <noarr/structures/tuning/formatters/opentuner_formatter.hpp>

static const char *GCC_FLAGS = "-O2 -g -std=c++20 -o tmp.bin -I../../../include " __FILE__;
#define SPECIFIC_TUNING_BEGIN(...) NOARR_TUNE_BEGIN(noarr::tuning::opentuner_formatter( \
		std::cout, \
		std::make_shared<noarr::tuning::direct_compile_command_builder>("g++", GCC_FLAGS), \
		std::make_shared<noarr::tuning::direct_run_command_builder>("./tmp.bin", "../matrices 1024"), \
		"return Result(time=int(run_result['stderr'].split()[2]))"))

#include "../program.hpp"
