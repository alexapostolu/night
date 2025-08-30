cmake_minimum_required(VERSION 3.16.0)
project(night VERSION 0.0.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
	message(STATUS "Configuring for Windows")
elseif(APPLE)
	message(STATUS "Configuring for macOS")
elseif(UNIX)
	message(STATUS "Configuring for Linux")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
	set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")
endif()

configure_file("${CMAKE_SOURCE_DIR}/code/version/version.hpp.in"
			   "${CMAKE_SOURCE_DIR}/code/version/version.hpp")

include_directories(
	"${CMAKE_SOURCE_DIR}/code/version"
	"${CMAKE_SOURCE_DIR}/code/include"
)

# --- Night main binary ---

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/code/src/*.cpp")
add_executable(night ${SOURCE_FILES})

if (UNIX AND NOT APPLE)
	target_link_libraries(night PRIVATE -static)
endif()

# --- Night tests binary ---

file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tests/src/*.cpp")

# Exclude main.cpp (contains the main() function) from test sources
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*main\\.cpp$")

add_executable(night-tests ${SOURCE_FILES} ${TEST_FILES})

target_include_directories(night-tests PRIVATE
	"${CMAKE_SOURCE_DIR}/code/include"
	"${CMAKE_SOURCE_DIR}/tests/include"
)

if (UNIX AND NOT APPLE)
	target_link_libraries(night-tests PRIVATE -static)
endif()
