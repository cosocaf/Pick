#include "ast.h"

#include "utils/vector_utils.h"

namespace pickc::parser
{
  Result<ExpressionNode*, std::vector<std::string>> ASTGenerator::exprGenerate()
  {
    switch(currentToken().kind) {
      case TokenKind::FnKeyword: {
        auto fn = fnDefGenerate();
        if(!fn) return error(fn.err());
        return ok(fn.get());
      }
      case TokenKind::DefKeyword:
      case TokenKind::MutKeyword: {
        auto var = varDefGenerate();
        if(!var) return error(var.err());
        return ok(var.get());
      }
      default:
        return asignGenerate();
    }
  }
}