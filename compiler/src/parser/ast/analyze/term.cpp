/**
 * @file term.cpp
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
#include "utils/vector_utils.h"

namespace pickc::parser {
  std::optional<ExpressionNode> ASTGenerator::analyzeTerm(ASTNode parentNode) {
    auto token = cur().value();
    auto left = analyzeFactor(parentNode);
    if(!left) return std::nullopt;

    while(true) {
      auto op = next();
      if(!op) {
        return left;
      }
      else if(!includes({ TokenKind::Add, TokenKind::Sub }, op.value().kind)) {
        back();
        return left;
      }

      if(!next(token, "式が必要です。")) {
        back();
        return std::nullopt;
      }

      TermNode term;
      if(op.value().kind == TokenKind::Add) {
        term = makeASTNode<AddNode>(rootNode, parentNode);
      }
      else {
        term = makeASTNode<SubNode>(rootNode, parentNode);
      }
      
      left.value()->parentNode = term;
      auto right = analyzeFactor(term);
      term->left = left.value();
      term->right = right.value();

      left = term;
    }
  }
}