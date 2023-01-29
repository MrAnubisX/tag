cmake_minimum_required(VERSION 3.20)
project(Tag C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_VERBOSE_MAKEFILE ON)

add_executable(tag main.c)

include_directories(Tag)
add_subdirectory(Tag)

target_link_libraries(tag TagCore)
