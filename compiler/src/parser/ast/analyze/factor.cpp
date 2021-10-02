/**
 * @file factor.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-10-03
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "../generator.h"

#include "output_buffer.h"

namespace pickc::parser {
  std::optional<ExpressionNode> ASTGenerator::analyzeFactor(ASTNode parentNode) {
    auto left = analyzeFrontUnaryExpression(parentNode);
    // TODO: implement factor nodes.
    return left;
  }
}