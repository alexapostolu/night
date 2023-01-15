cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(night VERSION 0.0.0)

configure_file("${PROJECT_SOURCE_DIR}/code/version/version.hpp.in"
			   "${PROJECT_SOURCE_DIR}/code/version/version.hpp")

include_directories("${PROJECT_SOURCE_DIR}/code/version"
					"${PROJECT_SOURCE_DIR}/code/include")

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "code/src/*.cpp")
add_executable(night ${SOURCES})

target_link_libraries(night -static)



# tests

file(GLOB_RECURSE SOURCES_TEST RELATIVE ${CMAKE_SOURCE_DIR} "tests/*.cpp")
add_executable(night_test ${SOURCES_TEST})

#target_link_libraries(night_test vtkCommon)
add_test(NAME night_test COMMAND night_test)