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
#include <filesystem>

#include "config.h"
#include "compiler_option.h"
#include "output_buffer.h"

#include "parser/parser.h"
#include "pcir/generator.h"
#include "bundler/bundler.h"
#include "linker/linker.h"

/**
 * @brief コンパイラのエントリポイント。
 * 
 * @param argc 実行時引数の数
 * @param argv 実行時引数の実体
 * @return int コンパイルの結果。詳しくはconfig.hのSTATUS_*を参照。
 */
int main(int argc, char* argv[]) {
  using namespace pickc;

  std::locale::global(std::locale(""));

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

  pcir::PCIRGenerator pcirGenerator(rootModule.value());
  if(!pcirGenerator.generate()) {
    outputBuffer.output(OutputMessageKind::All);
    return STATUS_PCIR_ERROR;
  }

  std::error_code e;
  std::filesystem::create_directories(compilerOption.outDir, e);
  if(e) {
    outputBuffer.emplace<InternalErrorMessage>("", "ディレクトリの作成に失敗しました。: " + compilerOption.outDir + " : " + e.message());
    outputBuffer.output(OutputMessageKind::All);
    return STATUS_INTERNAL_ERROR;
  }

  std::filesystem::path pcirFilePath = compilerOption.outDir;
  pcirFilePath /= compilerOption.projectName + ".pcir";

  if(!pcirGenerator.writeToFile(pcirFilePath.string())) {
    outputBuffer.output(OutputMessageKind::All);
    return STATUS_PCIR_ERROR;
  }

  bundler::Bundler bundler({pcirFilePath.string()});
  auto bundle = bundler.bundle();
  if(!bundle) {
    outputBuffer.output(OutputMessageKind::All);
    return STATUS_BUNDLER_ERROR;
  }

  std::filesystem::path buildFilePath = compilerOption.outDir;
  buildFilePath /= compilerOption.projectName + ".exe";

  linker::Linker linker(bundle.value());
  linker.link();
  linker.writeToFile(buildFilePath.string());

  outputBuffer.output(OutputMessageKind::All);

  return STATUS_SUCCESS;
}