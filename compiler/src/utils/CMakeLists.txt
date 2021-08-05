cmake_minimum_required(VERSION 3.10)

add_library(
  utils
  string_utils.cpp
  binary_vec.cpp
  file_id.cpp
)

target_include_directories(utils PRIVATE ${SRC_DIR})