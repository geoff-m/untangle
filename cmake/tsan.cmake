
get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(isMultiConfig)
    if(NOT "Tsan" IN_LIST CMAKE_CONFIGURATION_TYPES)
        list(APPEND CMAKE_CONFIGURATION_TYPES Tsan)
    endif()
endif()

set(CMAKE_C_FLAGS_Tsan
        "${CMAKE_C_FLAGS_DEBUG} -fsanitize=thread -fno-omit-frame-pointer -fPIE -pie" CACHE STRING
        "Flags used by the C compiler for Tsan build type or configuration." FORCE)

set(CMAKE_CXX_FLAGS_Tsan
        "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread -fno-omit-frame-pointer -fPIE -pie" CACHE STRING
        "Flags used by the C++ compiler for Tsan build type or configuration." FORCE)

set(CMAKE_EXE_LINKER_FLAGS_Tsan
        "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=thread -fPIE -pie" CACHE STRING
        "Linker flags to be used to create executables for Tsan build type." FORCE)

set(CMAKE_SHARED_LINKER_FLAGS_Tsan
        "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=thread -fPIE -pie" CACHE STRING
        "Linker lags to be used to create shared libraries for Tsan build type." FORCE)
