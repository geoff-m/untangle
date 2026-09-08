cmake_minimum_required(VERSION 3.20)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(APP_SOURCES
        src/linux/main.c
    )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(APP_SOURCES
        src/windows/main.cpp
    )
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

add_executable(untangle-app
        ${APP_SOURCES}
)
        
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(untangle-app PRIVATE LIBRARY_DIR="${CMAKE_INSTALL_PREFIX}")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    link_detours(untangle-app)
endif()


set_target_properties(untangle-app PROPERTIES OUTPUT_NAME "untangle")

