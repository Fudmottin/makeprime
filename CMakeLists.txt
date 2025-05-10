cmake_minimum_required(VERSION 3.16)
project(makeprime LANGUAGES CXX)

# Set modern C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Suppress CMP0167 warning
cmake_policy(SET CMP0167 NEW)

# Dependencies
include(cmake/FetchMillerRabin.cmake)
find_package(Boost REQUIRED COMPONENTS random)

# Target
add_executable(makeprime main.cpp)
target_sources(makeprime PRIVATE ${MILLER_RABIN_SOURCE_DIR}/miller_rabin.cpp)
target_include_directories(makeprime PRIVATE ${MILLER_RABIN_SOURCE_DIR})
target_link_libraries(makeprime PRIVATE Boost::random)

# Optimization and debug settings
if(NOT DEBUG)
    message(STATUS "Building with optimizations (-O3, -DNDEBUG)")
    target_compile_definitions(makeprime PRIVATE NDEBUG)
    target_compile_options(makeprime PRIVATE -O3 -Wall -Wextra -pedantic)
else()
    message(STATUS "Building in debug mode (-g)")
    target_compile_options(makeprime PRIVATE -g -Wall -Wextra -pedantic)
endif()

