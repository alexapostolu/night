cmake_minimum_required(VERSION 3.0)

project(night)

file(GLOB night_SRC CONFIGURE_DEPENDS "include/*.hpp" "src/*.cpp")

add_executable(night ${night_SRC})