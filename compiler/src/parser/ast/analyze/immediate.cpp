/**
 * @file immediate.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief ASTGenerator::analyzeImmediateの実装
 * @version 0.1
 * @date 2021-07-24
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
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