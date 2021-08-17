cmake_minimum_required(VERSION 3.0)

project(night)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR})

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "night/src/*.cpp")
add_executable(night ${SOURCES})