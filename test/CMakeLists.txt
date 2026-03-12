cmake_minimum_required(VERSION 3.20)

add_executable(untangle-test src/main.cpp
        src/main.cpp)

target_link_libraries(untangle-test PRIVATE untangle)
