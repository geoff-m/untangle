
get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(isMultiConfig)
    if(NOT "Ubsan" IN_LIST CMAKE_CONFIGURATION_TYPES)
        list(APPEND CMAKE_CONFIGURATION_TYPES Ubsan)
    endif()
endif()

set(CMAKE_C_FLAGS_Ubsan
        "${CMAKE_C_FLAGS_DEBUG} -fsanitize=undefined -fno-omit-frame-pointer" CACHE STRING
        "Flags used by the C compiler for Ubsan build type or configuration." FORCE)

set(CMAKE_CXX_FLAGS_Ubsan
        "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined -fno-omit-frame-pointer" CACHE STRING
        "Flags used by the C++ compiler for Ubsan build type or configuration." FORCE)

set(CMAKE_EXE_LINKER_FLAGS_Ubsan
        "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=undefined" CACHE STRING
        "Linker flags to be used to create executables for Ubsan build type." FORCE)

set(CMAKE_SHARED_LINKER_FLAGS_Ubsan
        "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=undefined" CACHE STRING
        "Linker lags to be used to create shared libraries for Ubsan build type." FORCE)
