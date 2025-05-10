cmake_minimum_required(VERSION 3.16)
project(makeprime LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/FetchMillerRabin.cmake)

find_package(Boost REQUIRED COMPONENTS random)

add_executable(makeprime main.cpp)
target_include_directories(makeprime PRIVATE ${MILLER_RABIN_SOURCE_DIR})
target_sources(makeprime PRIVATE ${MILLER_RABIN_SOURCE_DIR}/miller_rabin.cpp)
target_link_libraries(makeprime PRIVATE Boost::random)

