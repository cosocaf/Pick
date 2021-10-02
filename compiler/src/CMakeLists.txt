# Copyright (c) 2021 cosocaf

cmake_minimum_required(VERSION 3.10)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/parser)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/pcir)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/bundler)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/linker)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/utils)

add_executable(
  pickc
  main.cpp
  compiler_option.cpp
  output_buffer.cpp
)

target_include_directories(pickc PRIVATE ${SRC_DIR})
target_link_libraries(
  pickc PRIVATE
  parser
  pcir
  bundler
  linker
  utils
)
target_sources(pickc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pickc.exe.manifest)