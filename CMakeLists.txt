cmake_minimum_required(VERSION 3.16)
project(makeprime LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_policy(SET CMP0167 NEW)

# GMP detection
find_package(PkgConfig QUIET)

if(PkgConfig_FOUND)
    pkg_check_modules(GMP IMPORTED_TARGET gmp)
    if(GMP_FOUND)
        message(STATUS "Using GMP from pkg-config")
        set(USE_PKGCONFIG_GMP ON)
    endif()
endif()

if(NOT GMP_FOUND)
    message(STATUS "Falling back to manual GMP detection")

    find_path(GMP_INCLUDE_DIR NAMES gmp.h)
    find_library(GMP_LIBRARY NAMES gmp)

    if(NOT GMP_INCLUDE_DIR OR NOT GMP_LIBRARY)
        message(FATAL_ERROR "GMP library and headers not found")
    endif()

    add_library(GMP::GMP UNKNOWN IMPORTED)
    set_target_properties(GMP::GMP PROPERTIES
        IMPORTED_LOCATION "${GMP_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${GMP_INCLUDE_DIR}"
    )
endif()

# Executable
add_executable(makeprime
    src/main.cpp
)

# Link to GMP
if(USE_PKGCONFIG_GMP)
    target_link_libraries(makeprime PRIVATE PkgConfig::GMP)
else()
    target_link_libraries(makeprime PRIVATE GMP::GMP)
endif()

# Compiler flags
if(NOT DEBUG)
    message(STATUS "Building with optimizations (-O3, -DNDEBUG)")
    target_compile_definitions(makeprime PRIVATE NDEBUG)
    target_compile_options(makeprime PRIVATE -O3 -Wall -Wextra -pedantic)
else()
    message(STATUS "Building in debug mode (-g)")
    target_compile_options(makeprime PRIVATE -g -Wall -Wextra -pedantic)
    target_link_options(makeprime PRIVATE -fsanitize=address)
endif()

