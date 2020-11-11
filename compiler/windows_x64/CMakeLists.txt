cmake_minimum_required(VERSION 3.10)

add_library(
  windows_x64
  routine.cpp
  routine_compiler.cpp
  compiler.cpp
  linker.cpp
  lib_loader.cpp
  type.cpp
  operation_impl/binary_operations.cpp
  operation_impl/mul_div_operations.cpp
  operation_impl/mov_operation.cpp
  operation_impl/neg_operation.cpp
  operation_impl/convert_operations.cpp
  operation_impl/lea_operation.cpp
  operation_impl/push_pop_operation.cpp
  operation_impl/epilogue_operations.cpp
  operation_impl/call_operation.cpp
  operation_impl/jmp_operations.cpp
)

target_include_directories(windows_x64 PRIVATE ${ROOT_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(windows_x64 PRIVATE utils)
