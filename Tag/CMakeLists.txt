cmake_minimum_required(VERSION 3.20)
project(UserTagLib)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(SOURCE_FILES
  usertag.c
  usertag.h
  array.c
  array.h)

add_library(usertag STATIC ${SOURCE_FILES})

target_link_libraries(usertag "-framework CoreFoundation")
set_target_properties(usertag PROPERTIES OUTPUT_NAME "usertag")