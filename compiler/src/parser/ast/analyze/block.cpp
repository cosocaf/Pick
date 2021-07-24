#include "../generator.h"

#include <cassert>

namespace pickc::parser {
  std::optional<BlockNode> ASTGenerator::analyzeBlock(ASTNode parentNode) {
    assert(cur()->kind == TokenKind::LBrace);

    bool success = true;
    auto block = makeASTNode<BlockNode>(rootNode, parentNode);
    Token token = cur().value();
    while(next(token, "}が必要です。")) {
      if(token.kind == TokenKind::RBrase) break;
      if(auto statement = analyzeStatement(block)) {
        block->statements.push_back(statement.value());
      }
      else {
        success = false;
      }
    }

    if(!success) return std::nullopt;
    return block;
  }
}