# Copyright (c) 2021 cosocaf

cmake_minimum_required(VERSION 3.10)

add_library(
  linker
  linker.cpp
  routine_converter.cpp
  operator.cpp
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/windows_x64)

target_include_directories(linker PRIVATE ${SRC_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(linker PRIVATE bundler linker_windows_x64 utils)