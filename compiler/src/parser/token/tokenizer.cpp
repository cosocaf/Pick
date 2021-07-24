/**
 * @file tokenizer.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief tokenizer.hの実装
 * @version 0.1
 * @date 2021-07-22
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "tokenizer.h"

#include <fstream>
#include <cassert>
#include <regex>

#include "output_buffer.h"

namespace pickc::parser {
  Tokenizer::Tokenizer(const std::string& filename) : done(false), error(false), filename(filename) {}
  std::optional<TokenSequence> Tokenizer::tokenize() {
    assert(done == false);
    done = true;

    sequence.filename = filename;

    std::ifstream stream(filename);
    if(!stream) {
      outputBuffer.emplace<OutputMessage>(OutputMessageKind::Error, "ファイルを開けません: " + filename);
      return std::nullopt;
    }

    std::string lineStr;
    while(std::getline(stream, lineStr)) {
      sequence.rawFileData.emplace_back(std::move(lineStr));
    }

    line = 0;
    for(const auto& cur : sequence.rawFileData) {
      letter = 0;
      ++line;
      for(auto c : cur) {
        ++letter;
        switch(c) {
          case ' ': clearBuffer(); break;
          case '(': clearBuffer(); pushToken(TokenKind::LParen, "("); break;
          case ')': clearBuffer(); pushToken(TokenKind::RParen, ")"); break;
          case '{': clearBuffer(); pushToken(TokenKind::LBrace, "{"); break;
          case '}': clearBuffer(); pushToken(TokenKind::RBrase, "}"); break;
          case '[': clearBuffer(); pushToken(TokenKind::LBracket, "["); break;
          case ']': clearBuffer(); pushToken(TokenKind::RBracket, "]"); break;
          case ';': clearBuffer(); pushToken(TokenKind::Semicolon, ";"); break;
          default: pushBuffer(c);
        }
      }
      clearBuffer();
    }

    if(error) return std::nullopt;
    return sequence;
  }
  void Tokenizer::clearBuffer() {
    if(buffer.empty()) {
      goto end;
    }

    TokenKind kind;
    if(buffer == "fn") kind = TokenKind::FnKeyword;
    else if(buffer == "return") kind = TokenKind::ReturnKeyword;
    else if(std::regex_match(buffer, std::regex(R"(^[1-9][0-9]*$)"))) kind = TokenKind::Integer;
    else if(std::regex_match(buffer, std::regex(R"(^[_a-zA-Z][_a-zA-Z0-9]*$)"))) kind = TokenKind::Identify;
    else {
      kind = TokenKind::Unknown;
      outputBuffer.emplace<CompileErrorMessage>(filename, beginTokenLine, beginTokenLetter, buffer.size(), "予期しないトークン: " + buffer);
      error = true;
    }

    if(kind != TokenKind::Unknown) {
      pushToken(kind, buffer);
    }
    buffer.clear();

    end:
    beginTokenLine = line;
    beginTokenLetter = letter;
  }
  void Tokenizer::pushBuffer(char c) {
    if(buffer.empty()) {
      beginTokenLine = line;
      beginTokenLetter = letter;
    }
    buffer.push_back(c);
  }
  void Tokenizer::pushToken(TokenKind kind, const std::string& value) {
    sequence.tokens.emplace_back(kind, value, beginTokenLine, beginTokenLetter);
  }
}