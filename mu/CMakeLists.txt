cmake_minimum_required(VERSION 3.16)

set(HEADER_FILES
	include/mu/utils.h
)

set(SOURCE_FILES
	src/mu/utils.cpp
)

add_library(mu)

target_sources(mu
	PRIVATE ${HEADER_FILES}
			${SOURCE_FILES}
)

add_library(mido3ds::mu ALIAS mu)

target_include_directories(mu
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
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
