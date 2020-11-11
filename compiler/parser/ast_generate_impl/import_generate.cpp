#include "ast.h"

#include "utils/vector_utils.h"

namespace pickc::parser
{
  Result<ImportNode*, std::vector<std::string>> ASTGenerator::importGenerate()
  {
    assert(currentToken().kind == TokenKind::ImportKeyword);
    std::vector<std::string> errors;
    auto begin = currentTokenIter();
    if(!nextToken()) return error(createEOTError("モジュール名が必要です。"));
    if(currentToken().kind != TokenKind::Identify) errors.push_back(createASTError("モジュール名に記号やキーワードは使用できません。", currentToken()));
    auto cur = std::make_unique<VariableNode>(currentToken().value, std::vector<TypeNode*>{});
    while(true) {
      if(!nextToken() || currentToken().kind != TokenKind::Scope) {
        backToken(true);
        auto import = new ImportNode(cur.release());
        import->tokens = std::vector(begin, currentTokenIter() + 1);
        return ok(import);
      }
      if(!nextToken()) return error(errors + createEOTError("モジュール名が必要です。"));
      if(currentToken().kind != TokenKind::Identify) errors.push_back(createASTError("モジュール名に記号やキーワードは使用できません。", currentToken()));
      cur = std::unique_ptr<VariableNode>(new ScopedVariableNode(currentToken().value, {}, cur.release()));
    }
  }
}