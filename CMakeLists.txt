cmake_minimum_required(VERSION 3.0)

project(night VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR}/code/include)

configure_file(
	"${PROJECT_SOURCE_DIR}/code/version/version.hpp.in"
	"${PROJECT_BINARY_DIR}/code/version/version.hpp"
)

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "code/src/*.cpp")
add_executable(night ${SOURCES})
target_link_libraries(night -static)
