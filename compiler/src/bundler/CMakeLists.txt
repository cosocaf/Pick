# Copyright (c) 2021 cosocaf

cmake_minimum_required(VERSION 3.10)

add_library(
  bundler
  bundler.cpp
  bundle.cpp
  pcir_decoder.cpp
)

target_include_directories(bundler PRIVATE ${SRC_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(bundler PRIVATE utils)