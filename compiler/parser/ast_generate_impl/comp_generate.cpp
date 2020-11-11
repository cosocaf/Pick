#include "ast.h"

#include "utils/vector_utils.h"

namespace pickc::parser
{
  Result<ExpressionNode*, std::vector<std::string>> ASTGenerator::compGenerate()
  {
    auto begin = currentTokenIter();
    auto left = termGenerate();
    if(!left) return error(left.err());

    auto result = left.get();
    while(true) {
      auto op = nextToken();
      if(!op || !includes({ TokenKind::Equal, TokenKind::NotEqual, TokenKind::GreaterEqual, TokenKind::GreaterThan, TokenKind::LessEqual, TokenKind::LessThan }, op.get().kind)) {
        backToken(true);
        result->tokens = std::vector(begin, currentTokenIter() + 1);
        return ok(std::move(result));
      }
      if(!nextToken()) {
        delete result;
        return error(createEOTError("式が必要です。"));
      }
      auto right = termGenerate();
      if(!right) {
        delete result;
        return error(right.err());
      }
      result->tokens = std::vector(begin, currentTokenIter() + 1);
      switch(op.get().kind) {
        case TokenKind::Equal:
          result = new EqualNode(result, right.get());
          break;
        case TokenKind::NotEqual:
          result = new NotEqualNode(result, right.get());
          break;
        case TokenKind::GreaterEqual:
          result = new GreaterEqualNode(result, right.get());
          break;
        case TokenKind::GreaterThan:
          result = new GreaterThanNode(result, right.get());
          break;
        case TokenKind::LessEqual:
          result = new LessEqualNode(result, right.get());
          break;
        case TokenKind::LessThan:
          result = new LessThanNode(result, right.get());
          break;
        default:
          assert(false);
      }
    }
  }
}