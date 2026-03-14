cmake_minimum_required(VERSION 3.20)

add_library(untangle SHARED
        src/include/pthreads_intercept.h
        src/pthreads_intercept.cpp
        src/include/MutexInfo.h
        src/MutexInfo.cpp
        src/write.cpp
        include/untangle/untangle.h
        src/include/write.h)

target_include_directories(untangle PRIVATE src/include)
target_include_directories(untangle PUBLIC include)
