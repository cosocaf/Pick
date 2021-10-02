/**
 * @file statement.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief ASTGenerator::analyzeStatementの実装
 * @version 0.1
 * @date 2021-07-24
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "../generator.h"

namespace pickc::parser {
  std::optional<StatementNode> ASTGenerator::analyzeStatement(ASTNode parentNode) {
    auto token = cur().value();
    switch(token.kind) {
      case TokenKind::FnKeyword:
        return analyzeFunction(parentNode);
      case TokenKind::DefKeyword:
        return analyzeVarDecl(parentNode);
      case TokenKind::ReturnKeyword:
        return analyzeControl(parentNode);
      case TokenKind::Semicolon:
        next();
        return analyzeStatement(parentNode);
      default:
        return analyzeFormula(parentNode);
    }
  }
}