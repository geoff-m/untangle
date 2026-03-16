cmake_minimum_required(VERSION 3.20)

add_executable(untangle-test
        src/Tests.cpp
        src/include/OutputCapture.h
        src/utility/OutputCapture.cpp
        src/include/WaitCycle.h
        src/utility/WaitCycle.cpp
        src/TestMutexName.cpp
        src/TestSetWriter.cpp
)

target_include_directories(untangle-test PRIVATE src/include)

target_link_libraries(untangle-test
        PRIVATE untangle
        PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(untangle-test)
