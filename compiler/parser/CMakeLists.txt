cmake_minimum_required(VERSION 3.10)

add_library(
  parser
  parser.cpp
  token.cpp
  ast.cpp
  ast_node.cpp
)

target_include_directories(parser PRIVATE ${ROOT_DIR})
target_link_libraries(parser PRIVATE utils)