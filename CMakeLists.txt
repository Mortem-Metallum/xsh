cmake_minimum_required(VERSION 3.10)
project(Xsh C)

add_library(xshell SHARED libxshell.c)
add_executable(xsh xsh.c)
target_link_libraries(xsh PRIVATE xshell)