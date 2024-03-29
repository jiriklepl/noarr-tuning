#include <noarr/tuning/formatters/optuna_formatter.hpp>

static const char *GCC_FLAGS = "-O2 -g -std=c++20 -o tmp.bin -I../../../include -I../build/_deps/noarr-src/include " __FILE__;
#define SPECIFIC_TUNING_BEGIN(...) NOARR_TUNE_BEGIN(noarr::tuning::optuna_formatter( \
		std::cout, \
		noarr::tuning::direct_compile_command_builder("g++", GCC_FLAGS), \
		noarr::tuning::direct_run_command_builder("./tmp.bin", "../matrices 512"), \
		"return int(run_result.stderr.decode().split()[2])"))

#include "../program.hpp"
