cmake_minimum_required(VERSION 3.16)

add_executable(test src/test_main.cpp)

target_link_libraries(test PRIVATE mido3ds::mu)

set_property(TARGET test
	PROPERTY
		CXX_STANDARD 20
		CMAKE_CXX_STANDARD_REQUIRED YES
		CMAKE_CXX_EXTENSIONS NO
)

target_include_directories(test
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)
