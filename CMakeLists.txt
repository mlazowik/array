cmake_minimum_required(VERSION 3.3)
project(array)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pthread")

include_directories(BEFORE SYSTEM ../include)

set(COMMON_SOURCE_FILES err.c)

set(SERVER_SOURCE_FILES server.c ${COMMON_SOURCE_FILES})
add_executable(serwer ${SERVER_SOURCE_FILES})

set(CLIENT_SOURCE_FILES client.c ${COMMON_SOURCE_FILES})
add_executable(klient ${CLIENT_SOURCE_FILES})