cmake_minimum_required(VERSION 3.20)

include(CheckIncludeFile)
check_include_file("pthread.h" HAVE_PTHREAD)
if (HAVE_PTHREAD)
    set(PLATFORM_SOURCES
            src/include/pthreads_intercept.h
            src/pthreads_intercept.cpp
            src/platform-linux.cpp
            src/library-lifecycle-linux.cpp)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PLATFORM_SOURCES
            src/platform-windows.cpp)
else ()
    message(FATAL_ERROR "Unsupported platform")
endif ()
message("platform_srcs: ${PLATFORM_SOURCES}")
add_library(untangle SHARED
        include/untangle/untangle.h
        src/include/MutexInfo.h
        src/MutexInfo.cpp
        src/write.cpp
        src/include/write.h
        ${PLATFORM_SOURCES}
        src/include/platform.h
        src/unreachable.cpp
        src/library-lifecycle.cpp)

target_include_directories(untangle PRIVATE src/include)
target_include_directories(untangle PUBLIC include)
