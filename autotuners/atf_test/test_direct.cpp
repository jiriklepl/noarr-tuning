static const char *GCC_FLAGS = "-O2 -g -std=c++20 -o tmp.bin -I../../../include -I/home/jirka/Documents/extern/atf " __FILE__;
#define SPECIFIC_TUNING_BEGIN(...) NOARR_TUNE_BEGIN(atf_formatter( \
		std::cout, \
		std::make_shared<noarr::tuning::direct_compile_command_builder>("g++", GCC_FLAGS), \
		std::make_shared<noarr::tuning::direct_run_command_builder>("./tmp.bin", "../matrices 1024")))

#define SPECIFIC_GET_PAR(...) ATF_GET_TP(__VA_ARGS__)

#define SPECIFIC_TUNING_END(...) NOARR_TUNE_END(__VA_ARGS__)

#include "test.hpp"
#include "../program.hpp"
