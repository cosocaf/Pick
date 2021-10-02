/**
 * @file variable.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-10-02
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "../generator.h"

#include <cassert>

namespace pickc::parser {
  std::optional<VariableNode> ASTGenerator::analyzeVariable(ASTNode parentNode) {
    assert(cur().value().kind == TokenKind::Identify);

    auto var = makeASTNode<VariableNode>(rootNode, parentNode);
    auto token = cur().value();
    var->name = token.value;

    return var;
  }
}