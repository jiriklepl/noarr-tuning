#include <noarr/structures/tuning/formatters/optuna_formatter.hpp>

#define SPECIFIC_TUNING_BEGIN(...) NOARR_TUNE_BEGIN(noarr::tuning::optuna_formatter( \
		std::cout, \
		std::make_shared<noarr::tuning::cmake_compile_command_builder>("../..", "build", "opentuner_test_cmake_kernel"), \
		std::make_shared<noarr::tuning::direct_run_command_builder>("build/opentuner_test_cmake_kernel", "../matrices 1024"), \
		"return Result(time=run_result['time'])"))

#include "../program.hpp"
