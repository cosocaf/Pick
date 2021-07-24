/**
 * @file function.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief ASTGenerator::analyzeFunctionの実装
 * @version 0.1
 * @date 2021-07-24
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "../generator.h"

#include <cassert>

#include "output_buffer.h"

namespace pickc::parser {
  std::optional<FunctionNode> ASTGenerator::analyzeFunction(ASTNode parentNode) {
    assert(cur().value().kind == TokenKind::FnKeyword);

    auto fn = makeASTNode<FunctionNode>(rootNode, parentNode);
    auto token = cur().value();

    if(!next(token, "関数名が必要です。")) return std::nullopt;
    if(token.kind == TokenKind::Identify) {
      fn->name = token.value;
    }
    else if(token.kind == TokenKind::LParen) {
      back();
    }
    else {
      outputBuffer.emplace<CompileErrorMessage>(sequence.filename, token.line, token.letter, token.value.size(), "関数名が必要です。");
      return std::nullopt;
    }
    
    if(!next(token, TokenKind::LParen, "(が必要です。")) return std::nullopt;
    while(next(token, ")が必要です。")) {
      if(token.kind == TokenKind::RParen) break;
      // TODO: 引数処理
      assert(false);
    }
    if(!next(token, "式が必要です。")) return std::nullopt;

    if(auto formula = analyzeFormula(fn)) {
      fn->body = formula.value();
    }
    else {
      return std::nullopt;
    }

    return fn;
  }
}