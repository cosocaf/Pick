cmake_minimum_required(VERSION 3.10)

add_library(
  parser
  parser.cpp
  token.cpp
  ast.cpp
  ast_node.cpp
  ast_generate_impl/als_def_generate.cpp
  ast_generate_impl/arg_def_generate.cpp
  ast_generate_impl/asign_generate.cpp
  ast_generate_impl/back_unary_generate.cpp
  ast_generate_impl/cls_def_generate.cpp
  ast_generate_impl/comp_generate.cpp
  ast_generate_impl/expr_generate.cpp
  ast_generate_impl/extern_generate.cpp
  ast_generate_impl/factor_generate.cpp
  ast_generate_impl/fn_def_generate.cpp
  ast_generate_impl/front_unary_generate.cpp
  ast_generate_impl/import_generate.cpp
  ast_generate_impl/primary_generate.cpp
  ast_generate_impl/term_generate.cpp
  ast_generate_impl/type_generate.cpp
  ast_generate_impl/var_def_generate.cpp
  ast_generate_impl/var_generate.cpp
)

target_include_directories(parser PRIVATE ${ROOT_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(parser PRIVATE utils)