/**
 * @file parser.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief parser.hの実装
 * @version 0.1
 * @date 2021-07-22
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "parser.h"

#include <filesystem>

#include "token/tokenizer.h"
#include "ast/generator.h"

#include "compiler_option.h"
#include "output_buffer.h"
#include "utils/string_utils.h"

namespace pickc::parser {
  using dir_iter = std::filesystem::directory_iterator;

  Parser::Parser() : rootModule(std::make_shared<_ModuleTree>(compilerOption.projectName, compilerOption.srcDir, true, nullptr)) {}
  void Parser::init() {
    constructModuleTree(compilerOption.srcDir, rootModule);
  }
  void Parser::constructModuleTree(const std::string& dir, ModuleTree& module) {
    dir_iter itr(dir), end;
    std::error_code err;
    while(itr != end && !err) {
      auto entry = *itr;
      auto name = entry.path().filename().string();
      auto filename = entry.path().string();
      if(entry.is_directory()) {
        constructModuleTree(filename, module->push(name, filename, true));
      }
      else if(entry.is_regular_file() && endsWith(name, ".pick")) {
        module->push(name.substr(0, name.size() - 5), filename, false);
      }
      itr.increment(err);
    }
    if(err) {
      outputBuffer.emplace<InternalErrorMessage>(dir, "モジュールツリーの構築中にシステムエラーが発生しました。: " + err.message());
    }
  }
  std::optional<ModuleTree> Parser::parse() {
    init();
    if(outputBuffer.has()) return std::nullopt;

    tokenize(rootModule);
    if(outputBuffer.has()) return std::nullopt;

    ast(rootModule);
    if(outputBuffer.has()) return std::nullopt;

    return rootModule;
  }
  void Parser::tokenize(ModuleTree& module) {
    if(!module->isDirectoryModule()) {
      Tokenizer tokenizer(module->getFilename());
      if(auto sequence = tokenizer.tokenize()) {
        module->setTokenSequence(sequence.value());
      }
    }
    for(auto& submodule : *module) {
      tokenize(submodule);
    }
  }
  void Parser::ast(ModuleTree& module) {
    if(!module->isDirectoryModule()) {
      ASTGenerator generator(module->getTokenSequence());
      if(auto rootNode = generator.generate()) {
        module->setRootNode(rootNode.value());
      }
    }
    for(auto& submodule : *module) {
      ast(submodule);
    }
  }
}