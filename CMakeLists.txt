cmake_minimum_required(VERSION 2.8)
project(Assign3)

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
endif()

add_executable(Assign3 main.cpp)
