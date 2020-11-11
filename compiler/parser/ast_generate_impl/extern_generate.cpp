#include "ast.h"

#include "utils/vector_utils.h"

namespace pickc::parser
{
  Result<ExternNode*, std::vector<std::string>> ASTGenerator::externGenerate()
  {
    assert(currentToken().kind == TokenKind::ExternKeyword);
    auto begin = currentTokenIter();
    auto ext = std::make_unique<ExternNode>();
    std::vector<std::string> errors;
    if(!nextToken()) return error(createEOTError("関数名が必要です。"));
    if(currentToken().kind != TokenKind::Identify) errors.push_back(createASTError("関数名が必要です。", currentToken()));
    else if(auto name = variableGenerate()) ext->name = name.get();
    else errors += name.err();
    if(!nextToken()) return error(errors + createEOTError("(が必要です。"));
    if(auto args = argDefGenerate()) ext->args = std::move(args.get());
    else errors += args.err();
    auto token = nextToken();
    if(!token) return error(errors + createEOTError("戻り値の型名が必要です。"));
    if(token.get().kind != TokenKind::Colon) errors.push_back(createASTError("戻り値の型名が必要です。", token.get()));
    else {
      if(!nextToken()) return error(createEOTError("戻り値の型名が必要です。"));
      if(auto retType = typeGenerate()) ext->retType = retType.get();
      else errors += retType.err();
    }
    ext->tokens = std::vector(begin, currentTokenIter() + 1);
    if(errors.empty()) return ok(ext.release());
    return error(errors);
  }
}