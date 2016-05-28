# guessed...
cmake_minimum_required(VERSION 2.6)
PROJECT(ffffmlib C)

set(BUILD_SHARED_LIBS true)

add_library(ffffm ffffm.c)
target_link_libraries(ffffm uci)

add_library(ffffm-respondd ffffm-respondd.c)
target_link_libraries(ffffm-respondd ffffm)

INCLUDE(FindLua)

add_library(ffffm-lua ffffm-lua.c)
target_link_libraries(ffffm-lua ffffm)

add_executable(ffffm-test ffffm-test.c)
target_link_libraries(ffffm-test ffffm)

