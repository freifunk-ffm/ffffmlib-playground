# guessed...
cmake_minimum_required(VERSION 2.6)
PROJECT(ffffmlib C)

INCLUDE(CMakeDependentOption)

option(BUILD_SHARED_LIBS "Use shared libraries" OFF)
option(RESPONDD "Enable respondd support" ON)
option(LUA "Enable lua support" ON)
CMAKE_DEPENDENT_OPTION(ALFRED "Enable alfred support" ON "NOT LUA" OFF)

set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
set(LUA_PACKAGE lua5.1 lua)
set(CMAKE_C_FLAGS "-O3 -Wall --std=c99 -g3 -Wpedantic")

INCLUDE(FindPkgConfig)
pkg_search_module(UNL REQUIRED libnl-tiny)

if(LUA)
	pkg_search_module(LUA REQUIRED ${LUA_PACKAGE})

	if(NOT LUAPATH)
		if(DEFINED pkg_get_variable)
			pkg_get_variable(LUAPATH ${LUA_PACKAGE} INSTALL_CMOD)
		else()
			execute_process(
				COMMAND  lua -e "for k in string.gmatch(package.cpath .. \";\", \"([^;]+)/..so;\") do if k:sub(1,1) == \"/\" then print(k) break end end"
				OUTPUT_VARIABLE LUAPATH
				RESULT_VARIABLE LUA_CHECK_RES
				OUTPUT_STRIP_TRAILING_WHITESPACE
				)
		endif()
	endif()

endif()

if(RESPONDD)
	pkg_search_module(JSON REQUIRED json-c)
endif()

add_library(ffffm ffffm.c)
target_link_libraries(ffffm uci ${UNL_LIBRARIES})
target_include_directories(ffffm PUBLIC ${UNL_INCLUDE_DIRS})
install(TARGETS ffffm
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib/static)

if(RESPONDD)
	add_library(ffffm-respondd SHARED ffffm-respondd.c) # FIXME make this a module
	target_link_libraries(ffffm-respondd ffffm)
	target_link_libraries(ffffm-respondd ffffm ${JSON_LIBRARIES})
	target_include_directories(ffffm-respondd PUBLIC ${JSON_INCLUDE_DIRS})
	install(FILES $<TARGET_FILE:ffffm-respondd>
		RENAME ffffm.so
		DESTINATION lib/gluon/respondd/)
endif()

if(LUA)
	add_library(ffffm-lua SHARED ffffm-lua.c) # FIXME make this a module
	target_include_directories(ffffm-lua PUBLIC ${LUA_INCLUDE_DIRS})
	target_link_libraries(ffffm-lua ffffm ${LUA_LIBRARIES})
	install(FILES $<TARGET_FILE:ffffm-lua>
		RENAME ffffm.so
		DESTINATION ${LUAPATH})
endif()

if(ALFRED)
	install(FILES gluon-announce.lua
		RENAME ffffm
		DESTINATION lib/gluon/announce/nodeinfo.d)
endif()

enable_testing()

add_executable(ffffm-test ffffm-test.c)
target_link_libraries(ffffm-test ffffm)
add_test(NAME ffffm-test COMMAND ffffm-test)

if(RESPONDD)
	add_executable(ffffm-respondd-test ffffm-respondd-test.c)
	target_link_libraries(ffffm-respondd-test ffffm-respondd)
	add_test(NAME ffffm-respondd-test COMMAND ffffm-respondd-test)
endif()

if(LUA)
	add_test(NAME ffffm-lua-test COMMAND ffffm-lua-test)
endif()
