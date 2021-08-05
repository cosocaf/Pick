# Copyright (c) 2021 cosocaf

cmake_minimum_required(VERSION 3.10)

add_library(
  pcir
  generator.cpp
  text_section.cpp
  module_section.cpp
  symbol_section.cpp
  type_section.cpp
  fn_generator.cpp
  operator.cpp
  variable.cpp
  fn_section.cpp
)

target_include_directories(pcir PRIVATE ${SRC_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(pcir PRIVATE utils)