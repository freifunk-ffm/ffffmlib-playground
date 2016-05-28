# guessed...
cmake_minimum_required(VERSION 2.6)
PROJECT(ffffmlib C)

set(BUILD_SHARED_LIBS true)

INCLUDE(FindPkgConfig)
pkg_search_module(LUA REQUIRED lua5.1)

add_library(ffffm ffffm.c)
target_link_libraries(ffffm uci)

add_library(ffffm-respondd ffffm-respondd.c)
target_link_libraries(ffffm-respondd ffffm)

add_library(ffffm-lua ffffm-lua.c)
target_include_directories(ffffm-lua PUBLIC ${LUA_INCLUDE_DIRS})
target_link_libraries(ffffm-lua ffffm ${LUA_LIBRARIES})

add_executable(ffffm-test ffffm-test.c)
target_link_libraries(ffffm-test ffffm)

