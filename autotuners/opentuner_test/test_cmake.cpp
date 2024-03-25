#include <noarr/structures/tuning/formatters/opentuner_formatter.hpp>

#define SPECIFIC_TUNING_BEGIN(...) NOARR_TUNE_BEGIN(noarr::tuning::opentuner_formatter( \
		std::cout, \
		noarr::tuning::cmake_compile_command_builder("../..", "build", "opentuner_test_cmake_kernel"), \
		noarr::tuning::direct_run_command_builder("build/opentuner_test_cmake_kernel", "../matrices 1024"), \
		"return Result(time=int(run_result['stderr'].split()[2]))"))

#include "../program.hpp"
