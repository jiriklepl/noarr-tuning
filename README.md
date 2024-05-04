# Noarr Tuning

This repository contains the source code for the Noarr Tuning header-only library.

The library is designed to provide a simple and efficient way to tune the performance of C++ (or CUDA) code and work well with the [Noarr Structures](https://github.com/jiriklepl/noarr-structures) library. See the [tests](tests) directory for examples of this combination.

The library's abstraction separates the tuning backend's specifics from the tuned code. Currently, the library supports the [OpenTuner](https://opentuner.org/) and [Optuna](https://optuna.org/) autotuning frameworks via the [include/noarr/tuning/formatters/opentuner_formatter.hpp](include/noarr/tuning/formatters/opentuner_formatter.hpp) and [include/noarr/tuning/formatters/optuna_formatter.hpp](include/noarr/tuning/formatters/optuna_formatter.hpp) files, respectively. There is also an experimental support for the [ATF](https://gitlab.com/mdh-project/atf.git) framework via the [include/noarr/tuning/formatters/atf_formatter.hpp](include/noarr/tuning/formatters/atf_formatter.hpp) file. All of these are showcased in the [tests](tests) directory.

## Getting Started

The library is header-only and can be included in your project by adding the following line to your CMakeLists.txt file:

```cmake
target_include_directories(your_target PRIVATE path/to/noarr-tuning/include)
```

If you want to use the FetchContent module to include the library in your project, you can use the following code snippet:

```cmake
include(FetchContent)

FetchContent_Declare(
    NoarrTuning
    GIT_REPOSITORY https://github.com/jiriklepl/noarr-tuning.git
    GIT_TAG        main)
FetchContent_MakeAvailable(NoarrTuning)

target_include_directories(your_target PRIVATE ${NoarrTuning_SOURCE_DIR}/include)
```

### Prerequisites

The library requires a C++20-compliant compiler. No further dependencies are needed.

### Usage

The library provides a simple interface to autotune your code. The following code snippet demonstrates how to use the library with the [CMake](https://cmake.org/) build system (things in ALL CAPS are placeholders for user-defined values):

```cpp
#include <iostream>

// include the library
#include <noarr/tuning/formatters/opentuner_formatter.hpp>

// define the tuned parameters
struct tuning {
    NOARR_TUNE_BEGIN(noarr::tuning::opentuner_formatter(
        std::cout, // output stream for the tuning script
        noarr::tuning::cmake_compile_command_builder(CMAKE_DIR, BUILD_DIR, ALGORITHM_NAME), 
        noarr::tuning::direct_run_command_builder(ALGORITHM_NAME, ALGORITHM_ARGS...),
        "return Result(time=int(run_result['stdout'].split()[2]))"));
    
    NOARR_TUNE_PAR(size, noarr::tuning::range, 10);
    NOARR_TUNE_PAR(layout, noarr::tuning::choice, LAYOUT1, LAYOUT2, LAYOUT3);

    NOARR_TUNE_END();
} tuning;

int main() {
    // use the tuned parameters:
    auto structure = CONSTRUCT(*tuning.layout, 1 << *tuning.size);

    auto time = MEASURE_ALGORITHM(structure);

    // report the time
    std::cout << "Time: " << time << std::endl;
}
```

## License

This project is licensed under the permissive MIT License - see the [LICENSE](LICENSE) file for details. Any contributions to the project are welcome.
