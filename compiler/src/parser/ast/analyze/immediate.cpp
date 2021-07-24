#include "../generator.h"

#include <cassert>

namespace pickc::parser {
  std::optional<ImmediateNode> ASTGenerator::analyzeImmediate(ASTNode parentNode) {
    auto token = cur().value();

    switch(token.kind) {
      case TokenKind::Integer:
        return makeASTNode<IntegerNode>(rootNode, parentNode, std::stoi(token.value));
      default:
        assert(false);
    }
    return std::nullopt;
  }
}