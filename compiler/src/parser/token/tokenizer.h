/**
 * @file tokenizer.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 字句解析器
 * @version 0.1
 * @date 2021-07-22
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PARSER_TOKEN_TOKENIZER_H_
#define PARSER_TOKEN_TOKENIZER_H_

#include <string>
#include <optional>

#include "token.h"

namespace pickc::parser {
  /**
   * @brief 字句解析器
   * 
   */
  class Tokenizer {
    bool done;
    bool error;
    std::string filename;
    TokenSequence sequence;
    std::string buffer;
    size_t line, letter;
    size_t beginTokenLine, beginTokenLetter;
  public:
    /**
     * @brief Tokenizerを構築する。
     * 
     * @param filename 解析するファイル名
     */
    Tokenizer(const std::string& filename);
    /**
     * @brief 字句解析をする。
     * 複数回呼び出された場合、アサーションエラーになる。
     * @return std::optional<TokenSequence> 字句解析の結果。失敗した場合std::nullopt
     */
    std::optional<TokenSequence> tokenize();
  private:
    /**
     * @brief Tokenizer::bufferの中身を解析し、Tokenizer::sequenceに挿入する。
     * その後、Tokenizer::bufferを空にする。
     */
    void clearBuffer();
    /**
     * @brief Tokenizer::bufferに文字を挿入する。
     * 
     * @param c 挿入する文字
     */
    void pushBuffer(char c);
    /**
     * @brief Tokenizer::sequenceにトークンを挿入する。
     * 
     * @param kind 挿入するトークンの種別
     * @param value 挿入するトークンの値
     */
    void pushToken(TokenKind kind, const std::string& value);
  };
}

#endif // PARSER_TOKEN_TOKENIZER_H_