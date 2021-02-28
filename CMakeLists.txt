cmake_minimum_required(VERSION 3.0)

project(night)

file(GLOB night_SRC "include/*.hpp" "src/*.cpp")

set(CMAKE_CXX_STANDARD 17)

add_executable(night ${night_SRC})