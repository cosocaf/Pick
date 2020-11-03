cmake_minimum_required(VERSION 3.10)

add_library(
  ssa
  converter.cpp
  ssa_struct.cpp
)

target_include_directories(ssa PRIVATE ${ROOT_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ssa PRIVATE utils)
