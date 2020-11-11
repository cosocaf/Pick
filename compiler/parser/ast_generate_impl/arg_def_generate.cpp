#include "ast.h"

#include "utils/vector_utils.h"

namespace pickc::parser
{
  Result<std::vector<ArgumentDefineNode*>, std::vector<std::string>> ASTGenerator::argDefGenerate()
  {
    assert(currentToken().kind == TokenKind::LParen);

    auto token = nextToken();
    if(!token) return error(createEOTError(")が必要です。"));
    if(token.get().kind == TokenKind::RParen) return ok(std::vector<ArgumentDefineNode*>());

    std::vector<std::unique_ptr<ArgumentDefineNode>> args;
    std::vector<std::string> errors;
    const auto argGen = [&]() -> Option<std::vector<std::string>> {
      auto begin = currentTokenIter();
      auto argDef = std::make_unique<ArgumentDefineNode>();
      if(token.get().kind == TokenKind::DefKeyword) {
        argDef->isMut = false;
        token = nextToken();
        if(!token) return some(createEOTError("引数名が必要です。"));
      }
      else if (token.get().kind == TokenKind::MutKeyword) {
        argDef->isMut = true;
        token = nextToken();
        if(!token) return some(createEOTError("引数名が必要です。"));
      }
      if(auto name = variableGenerate()) argDef->name = name.get();
      else errors += name.err();

      token = nextToken();
      if(!token) return some(createEOTError(")が必要です。"));
      if(token.get().kind == TokenKind::Colon) {
        token = nextToken();
        if(!token) return some(createEOTError("引数の型名が必要です。"));
        if(auto type = typeGenerate()) argDef->type = type.get();
        else errors += type.err();
        token = nextToken();
        if(!token) return some(createEOTError(")が必要です。"));
      }

      if(token.get().kind == TokenKind::Asign) {
        token = nextToken();
        if(!token) return some(createEOTError("式が必要です。"));
        if(auto expr = exprGenerate()) argDef->init = expr.get();
        else errors += expr.err();
      }

      argDef->tokens = std::vector(begin, currentTokenIter() + 1);
      args.emplace_back(std::move(argDef));
      return none;
    };

    if(auto res = argGen()) return error(errors + res.get());
    while(true) {
      if(currentToken().kind == TokenKind::Comma) {
        token = nextToken();
        if(!token) return error(errors + createEOTError("引数が必要です。"));
        if(auto res = argGen()) return error(errors + res.get());
      }
      else if(currentToken().kind == TokenKind::RParen) {
        break;
      }
      else {
        errors.push_back(createASTError(")が必要です。", token.get()));
        break;
      }
    }

    if(errors.empty()) {
      std::vector<ArgumentDefineNode*> result;
      for(auto& arg : args) {
        result.push_back(arg.release());
      }
      return ok(std::move(result));
    }
    return error(errors);
  }
}