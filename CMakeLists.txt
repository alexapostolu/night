cmake_minimum_required(VERSION 3.5.1)

project(night)

set(EXEC night)
set(CMAKE_CXX_STANDARD 17)

include_directories(includes)
file(GLOB SOURCES src/*.cpp)

add_executable(${EXEC} ${SOURCES})
