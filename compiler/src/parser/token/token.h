/**
 * @file token.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 字句表現
 * @version 0.1
 * @date 2021-07-22
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */

#ifndef PICKC_PARSER_TOKEN_TOKEN_H_
#define PICKC_PARSER_TOKEN_TOKEN_H_

#include <string>
#include <vector>

namespace pickc::parser {
  /**
   * @brief トークンの種別の列挙。
   * 
   */
  enum struct TokenKind {
    FnKeyword,        // fn
    ReturnKeyword,    // return
    LParen,           // (
    RParen,           // )
    LBrace,           // {
    RBrase,           // }
    LBracket,         // [
    RBracket,         // ]
    Semicolon,        // ;
    Integer,          // [0-9]+
    Identify,         // xxx
    Unknown,          // 
  };
  /**
   * @brief 単一のトークンを表す。
   * 
   */
  struct Token {
    /**
     * @brief トークンの種別。
     * 
     */
    TokenKind kind;
    /**
     * @brief トークンの内容
     * 
     */
    std::string value;
    /**
     * @brief このトークンが存在する行。
     * 
     */
    size_t line;
    /**
     * @brief このトークンが存在する列。
     * 
     */
    size_t letter;
    Token(TokenKind kind, const std::string& value, size_t line, size_t letter);
    Token(const Token& token);
    Token(Token&& token);
    Token& operator=(const Token& token);
    Token& operator=(Token&& token);
  };
  /**
   * @brief 単一のファイルのトークン列を表す。
   * Tokenizerの処理結果に使用される。
   */
  struct TokenSequence
  {
    /**
     * @brief このファイルの名前。
     * 
     */
    std::string filename;
    /**
     * @brief ファイルの内容を行ごとにまとめたもの。
     * 
     */
    std::vector<std::string> rawFileData;
    /**
     * @brief このファイルのトークン列。
     * 
     */
    std::vector<Token> tokens;
  };
}

#endif // PICKC_PARSER_TOKEN_TOKEN_H_