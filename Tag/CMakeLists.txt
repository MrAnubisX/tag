cmake_minimum_required(VERSION 3.20)
project(TagCore)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(SOURCE_FILES
  tag.c
  tag.h
  array.c
  array.h)

add_library(TagCore STATIC ${SOURCE_FILES})
target_link_libraries(TagCore "-framework CoreFoundation")

set_target_properties(TagCore PROPERTIES OUTPUT_NAME "tag")