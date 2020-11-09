cmake_minimum_required(VERSION 3.10)

add_library(
  pcir
  pcir.cpp
  module_analyzer.cpp
  semantic_analyzer.cpp
  pcir_struct.cpp
  pcir_dump.cpp
  module_analyzer_impl/expr_analyze.cpp
  module_analyzer_impl/block_analyze.cpp
  module_analyzer_impl/var_analyze.cpp
  module_analyzer_impl/literal_analyze.cpp
  module_analyzer_impl/unary_analyze.cpp
  module_analyzer_impl/binary_analyze.cpp
  module_analyzer_impl/var_def_analyze.cpp
  module_analyzer_impl/fn_def_analyze.cpp
  module_analyzer_impl/if_analyze.cpp
)

target_include_directories(pcir PRIVATE ${ROOT_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(pcir PRIVATE utils)
