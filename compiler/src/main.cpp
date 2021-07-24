/**
 * @file main.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief コンパイラのエントリポイント。
 * @version 0.1
 * @date 2021-07-22
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */

#include <iostream>

#include "config.h"
#include "compiler_option.h"
#include "output_buffer.h"

#include "parser/parser.h"

/**
 * @brief コンパイラのエントリポイント。
 * 
 * @param argc 実行時引数の数
 * @param argv 実行時引数の実体
 * @return int コンパイルの結果。詳しくはconfig.hのSTATUS_*を参照。
 */
int main(int argc, char* argv[]) {
  using namespace pickc;

  initCompilerOption(argc, argv);
  if(outputBuffer.has()) {
    outputBuffer.output(OutputMessageKind::All);
    return STATUS_INVALID_OPTION;
  }
  else if(compilerOption.help) {
    std::cout << USAGE << std::endl;
    return STATUS_SUCCESS;
  }
  else if(compilerOption.version) {
    std::cout << VERSION << std::endl;
    return STATUS_SUCCESS;
  }

  parser::Parser parser;
  auto rootModule = parser.parse();
  if(outputBuffer.has()) {
    outputBuffer.output(OutputMessageKind::All);
    return STATUS_PARSER_ERROR;
  }

  outputBuffer.output(OutputMessageKind::All);

  return STATUS_SUCCESS;
}