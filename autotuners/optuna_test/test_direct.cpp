static const char *GCC_FLAGS = "-O2 -g -std=c++20 -o tmp.bin -I../../../include " __FILE__;
#define SPECIFIC_TUNING_BEGIN NOARR_TUNE_BEGIN(optuna_formatter( \
		std::cout, \
		std::make_shared<noarr::tuning::direct_compile_command_builder>("g++", GCC_FLAGS), \
		std::make_shared<noarr::tuning::direct_run_command_builder>("./tmp.bin", "../matrices 1024"), \
		"return run_time"))

#include "test.hpp"
#include "../program.hpp"
