#include "../generator.h"

namespace pickc::parser {
  std::optional<StatementNode> ASTGenerator::analyzeStatement(ASTNode parentNode) {
    auto token = cur().value();
    switch(token.kind) {
      case TokenKind::FnKeyword:
        return analyzeFunction(parentNode);
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