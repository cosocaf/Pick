#ifndef PICKC_PARSER_AST_H_
#define PICKC_PARSER_AST_H_

#include "token.h"
#include "ast_node.h"

namespace pickc::parser
{ 
  class ASTGenerator
  {
    TokenSequence sequence;
    int _tokenIndex;
  private:
    Option<Token> nextToken(bool index = true);
    Option<Token> backToken(bool index = false);
    Token currentToken();
    bool hasNext();
    std::vector<pickc::parser::Token>::iterator currentTokenIter();
    std::string createASTError(const std::string& message, const Token& token);
    std::vector<std::string> createEOTError(const std::string& addMessage);
    Result<FunctionDefineNode*, std::vector<std::string>> fnDefGenerate();
    Result<VariableDefineNode*, std::vector<std::string>> varDefGenerate();
    Result<ClassDefineNode*, std::vector<std::string>> clsDefGenerate();
    Result<AliasDefineNode*, std::vector<std::string>> alsDefGenerate();
    Result<ImportNode*, std::vector<std::string>> importGenerate();
    Result<ExternNode*, std::vector<std::string>> externGenerate();
    Result<VariableNode*, std::vector<std::string>> variableGenerate();
    Result<TypeNode*, std::vector<std::string>> typeGenerate();
    Result<std::vector<ArgumentDefineNode*>, std::vector<std::string>> argDefGenerate();
    Result<ExpressionNode*, std::vector<std::string>> exprGenerate();
    Result<ExpressionNode*, std::vector<std::string>> asignGenerate();
    Result<ExpressionNode*, std::vector<std::string>> compGenerate();
    Result<ExpressionNode*, std::vector<std::string>> termGenerate();
    Result<ExpressionNode*, std::vector<std::string>> factorGenerate();
    Result<ExpressionNode*, std::vector<std::string>> frontUnaryGenerate();
    Result<ExpressionNode*, std::vector<std::string>> backUnaryGenerate();
    Result<ExpressionNode*, std::vector<std::string>> primaryGenerate();
  public:
    ASTGenerator(const TokenSequence& sequence);
    Result<RootNode, std::vector<std::string>> generate();
  };
}

#endif // PICKC_PARSER_AST_H_