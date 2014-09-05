# RPi debug build

cmake_minimum_required(VERSION 2.8)

project(crag)

# TODO: -Weffc++ -ftemplate-backtrace-limit=0
set(CMAKE_CXX_FLAGS "-std=c++11 -g -pthread -DPROFILE -DdSINGLE -DCRAG_RPI -D__STRICT_ANSI__ -Wall -Wextra -Wfatal-errors -fno-exceptions -fno-unwind-tables")

set(CRAG_RPI 1)
add_subdirectory(../../src bin)
