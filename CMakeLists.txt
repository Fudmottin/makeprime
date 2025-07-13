cmake_minimum_required(VERSION 3.16)
project(makeprime LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_policy(SET CMP0167 NEW)

# Find GMP via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GMP REQUIRED IMPORTED_TARGET gmp)

# Executable sources
add_executable(makeprime
    src/main.cpp
    src/gmp_mr.cpp
)

# Include + link
target_link_libraries(makeprime PRIVATE PkgConfig::GMP)

# Optimization/debug flags
if(NOT DEBUG)
    message(STATUS "Building with optimizations (-O3, -DNDEBUG)")
    target_compile_definitions(makeprime PRIVATE NDEBUG)
    target_compile_options(makeprime PRIVATE -O3 -Wall -Wextra -pedantic)
else()
    message(STATUS "Building in debug mode (-g)")
    target_compile_options(makeprime PRIVATE -g -Wall -Wextra -pedantic)
endif()

