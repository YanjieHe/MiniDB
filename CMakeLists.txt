cmake_minimum_required(VERSION 3.5)

project(MiniDB)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra")

include_directories(include/ libs/ test/)

file(GLOB SOURCES "src/*.cpp")
file(GLOB TESTS "test/*.cpp")

add_executable(MiniDB ${SOURCES} ${TESTS})

SET(CMAKE_BUILD_TYPE "Debug") 
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")