cmake_minimum_required(VERSION 3.16)
project(MU VERSION 0.1.0 LANGUAGES CXX C)

if(APPLE)
    enable_language(OBJC)
endif()

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

add_library(mu
	src/utils.h
	src/utils.cpp
)

target_link_libraries(mu
	PUBLIC
		fmt::fmt
		$<$<PLATFORM_ID:Windows>:dbghelp>
		${CMAKE_DL_LIBS}
)

set_property(TARGET mu
	PROPERTY
		CXX_STANDARD 20
		CMAKE_CXX_STANDARD_REQUIRED YES
		CMAKE_CXX_EXTENSIONS NO
)

target_compile_definitions(mu
	PUBLIC
		ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
		$<$<PLATFORM_ID:Windows>:UNICODE;_UNICODE>
		$<$<PLATFORM_ID:Windows>:OS_WINDOWS=1>
		$<$<PLATFORM_ID:Linux>:OS_LINUX=1>
		$<$<PLATFORM_ID:Darwin>:OS_MACOS=1>
		$<$<CXX_COMPILER_ID:Clang>:COMPILER_CLANG=1>
		$<$<CXX_COMPILER_ID:AppleClang>:COMPILER_CLANG=1>
		$<$<CXX_COMPILER_ID:AppleClang>:COMPILER_APPLE_CLANG=1>
		$<$<CXX_COMPILER_ID:GNU>:COMPILER_GNU=1>
		$<$<CXX_COMPILER_ID:MSVC>:COMPILER_MSVC=1>
		$<$<CONFIG:DEBUG>:DEBUG>
)

if (${MU_PEDANTIC_BUILD})
	target_compile_options(mu
		PUBLIC
			$<$<CXX_COMPILER_ID:MSVC>:/W4 /NODEFAULTLIB:library>
			$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Wno-nested-anon-types>
	)
endif()
