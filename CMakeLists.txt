cmake_minimum_required(VERSION 3.20)
project(untangle VERSION 1.3)
option(UNTANGLE_TEST "Build tests" ON)
include(cmake/asan.cmake)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(lib)
add_subdirectory(app)

if (UNTANGLE_TEST)
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
endif()

install(TARGETS untangle-app RUNTIME DESTINATION bin COMPONENT Runtime)
install(TARGETS untangle LIBRARY DESTINATION lib COMPONENT Runtime)
install(DIRECTORY lib/include/ DESTINATION include COMPONENT Development)
set(CPACK_PACKAGE_VENDOR "Geoff McQueen")
set(CPACK_PACKAGE_CONTACT "geoff@geoff.space")
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Debugging tool for pthreads")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}.${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)
