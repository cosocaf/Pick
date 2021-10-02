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
  ast/analyze/expression.cpp
  ast/analyze/primary_expression.cpp
  ast/analyze/control.cpp
  ast/analyze/immediate.cpp
  ast/analyze/variable.cpp
  ast/analyze/block.cpp
  ast/analyze/back_unary_expression.cpp
  ast/analyze/front_unary_expression.cpp
  ast/analyze/factor.cpp
  ast/analyze/term.cpp
  ast/analyze/compare.cpp
  ast/analyze/asign.cpp
)

target_include_directories(parser PRIVATE ${SRC_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(parser PRIVATE utils)