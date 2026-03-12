cmake_minimum_required(VERSION 3.20)

add_library(untangle SHARED
        src/include/pthreads_intercept.h
        src/pthreads_intercept.cpp)

target_include_directories(untangle PRIVATE src/include)
