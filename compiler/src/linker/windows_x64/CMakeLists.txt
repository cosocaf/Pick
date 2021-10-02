# Copyright (c) 2021 cosocaf

cmake_minimum_required(VERSION 3.10)

add_library(
  linker_windows_x64
  routine_compiler.cpp
  assembly.cpp
  exe_converter.cpp
  lib_loader.cpp
)

target_include_directories(linker_windows_x64 PRIVATE ${SRC_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(linker_windows_x64 PRIVATE utils)