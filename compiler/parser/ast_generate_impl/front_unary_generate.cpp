#include "ast.h"

#include "utils/vector_utils.h"

namespace pickc::parser
{
  Result<ExpressionNode*, std::vector<std::string>> ASTGenerator::frontUnaryGenerate()
  {
    if(includes({ TokenKind::Plus, TokenKind::Minus, TokenKind::Inc, TokenKind::Dec }, currentToken().kind)) {
      auto begin = currentTokenIter();
      const auto op = currentToken().kind;
      if(!nextToken()) return error(createEOTError("式が必要です。"));
      auto base = frontUnaryGenerate();
      if(!base) return error(base.err());
      FrontUnaryNode* node = nullptr;
      switch(op) {
        case TokenKind::Plus:
          node = new PlusNode(base.get());
          break;
        case TokenKind::Minus:
          node = new MinusNode(base.get());
          break;
        case TokenKind::Inc:
          node = new FrontIncrementNode(base.get());
          break;
        case TokenKind::Dec:
          node = new FrontDecrementNode(base.get());
          break;
        case TokenKind::Copy:
          node = new CopyNode(base.get());
          break;
        default:
          assert(false);
      }
      node->tokens = std::vector(begin, currentTokenIter() + 1);
      return ok(node);
    }
    return backUnaryGenerate();
  }
}