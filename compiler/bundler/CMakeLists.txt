cmake_minimum_required(VERSION 3.10)

add_library(
  bundler
  symbol.cpp
  function.cpp
  fn_compiler.cpp
  bundler.cpp
)

target_include_directories(bundler PRIVATE ${ROOT_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(bundler PRIVATE utils)
