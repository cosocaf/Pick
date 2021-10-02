/**
 * @file primary_expression.cpp
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
  std::optional<ExpressionNode> ASTGenerator::analyzePrimaryExpression(ASTNode parentNode) {
    auto token = cur().value();
    switch(token.kind) {
      case TokenKind::LBrace:
        return analyzeBlock(parentNode);
      case TokenKind::Integer:
        return analyzeImmediate(parentNode);
      case TokenKind::Identify:
        return analyzeVariable(parentNode);
      default:
        outputBuffer.emplace<CompileErrorMessage>(sequence.filename, token.line, token.letter, token.value.size(), "式ではありません。");
        return std::nullopt;
    }
  }
}