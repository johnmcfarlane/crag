# Linux release build

cmake_minimum_required(VERSION 2.8)

project(crag)

# TODO: -Weffc++ -ftemplate-backtrace-limit=0
set(CMAKE_CXX_FLAGS "-std=c++11 -O3 -pthread -DNDEBUG -DdSINGLE -DCRAG_RPI -D__STRICT_ANSI__ -Wall -Wextra -Wfatal-errors -fno-exceptions -fno-rtti -Wno-strict-aliasing")

add_subdirectory(../../src bin)
