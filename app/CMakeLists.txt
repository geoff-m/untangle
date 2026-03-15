cmake_minimum_required(VERSION 3.20)

add_executable(untangle-app
        src/main.c
)

target_link_libraries(untangle-app
        PRIVATE untangle)

target_compile_definitions(untangle-app PRIVATE LIBRARY_DIR="${CMAKE_INSTALL_PREFIX}")

set_target_properties(untangle-app PROPERTIES OUTPUT_NAME "untangle")
