/**
 * @file formula.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief ASTGenerator::analyzeExpressionの実装
 * @version 0.1
 * @date 2021-07-24
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "../generator.h"

#include "output_buffer.h"

namespace pickc::parser {
  std::optional<ExpressionNode> ASTGenerator::analyzeExpression(ASTNode parentNode) {
    return analyzeAsign(parentNode);
  }
}