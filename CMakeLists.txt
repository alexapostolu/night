cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(night VERSION 0.0.1)

configure_file("${PROJECT_SOURCE_DIR}/code/version/version.hpp.in"
			   "${PROJECT_SOURCE_DIR}/code/version/version.hpp")

include_directories("${PROJECT_SOURCE_DIR}/code/version"
					"${PROJECT_SOURCE_DIR}/code/include")

# Night

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_SOURCE_DIR} "code/src/*.cpp")
add_executable(night ${SOURCE_FILES})

target_link_libraries(night -static)

# Night Tests

file(GLOB_RECURSE TEST_FILES ${CMAKE_SOURCE_DIR} "tests/src/*.cpp")
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*main\\.cpp$")
add_executable(night-tests ${SOURCE_FILES} ${TEST_FILES})

target_include_directories(night-tests PRIVATE
	"${PROJECT_SOURCE_DIR}/code/include"
    "${PROJECT_SOURCE_DIR}/tests/include")

target_link_libraries(night-tests -static)
