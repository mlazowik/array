cmake_minimum_required(VERSION 3.3)
project(array)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

include_directories(BEFORE SYSTEM ../include)

set(SOURCE_FILES main.c)
add_executable(array ${SOURCE_FILES})