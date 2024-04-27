#define SPECIFIC_FORMATTER noarr/tuning/formatters/optuna_formatter.hpp
#define SPECIFIC_TUNING_BEGIN(...) NOARR_TUNE_BEGIN(noarr::tuning::optuna_formatter( \
		std::cout, \
		noarr::tuning::cmake_compile_command_builder("..", ".", "optuna_test_cmake_kernel"), \
		noarr::tuning::direct_run_command_builder("./optuna_test_cmake_kernel", "../matrices 512"), \
		"return int(run_result.stderr.decode().split()[2])"))

#include "../program.hpp"
