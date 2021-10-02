# Copyright (c) 2021 cosocaf

cmake_minimum_required(VERSION 3.10)

add_library(
  parser
  parser.cpp
  module_tree.cpp
  token/token.cpp
  token/tokenizer.cpp
  ast/node.cpp
  ast/generator.cpp
  ast/analyze/statement.cpp
  ast/analyze/function.cpp
  ast/analyze/var_decl.cpp
  ast/analyze/formula.cpp
  ast/analyze/control.cpp
  ast/analyze/immediate.cpp
  ast/analyze/variable.cpp
  ast/analyze/block.cpp
)

target_include_directories(parser PRIVATE ${SRC_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(parser PRIVATE utils)