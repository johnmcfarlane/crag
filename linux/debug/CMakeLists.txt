# Linux debug build

cmake_minimum_required(VERSION 2.8)

project(crag)

# TODO: -Weffc++ -fno-unwind-tables
set(CMAKE_CXX_FLAGS "-std=c++11 -g -pthread -DdSINGLE -DPROFILE -D__STRICT_ANSI__ -Wall -Wextra -Wfatal-errors -fno-rtti -fno-exceptions")

#include_directories("src" "dependencies/include")
#set(CMAKE_CXX_COMPILER "/usr/bin/colorgcc")
#set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_COMPILER "/usr/bin/clang")

add_subdirectory(../../src bin)
