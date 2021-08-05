/**
 * @file generator.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief generator.hの実装
 * @version 0.1
 * @date 2021-07-24
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "generator.h"

#include <cassert>

#include "compiler_option.h"
#include "output_buffer.h"

namespace pickc::parser {
  ASTGenerator::ASTGenerator(const TokenSequence& sequence) : done(false), sequence(sequence), curIndex(0) {}
  std::optional<RootNode> ASTGenerator::generate() {
    assert(done == false);
    done = true;

    if(compilerOption.debug) {
      outputBuffer.emplace<OutputMessage>(OutputMessageKind::Debug, "Generate AST: " + sequence.filename);
    }
    rootNode = makeASTNode<RootNode>();

    bool success = true;
    while(auto token = next()) {
      switch(token->kind) {
        case TokenKind::FnKeyword:
          if(auto fn = analyzeFunction(rootNode)) {
            rootNode->functions.push_back(fn.value());
          }
          else {
            success = false;
          }
          break;
        default:
          outputBuffer.emplace<CompileErrorMessage>(sequence.filename, token->line, token->letter, token->value.length(), "宣言が必要です。");
          success = false;
      }
    }

    if(!success) {
      if(compilerOption.debug) {
        outputBuffer.emplace<OutputMessage>(OutputMessageKind::Debug, "Failed to generate AST: " + sequence.filename);
      }
      return std::nullopt;
    }

    if(compilerOption.debug) {
      outputBuffer.emplace<OutputMessage>(OutputMessageKind::Debug, "AST generation has been completed: " + sequence.filename);
      outputBuffer.emplace<OutputMessage>(OutputMessageKind::Debug, "Dump the AST: " + sequence.filename + "\n" + rootNode->dump(""));
    }
    return rootNode;
  }
  std::optional<Token> ASTGenerator::cur() {
    if(curIndex <= 0 || curIndex > sequence.tokens.size()) return std::nullopt;
    return sequence.tokens[curIndex - 1];
  }
  std::optional<Token> ASTGenerator::next() {
    if(curIndex >= sequence.tokens.size()) return std::nullopt;
    return sequence.tokens[curIndex++];
  }
  bool ASTGenerator::next(Token& token, const std::string& message) {
    if(curIndex >= sequence.tokens.size()) {
      const auto& back = sequence.tokens.back();
      outputBuffer.emplace<CompileErrorMessage>(sequence.filename, back.line, back.letter, back.value.length(), message);
      return false;
    }
    token = sequence.tokens[curIndex++];
    return true;
  }
  bool ASTGenerator::next(Token& token, TokenKind expect, const std::string& message) {
    if(curIndex >= sequence.tokens.size()) {
      const auto& back = sequence.tokens.back();
      outputBuffer.emplace<CompileErrorMessage>(sequence.filename, back.line, back.letter, back.value.length(), message);
      return false;
    }
    if(sequence.tokens[curIndex].kind != expect) {
      return false;
    }
    token = sequence.tokens[curIndex++];
    return true;
  }
  void ASTGenerator::back() {
    --curIndex;
    assert(curIndex >= 0);
  }
}