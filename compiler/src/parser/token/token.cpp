/**
 * @file token.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief token.hの実装
 * @version 0.1
 * @date 2021-07-22
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "token.h"

namespace pickc::parser {
  Token::Token(TokenKind kind, const std::string& value, size_t line, size_t letter) : kind(kind), value(value), line(line), letter(letter) {}
  Token::Token(const Token& token) : kind(token.kind), value(token.value), line(token.line), letter(token.letter) {}
  Token::Token(Token&& token) : kind(token.kind), value(std::move(token.value)), line(token.line), letter(token.letter) {}
  Token& Token::operator=(const Token& token)
  {
    kind = token.kind;
    value = token.value;
    line = token.line;
    letter = token.letter;
    return *this;
  }
  Token& Token::operator=(Token&& token)
  {
    kind = token.kind;
    value = std::move(token.value);
    line = token.line;
    letter = token.letter;
    return *this;
  }
}