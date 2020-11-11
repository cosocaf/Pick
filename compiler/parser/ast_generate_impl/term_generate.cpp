#include "ast.h"

#include "utils/vector_utils.h"

namespace pickc::parser
{
  Result<ExpressionNode*, std::vector<std::string>> ASTGenerator::termGenerate()
  {
    auto begin = currentTokenIter();
    auto left = factorGenerate();
    if(!left) return error(left.err());

    auto result = left.get();
    while(true) {
      auto op = nextToken();
      if(!op || !includes({ TokenKind::Plus, TokenKind::Minus }, op.get().kind)) {
        backToken(true);
        result->tokens = std::vector(begin, currentTokenIter() + 1);
        return ok(result);
      }
      if(!nextToken()) {
        delete result;
        return error(createEOTError("式が必要です。"));
      }
      auto right = factorGenerate();
      if(!right) {
        delete result;
        return error(right.err());
      }
      result->tokens = std::vector(begin, currentTokenIter() + 1);
      switch(op.get().kind) {
        case TokenKind::Plus:
          result = new AddNode(result, right.get());
          break;
        case TokenKind::Minus:
          result = new SubNode(result, right.get());
          break;
        default:
          assert(false);
      }
    }
  }
}