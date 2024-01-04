cmake_minimum_required(VERSION 3.16)
project(MU VERSION 0.1.0 LANGUAGES CXX C)

if(APPLE)
    enable_language(OBJC)
endif()

set(MASTER_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	set(MASTER_PROJECT ON)
endif ()

option(MU_BUILD_TESTS    "Build mu tests."                      ${MASTER_PROJECT})
option(MU_PEDANTIC_BUILD "Enable pedantic warnings during build" OFF)

if (CMAKE_BUILD_TYPE STREQUAL "")
	set(CMAKE_BUILD_TYPE Debug)
endif()

if (CPM_SOURCE_CACHE STREQUAL "")
	message(WARNING "CPM_SOURCE_CACHE is empty, this will download locally all packages")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_BUILD_RPATH "$ORIGIN")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CPM)
CPMAddPackage(
	NAME             fmt
	GIT_REPOSITORY   https://github.com/fmtlib/fmt.git
	GIT_TAG          10.0.0
	GIT_SHALLOW      TRUE
)

add_subdirectory(mu)

if (MU_BUILD_TESTS)
	add_subdirectory(test)
endif()
