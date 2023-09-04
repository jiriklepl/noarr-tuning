#include <noarr/structures/tuning/formatters/atf_formatter.hpp>

#define SPECIFIC_TUNING_BEGIN(...) NOARR_TUNE_BEGIN(noarr::tuning::atf_formatter( \
		std::cout, \
		std::make_shared<noarr::tuning::cmake_compile_command_builder>("../..", "build", "atf_test_cmake_kernel", "", "\""), \
		std::make_shared<noarr::tuning::direct_run_command_builder>("build/atf_test_cmake_kernel", "../matrices 1024")))

#define SPECIFIC_GET_PAR(...) NOARR_ATF_TP(__VA_ARGS__)
#define SPECIFIC_TUNING_END(...) NOARR_TUNE_END(__VA_ARGS__)

#include "../program.hpp"
