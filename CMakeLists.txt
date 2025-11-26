
cmake_minimum_required(VERSION 3.10)
project(Xsh C)

find_package(Curses REQUIRED)
add_library(xshell SHARED libxshell.c)
add_compile_options(-Wno-implicit-function-declaration)
add_compile_options(-Wno-int-conversion)
add_executable(xsh xsh.c)
target_link_libraries(xsh PRIVATE xshell)

