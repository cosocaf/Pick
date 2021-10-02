/**
 * @file var_decl.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief ASTGenerator::analyzeVarDeclの実装
 * @version 0.1
 * @date 2021-10-02
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "../generator.h"

#include <cassert>

#include "output_buffer.h"

namespace pickc::parser {
  std::optional<VariableDeclarationNode> ASTGenerator::analyzeVarDecl(ASTNode parentNode) {
    assert(cur().value().kind == TokenKind::DefKeyword);

    auto var = makeASTNode<VariableDeclarationNode>(rootNode, parentNode);
    var->isMut = false;
    auto token = cur().value();

    if(!next(token, "変数名が必要です。")) return std::nullopt;

    if(token.kind == TokenKind::Identify) {
      var->name = token.value;
    }
    else {
      outputBuffer.emplace<CompileErrorMessage>(sequence.filename, token.line, token.letter, token.value.size(), "変数名が必要です。");
      return std::nullopt;
    }

    if(auto res = next()) {
      if(res.value().kind == TokenKind::Asign) {
        if(!next(token, "式が必要です。")) return std::nullopt;
        auto value = analyzeExpression(var);
        if(!value) return std::nullopt;
        var->value = value.value();
      }
      else {
        back();
      }
    }

    return var;
  }
}