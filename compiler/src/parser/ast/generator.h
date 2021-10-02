/**
 * @file generator.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief ASTを生成する
 * @version 0.1
 * @date 2021-07-23
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_PARSER_AST_GENERATOR_H_
#define PICKC_PARSER_AST_GENERATOR_H_

#include <optional>

#include "node.h"
#include "token/token.h"

namespace pickc::parser {
  /**
   * @brief ASTを生成する。
   * 
   */
  class ASTGenerator {
    bool done;
    RootNode rootNode;
    TokenSequence sequence;
    size_t curIndex;
  public:
    /**
     * @brief ASTGeneratorを構築する。
     * 
     * @param sequence ASTを構築するトークン列
     */
    ASTGenerator(const TokenSequence& sequence);
    /**
     * @brief ASTを生成する
     * 
     * @return std::optional<RootNode> ASTのルートノード 
     */
    std::optional<RootNode> generate();
  private:
    /**
     * @brief 現在見ているトークンを返す。
     * 
     * @return std::optional<Token> 現在見ているトークン
     */
    std::optional<Token> cur();
    /**
     * @brief 現在見ているトークンを返し、ASTGenerator::curを一つ進める。
     * 
     * @return std::optional<Token> 現在見ているトークン
     */
    std::optional<Token> next();
    /**
     * @brief 現在見ているトークンを第一引数に代入し、ASTGenerator::curを一つ進める。
     * もし、シーケンスの終わりに達していたら、outputBufferにエラーメッセージを追加する。
     * 
     * @param token 現在見ているトークンを受け取る
     * @param message シーケンスの終わりに達していた場合のエラーメッセージ
     * @return true シーケンスが終わりに達していない場合
     * @return false シーケンスが終わりに達していた場合
     */
    bool next(Token& token, const std::string& message);
    /**
     * @brief 現在見ているトークンを第一引数に代入し、ASTGenerator::curを一つ進める。
     * もし、シーケンスの終わりに達していたりトークンの種別が期待するものでない場合、outputBufferにエラーメッセージを追加する。
     * 
     * @param token 現在見ているトークンを受け取る
     * @param expect 
     * @param message シーケンスの終わりに達していた場合のエラーメッセージ
     * @return true シーケンスが終わりに達していない場合
     * @return false シーケンスが終わりに達していた場合やトークンの種別が期待するものでない場合
     */
    bool next(Token& token, TokenKind expect, const std::string& message);
    /**
     * @brief curを一つ戻す。
     * 
     */
    void back();

    /*
     * ここ以下に定義する関数群はast/analyze以下で実装する。
     */

    /**
     * @brief 文を解析する。
     * 
     * @param parentNode このノードの親
     * @return std::optional<StatementNode> 解析結果 
     */
    std::optional<StatementNode> analyzeStatement(ASTNode parentNode);
    /**
     * @brief 関数を解析する。
     * sequence[cur].token.kindがTokenKind::FnKeywordである必要がある。
     * 
     * @param parentNode このノードの親
     * @return std::optional<FunctionNode> 解析結果
     */
    std::optional<FunctionNode> analyzeFunction(ASTNode parentNode);
    /**
     * @brief 変数宣言を解析する。
     * sequence[cur].token.kindがTokenKind::DefKeywordである必要がある。
     * 
     * @param parentNode このノードの親
     * @return std::optional<VariableDeclarationNode> 解析結果
     */
    std::optional<VariableDeclarationNode> analyzeVarDecl(ASTNode parentNode);
    /**
     * @brief 式を解析する。
     * 
     * @param parentNode このノードの親
     * @return std::optional<ExpressionNode> 解析結果 
     */
    std::optional<ExpressionNode> analyzeExpression(ASTNode parentNode);
    std::optional<ExpressionNode> analyzePrimaryExpression(ASTNode parentNode);
    /**
     * @brief ブロックを解析する。
     * 
     * @param parentNode このノードの親
     * @return std::optional<BlockNode> 解析結果
     */
    std::optional<BlockNode> analyzeBlock(ASTNode parentNode);
    /**
     * @brief 制御構文を解析する。
     * 
     * @param parentNode このノードの親
     * @return std::optional<ControlNode> 解析結果 
     */
    std::optional<ControlNode> analyzeControl(ASTNode parentNode);
    /**
     * @brief 即値を解析する。
     * 
     * @param parentNode このノードの親
     * @return std::optional<ImmediateNode> 解析結果
     */
    std::optional<ImmediateNode> analyzeImmediate(ASTNode parentNode);
    /**
     * @brief 変数を解析する。
     * 
     * @param parentNode このノードの親
     * @return std::optional<VariableNode> 解析結果
     */
    std::optional<VariableNode> analyzeVariable(ASTNode parentNode);
    std::optional<ExpressionNode> analyzeBackUnaryExpression(ASTNode parentNode);
    std::optional<ExpressionNode> analyzeFrontUnaryExpression(ASTNode parentNode);
    std::optional<ExpressionNode> analyzeFactor(ASTNode parentNode);
    std::optional<ExpressionNode> analyzeTerm(ASTNode parentNode);
    std::optional<ExpressionNode> analyzeCompare(ASTNode parentNode);
    std::optional<ExpressionNode> analyzeAsign(ASTNode parentNode);
  };
}

#endif // PICKC_PARSER_AST_GENERATOR_H_