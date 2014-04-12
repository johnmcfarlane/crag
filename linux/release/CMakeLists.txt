# Linux release build

cmake_minimum_required(VERSION 2.8)

project(crag)

# TODO: -Weffc++ -ftemplate-backtrace-limit=0
set(CMAKE_CXX_FLAGS "-std=c++11 -O3 -pthread -DNDEBUG -DdSINGLE -D__STRICT_ANSI__ -Wall -Wextra -Wfatal-errors -fno-exceptions -fno-rtti -Wno-strict-aliasing")

#set(CMAKE_CXX_COMPILER "/usr/bin/colorgcc")
#set(CMAKE_CXX_COMPILER "/usr/bin/g++")
#set(CMAKE_CXX_COMPILER "/usr/bin/c++")
#set(CMAKE_CXX_COMPILER "/usr/bin/clang")

add_subdirectory(../../src bin)
