#include "ast.h"

namespace pickc::parser
{
  Result<VariableNode*, std::vector<std::string>> ASTGenerator::variableGenerate()
  {
    auto begin = currentTokenIter();
    auto token = currentToken();
    if(token.kind == TokenKind::Identify) {
      auto name = token.value;
      std::vector<TypeNode*> generics;
      const auto backup = _tokenIndex;
      auto generic = nextToken();
      if(generic && generic.get().kind == TokenKind::LessThan) {
        nextToken();
        std::vector<std::unique_ptr<TypeNode>> ugenerics;
        while(true) {
          auto type = typeGenerate();
          if(!type) {
            _tokenIndex = backup;
            break;
          }
          ugenerics.push_back(std::unique_ptr<TypeNode>(type.get()));
          auto next = nextToken();
          if(!next || (next.get().kind != TokenKind::Comma && next.get().kind != TokenKind::GreaterThan)) {
            _tokenIndex = backup;
            break;
          }
          if(next.get().kind == TokenKind::Comma) {
            nextToken();
            continue;
          }
          else if(next.get().kind == TokenKind::GreaterThan) {
            generics.reserve(ugenerics.size());
            for(auto& gen : ugenerics) {
              generics.push_back(gen.release());
            }
            break;
          }
          else {
            assert(false);
          }
        }
      }
      else {
        _tokenIndex = backup;
      }
      auto var = new VariableNode(name, generics);
      var->tokens = std::vector(begin, currentTokenIter() + 1);
      return ok(var);
    }
    return error(std::vector{ createASTError("キーワードや記号は使用できません。", token) });
  }
}