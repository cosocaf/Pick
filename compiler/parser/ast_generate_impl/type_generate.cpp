#include "ast.h"

#include "utils/vector_utils.h"

namespace pickc::parser
{
  Result<TypeNode*, std::vector<std::string>> ASTGenerator::typeGenerate()
  {
    auto begin = currentTokenIter();
    std::unique_ptr<TypeNode> type;
    std::vector<std::string> errors;
    switch(currentToken().kind) {
      case TokenKind::I8Keyword:
        type = std::make_unique<I8TypeNode>();
        break;
      case TokenKind::I16Keyword:
        type = std::make_unique<I16TypeNode>();
        break;
      case TokenKind::I32Keyword:
        type = std::make_unique<I32TypeNode>();
        break;
      case TokenKind::I64Keyword:
        type = std::make_unique<I64TypeNode>();
        break;
      case TokenKind::U8Keyword:
        type = std::make_unique<U8TypeNode>();
        break;
      case TokenKind::U16Keyword:
        type = std::make_unique<U16TypeNode>();
        break;
      case TokenKind::U32Keyword:
        type = std::make_unique<U32TypeNode>();
        break;
      case TokenKind::U64Keyword:
        type = std::make_unique<U64TypeNode>();
        break;
      case TokenKind::F32Keyword:
        type = std::make_unique<F32TypeNode>();
        break;
      case TokenKind::F64Keyword:
        type = std::make_unique<F64TypeNode>();
        break;
      case TokenKind::CharKeyword:
        type = std::make_unique<CharTypeNode>();
        break;
      case TokenKind::BoolKeyword:
        type = std::make_unique<BoolTypeNode>();
        break;
      case TokenKind::VoidKeyword:
        type = std::make_unique<VoidTypeNode>();
        break;
      case TokenKind::PtrKeyword: {
        auto next = nextToken();
        if(!next) return error(createEOTError("<が必要です。"));
        if(next.get().kind != TokenKind::LessThan) return error(std::vector{ createASTError("<が必要です。", next.get()) });
        if(!nextToken()) return error(createEOTError("型名が必要です。"));
        auto base = typeGenerate();
        if(!base) return error(base.err());
        next = nextToken();
        if(!next) return error(createEOTError(">が必要です。"));
        if(next.get().kind != TokenKind::GreaterThan) return error(std::vector{ createASTError(">が必要です。", next.get()) });
        type = std::make_unique<PtrTypeNode>(base.get());
        break;
      }
      case TokenKind::FnKeyword: {
        auto next = nextToken();
        if(!next) return error(createEOTError("(が必要です。"));
        if(next.get().kind != TokenKind::LParen) {
          errors.push_back(createASTError("(が必要です。", next.get()));
          break;
        }
        next = nextToken();
        if(!next) return error(errors + createEOTError(")が必要です。"));
        std::vector<std::unique_ptr<TypeNode>> uargs;
        if(next.get().kind != TokenKind::RParen) {
          while(true) {
            auto arg = typeGenerate();
            if(!arg) {
              errors += arg.err();
              break;
            }
            uargs.push_back(std::unique_ptr<TypeNode>(arg.get()));
            next = nextToken();
            if(!next) return error(errors + createEOTError(")が必要です。"));
            if(next.get().kind == TokenKind::RParen) break;
            else if(next.get().kind == TokenKind::Comma) next = nextToken();
            else errors.push_back(createASTError(")が必要です。", next.get()));
          }
          if(!errors.empty()) break;
        }
        next = nextToken();
        if(!next) return error(errors + createEOTError(":が必要です。"));
        if(next.get().kind != TokenKind::Colon) {
          errors.push_back(createASTError(":が必要です。", next.get()));
          break;
        }
        if(!nextToken()) return error(errors + createEOTError("型名が必要です。"));
        auto retType = typeGenerate();
        if(!retType) {
          errors += retType.err();
          break;
        }
        std::vector<TypeNode*> args;
        args.reserve(uargs.size());
        for(auto& arg : uargs) {
          args.push_back(arg.release());
        }
        type = std::make_unique<FnTypeNode>(retType.get(), args);
        break;
      }
      case TokenKind::Identify: {
        std::vector<std::string> name{ currentToken().value };
        while(auto token = nextToken()) {
          if(token.get().kind == TokenKind::Scope) {
            token = nextToken();
            if(!token) return error(errors + createEOTError("型名が必要です。"));
            if(token.get().kind != TokenKind::Identify) {
              errors.push_back(createASTError("型名にキーワードや記号は使用できません。", token.get()));
              continue;
            }
            name.push_back(token.get().value);
          }
          else {
            backToken(true);
            break;
          }
        }
        type = std::make_unique<UserDefineTypeNode>(name);
        auto next = nextToken();
        if(!next || next.get().kind != TokenKind::LessThan) {
          backToken(true);
          break;
        }
        if(!nextToken()) return error(errors + createEOTError("型名が必要です。"));
        std::vector<std::unique_ptr<TypeNode>> types;
        while(true) {
          auto gen = typeGenerate();
          if(!gen) {
            errors += gen.err();
            break;
          }
          types.emplace_back(std::unique_ptr<TypeNode>(gen.get()));
          next = nextToken();
          if(!next) return error(errors + createEOTError(">が必要です。"));
          if(next.get().kind == TokenKind::GreaterThan) break;
          if(next.get().kind == TokenKind::Comma) {
            nextToken();
            continue;
          }
          errors.push_back(">が必要です。");
          break;
        }
        std::vector<TypeNode*> generics;
        generics.reserve(types.size());
        for(auto& t : types) {
          generics.push_back(t.release());
        }
        type = std::make_unique<GenericsTypeNode>(dynamic_cast<UserDefineTypeNode*>(type.release())->name, generics);
        break;
      }
      default:
        return error(std::vector{ createASTError("型名にキーワードや記号は使用できません。", currentToken()) });
    }
    auto token = nextToken();
    while(token && token.get().kind == TokenKind::LBracket) {
      token = nextToken();
      if(!token) return error(createEOTError("配列の大きさが必要です。"));
      if(token.get().kind != TokenKind::Integer) return error(std::vector{ createASTError("配列の大きさが必要です。", token.get()) });
      type = std::make_unique<ArrayTypeNode>(type.release(), std::stoull(token.get().value));
      token = nextToken();
      if(!token) return error(createEOTError("]が必要です。"));
      if(token.get().kind != TokenKind::RBracket) return error(std::vector{ createASTError("]が必要です。", token.get()) });
      token = nextToken();
    }
    backToken(true);
    if(errors.empty()) {
      type->tokens = std::vector(begin, currentTokenIter() + 1);
      return ok(type.release());
    }
    return error(errors);
  }
}