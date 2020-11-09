#include "ast.h"

#include "utils/vector_utils.h"

namespace pickc::parser
{
  Result<ExpressionNode*, std::vector<std::string>> ASTGenerator::asignGenerate()
  {
    auto begin = currentTokenIter();
    auto left = compGenerate();
    if(!left) return error(left.err());
    auto op = nextToken();
    if(!op || !includes({ TokenKind::Asign, TokenKind::AddAsign, TokenKind::SubAsign, TokenKind::MulAsign, TokenKind::DivAsign, TokenKind::ModAsign }, op.get().kind)) {
      backToken(true);
      left.get()->tokens = std::vector(begin, currentTokenIter() + 1);
      return ok(left.get());
    }

    if(!nextToken()) {
      delete left.get();
      return error(createEOTError("式が必要です。"));
    }
    auto right = exprGenerate();
    if(!right) {
      delete left.get();
      return error(right.err());
    }
    AsignNode* node = nullptr;
    switch(op.get().kind) {
      case TokenKind::Asign:
        node = new AsignNode(left.get(), right.get());
        break;
      case TokenKind::AddAsign:
        node = new AddAsignNode(left.get(), right.get());
        break;
      case TokenKind::SubAsign:
        node = new SubAsignNode(left.get(), right.get());
        break;
      case TokenKind::MulAsign:
        node = new MulAsignNode(left.get(), right.get());
        break;
      case TokenKind::DivAsign:
        node = new DivAsignNode(left.get(), right.get());
        break;
      case TokenKind::ModAsign:
        node = new ModAsignNode(left.get(), right.get());
        break;
      default:
        assert(false);
    }
    node->tokens = std::vector(begin, currentTokenIter() + 1);
    return ok(node);
  }
}