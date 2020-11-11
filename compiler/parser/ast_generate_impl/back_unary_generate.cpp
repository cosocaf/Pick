#include "ast.h"

#include "utils/vector_utils.h"

namespace pickc::parser
{
  Result<ExpressionNode*, std::vector<std::string>> ASTGenerator::backUnaryGenerate()
  {
    auto begin = currentTokenIter();
    auto base = primaryGenerate();
    if(!base) return error(base.err());
    auto result = base.get();
    while(true) {
      auto op = nextToken();
      if(!op || !includes({ TokenKind::Inc, TokenKind::Dec, TokenKind::Dot, TokenKind::LBracket, TokenKind::LParen }, op.get().kind)) {
        backToken(true);
        result->tokens = std::vector(begin, currentTokenIter() + 1);
        return ok(result);
      }
      switch(op.get().kind) {
        case TokenKind::Inc:
          result = new BackIncrementNode(result);
          break;
        case TokenKind::Dec:
          result = new BackDecrementNode(result);
          break;
        case TokenKind::Dot: {
          if(!nextToken()) {
            delete result;
            return error(createEOTError("メンバー名が必要です。"));
          }
          auto member = variableGenerate();
          if(!member) {
            delete result;
            return error(member.err());
          }
          result = new MemberAccessNode(result, member.get());
          break;
        }
        case TokenKind::LBracket: {
          if(!nextToken()) {
            delete result;
            return error(createEOTError("式が必要です。"));
          }
          auto suffix = exprGenerate();
          if(!suffix) {
            delete result;
            return error(suffix.err());
          }
          auto rb = nextToken();
          if(!rb) {
            delete result;
            return error(createEOTError("]が必要です。"));
          }
          if(rb.get().kind != TokenKind::RBracket) {
            delete result;
            return error(std::vector{ createASTError("]が必要です。", rb.get()) });
          }
          result = new ArrayAccessNode(result, suffix.get());
          break;
        }
        case TokenKind::LParen: {
          auto next = nextToken();
          if(!next) {
            delete result;
            return error(createEOTError(")が必要です。"));
          }
          std::vector<std::unique_ptr<ExpressionNode>> uargs;
          if(next.get().kind != TokenKind::RParen) {
            while(true) {
              auto arg = exprGenerate();
              if(!arg) {
                delete result;
                return error(arg.err());
              }
              uargs.emplace_back(std::unique_ptr<ExpressionNode>(arg.get()));
              next = nextToken();
              if(!next) {
                delete result;
                return error(createEOTError(")が必要です。"));
              }
              if(next.get().kind == TokenKind::RParen) break;
              if(next.get().kind == TokenKind::Comma) {
                if(!nextToken()) {
                  delete result;
                  return error(createEOTError("式が必要です。"));
                }
                continue;
              }
              delete result;
              return error(std::vector{createASTError(")が必要です。", next.get())});
            }
          }
          std::vector<ExpressionNode*> args;
          for(auto& arg : uargs) {
            args.push_back(arg.release());
          }
          result = new CallNode(result, std::move(args));
          break;
        }
        default:
          assert(false);
      }
    }
    result->tokens = std::vector(begin, currentTokenIter() + 1);
    return ok(result);
  }
}