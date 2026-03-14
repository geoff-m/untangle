cmake_minimum_required(VERSION 3.20)
project(untangle)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(lib)

enable_testing()
include(FetchContent)
add_subdirectory(test)
find_package(GoogleTest QUIET)
if(GoogleTest_FOUND)
    message(STATUS "Will use found GoogleTest package")
else()
    message(STATUS "GoogleTest package not found--will attempt to download it")
    FetchContent_Declare(
            GoogleTest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        52eb8108c5bdec04579160ae17225d66034bd723 # v1.17.0
    )
    FetchContent_MakeAvailable(GoogleTest)
endif()