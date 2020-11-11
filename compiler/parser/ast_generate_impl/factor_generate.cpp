#include "ast.h"

#include "utils/vector_utils.h"

namespace pickc::parser
{
  Result<ExpressionNode*, std::vector<std::string>> ASTGenerator::factorGenerate()
  {
    auto begin = currentTokenIter();
    auto left = frontUnaryGenerate();
    if(!left) return error(left.err());

    auto result = left.get();
    while(true) {
      auto op = nextToken();
      if(!op || !includes({ TokenKind::Asterisk, TokenKind::Slash, TokenKind::Percent }, op.get().kind)) {
        backToken(true);
        result->tokens = std::vector(begin, currentTokenIter() + 1);
        return ok(result);
      }
      if(!nextToken()) {
        delete result;
        return error(createEOTError("式が必要です。"));
      }
      auto right = frontUnaryGenerate();
      if(!right) {
        delete result;
        return error(right.err());
      }
      result->tokens = std::vector(begin, currentTokenIter() + 1);
      switch(op.get().kind) {
        case TokenKind::Asterisk:
          result = new MulNode(result, right.get());
          break;
        case TokenKind::Slash:
          result = new DivNode(result, right.get());
          break;
        case TokenKind::Percent:
          result = new ModNode(result, right.get());
          break;
        default:
          assert(false);
      }
    } 
  }
}