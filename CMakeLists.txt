cmake_minimum_required(VERSION 3.10)
project(sockettest C)

set(CMAKE_C_STANDARD 99)

add_executable(server server_echo_itor.c)
add_executable(client client.c)
add_executable(test1 sigtest.c)