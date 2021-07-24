#include "../generator.h"

#include <cassert>

namespace pickc::parser {
  std::optional<ControlNode> ASTGenerator::analyzeControl(ASTNode parentNode) {
    auto token = cur().value();
    assert(token.kind == TokenKind::ReturnKeyword);

    switch(token.kind) {
      case TokenKind::ReturnKeyword: {
        auto ret = makeASTNode<ControlNode>(rootNode, parentNode, ControlNodeKind::Return);
        if(!next(token, "式が必要です。")) {
          return std::nullopt;
        }
        if(token.kind == TokenKind::Semicolon) {
          // TODO: return void
          assert(false);
        }
        else {
          if(auto formula = analyzeFormula(ret)) {
            ret->formula = std::move(formula.value());
          }
          else {
            return std::nullopt;
          }
        }
        if(!next(token, TokenKind::Semicolon, ";が必要です。")) {
          return std::nullopt;
        }
        return ret;
      }
      default:
        assert(false);
    }
    return std::nullopt;
  }
}