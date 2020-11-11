#include "ast.h"

#include "utils/vector_utils.h"

namespace pickc::parser
{
  Result<FunctionDefineNode*, std::vector<std::string>> ASTGenerator::fnDefGenerate()
  {
    assert(currentToken().kind == TokenKind::FnKeyword);
    auto begin = currentTokenIter();
    auto fnDef = std::make_unique<FunctionDefineNode>();
    std::vector<std::string> errors;
    if(!nextToken()) return error(createEOTError("関数名または(が必要です。"));
    if(currentToken().kind == TokenKind::Identify) {
      if(auto name = variableGenerate()) fnDef->name = name.get();
      else errors += name.err();
      if(!nextToken()) return error(errors + createEOTError("(が必要です。"));
    }
    if(auto args = argDefGenerate()) fnDef->args = std::move(args.get());
    else errors += args.err();
    auto token = nextToken();
    if(!token) return error(errors + createEOTError("関数の本文が必要です。"));
    if(token.get().kind == TokenKind::Colon) {
      if(!nextToken()) return error(createEOTError("戻り値の型名が必要です。"));
      if(auto retType = typeGenerate()) fnDef->retType = retType.get();
      else errors += retType.err();
      if(!nextToken()) return error(errors + createEOTError("関数の本文が必要です。"));
    }
    if(auto body = exprGenerate()) fnDef->body = body.get();
    else errors += body.err();
    if(errors.empty()) {
      fnDef->tokens = std::vector(begin, currentTokenIter() + 1);
      return ok(fnDef.release());
    }
    return error(errors);
  }
}