cmake_minimum_required(VERSION 3.10)
project(sockettest C)

set(CMAKE_C_STANDARD 99)

add_executable(server_echo_itor server_echo_itor.c)
add_executable(server_echo_curr server_echo_curr.c)


add_executable(client1 client1.c)
add_executable(client2 client2.c)
add_executable(sig sig.c)