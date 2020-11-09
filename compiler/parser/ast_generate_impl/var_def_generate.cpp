#include "ast.h"

#include "utils/vector_utils.h"

namespace pickc::parser
{
  Result<VariableDefineNode*, std::vector<std::string>> ASTGenerator::varDefGenerate()
  {
    assert(currentToken().kind == TokenKind::DefKeyword || currentToken().kind ==  TokenKind::MutKeyword);
    auto begin = currentTokenIter();
    auto varDef = std::make_unique<VariableDefineNode>();
    std::vector<std::string> errors;
    varDef->isMut = currentToken().kind == TokenKind::MutKeyword;
    if(!nextToken()) return error(createEOTError("変数名が必要です。"));
    if(auto name = variableGenerate()) varDef->name = name.get();
    else errors += name.err();
    if(auto token = nextToken()) {
      if(token.get().kind == TokenKind::Colon) {
        if(!nextToken()) return error(errors + createEOTError("型名が必要です。"));
        if(auto type = typeGenerate()) varDef->type = std::move(type.get());
        else errors += type.err();
        if (!(token = nextToken())) {
          varDef->tokens = std::vector(begin, currentTokenIter() + 1);
          return ok(varDef.release());
        }
      }
      if(token.get().kind == TokenKind::Asign) {
        if(!nextToken()) return error(errors + createEOTError("式が必要です。"));
        if(auto expr = exprGenerate()) varDef->init = expr.get();
        else errors += expr.err();
      }
    }
    if(errors.empty()) {
      varDef->tokens = std::vector(begin, currentTokenIter() + 1);
      return ok(varDef.release());
    }
    return error(errors);
  }
}