#define SPECIFIC_TUNING_BEGIN NOARR_TUNE_BEGIN(opentuner_formatter( \
		std::cout, \
		std::make_shared<noarr::tuning::cmake_compile_command_builder>("../..", "build", "opentuner_test_cmake_kernel"), \
		std::make_shared<noarr::tuning::direct_run_command_builder>("build/opentuner_test_cmake_kernel", "../matrices 1024"), \
		"return Result(time=run_result['time'])"))

#include "test.hpp"
#include "../program.hpp"
