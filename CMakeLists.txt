cmake_minimum_required(VERSION 3.0)

project(night LANGUAGES CXX VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR}/code/include)

configure_file(
	${PROJECT_SOURCE_DIR}/code/include/cmakedef.hpp.in
	${PROJECT_SOURCE_DIR}/code/include/cmakedef.hpp @ONLY
)

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "code/src/*.cpp")
add_executable(night ${SOURCES} "code/src/back-end/check.cpp")
target_link_libraries(night -static)
