cmake_minimum_required(VERSION 3.3)
project(array)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

include_directories(BEFORE SYSTEM ../include)

set(SERVER_SOURCE_FILES server.c)
add_executable(serwer ${SERVER_SOURCE_FILES})

set(CLIENT_SOURCE_FILES client.c)
add_executable(klient ${CLIENT_SOURCE_FILES})