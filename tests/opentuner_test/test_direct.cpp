static const char *GCC_FLAGS = "-O2 -g -std=c++20 -o tmp.bin -I../../../include -I../build/_deps/noarr-src/include " __FILE__;

#define SPECIFIC_FORMATTER noarr/tuning/formatters/opentuner_formatter.hpp
#define SPECIFIC_TUNING_BEGIN(...) NOARR_TUNE_BEGIN(noarr::tuning::opentuner_formatter( \
		std::cout, \
		noarr::tuning::direct_compile_command_builder("g++", GCC_FLAGS), \
		noarr::tuning::direct_run_command_builder("./tmp.bin", "../matrices 512"), \
		"return Result(time=int(run_result['stderr'].split()[2]))"))

#include "../program.hpp"
