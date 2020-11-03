cmake_minimum_required(VERSION 3.10)

add_executable(
  pickc
  main.cpp
  compiler_option.cpp
  module_tree.cpp
)

target_include_directories(pickc PRIVATE ${ROOT_DIR})
target_link_libraries(pickc PRIVATE parser pcir ssa windows_x64 utils)
