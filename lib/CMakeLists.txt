cmake_minimum_required(VERSION 3.20)

include(CheckIncludeFile)
check_include_file("pthread.h" HAVE_PTHREAD)
if (HAVE_PTHREAD)
    set(PLATFORM_SOURCES
            src/include/pthreads_intercept.h
            src/linux/pthreads_intercept.cpp
            src/linux/platform-linux.cpp
            src/linux/library-lifecycle-linux.cpp)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PLATFORM_SOURCES
            src/windows/platform-windows.cpp
            src/windows/detours-intercept.cpp
            src/windows/library-lifecycle-windows.cpp)
else ()
    message(FATAL_ERROR "Unsupported platform")
endif ()

add_library(untangle SHARED
        include/untangle/untangle.h
        src/include/MutexInfo.h
        src/MutexInfo.cpp
        src/write.cpp
        src/include/write.h
        ${PLATFORM_SOURCES}
        src/include/platform.h
        src/unreachable.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(untangle PROPERTIES OUTPUT_NAME "untangle64")
    link_detours(untangle)
    if(MSVC)
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    endif()
endif()

target_include_directories(untangle PRIVATE src/include)
target_include_directories(untangle PUBLIC include)


