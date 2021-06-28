cmake_minimum_required(VERSION 3.12)

project(synth)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(${PROJECT_NAME} main.cpp)