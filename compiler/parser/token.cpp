#include "token.h"

#include <cassert>
#include <regex>

namespace pickc::parser
{
  Tokenizer::Tokenizer(const std::string& path) : sequence{path, {}}, errors(), stream(), done(false) {}
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
  std::string TokenSequence::toOutputString(size_t line) const
  {
    std::string res;
    res += '|';
    res += std::to_string(line + 1);
    res += "| ";
    res += rawFileData[line];
    res += '\n';
    return res;
  }
  Result<TokenSequence, std::vector<std::string>> Tokenizer::tokenize()
  {
    assert(!done);
    stream.open(sequence.file);
    if(!stream) {
      return error(std::vector{"エラー: ファイル " + sequence.file + " が開けませんでした。"});
    }

    std::string str;
    std::string buf;
    size_t line, letter;
    const auto clearBuf = [&]() {
      if(buf.size() == 0) return;
      TokenKind kind;
      if (buf == "fn") kind = TokenKind::FnKeyword;
      else if (buf == "def") kind = TokenKind::DefKeyword;
      else if (buf == "mut") kind = TokenKind::MutKeyword;
      else if (buf == "extern") kind = TokenKind::ExternKeyword;
      else if (buf == "import") kind = TokenKind::ImportKeyword;
      else if (buf == "return") kind = TokenKind::ReturnKeyword;
      else if (buf == "if") kind = TokenKind::IfKeyword;
      else if (buf == "else") kind = TokenKind::ElseKeyword;
      else if (buf == "for") kind = TokenKind::ForKeyword;
      else if (buf == "while") kind = TokenKind::WhileKeyword;
      else if (buf == "loop") kind = TokenKind::LoopKeyword;
      else if (buf == "pub") kind = TokenKind::PubKeyword;
      else if (buf == "pri") kind = TokenKind::PriKeyword;
      else if (buf == "class") kind = TokenKind::ClassKeyword;
      else if (buf == "construct") kind = TokenKind::ConstructKeyword;
      else if (buf == "destruct") kind = TokenKind::DestructKeyword;
      else if (buf == "type") kind = TokenKind::TypeKeyword;
      else if (buf == "i8") kind = TokenKind::I8Keyword;
      else if (buf == "i16") kind = TokenKind::I16Keyword;
      else if (buf == "i32") kind = TokenKind::I32Keyword;
      else if (buf == "i64") kind = TokenKind::I64Keyword;
      else if (buf == "u8") kind = TokenKind::U8Keyword;
      else if (buf == "u16") kind = TokenKind::U16Keyword;
      else if (buf == "u32") kind = TokenKind::U32Keyword;
      else if (buf == "u64") kind = TokenKind::U64Keyword;
      else if (buf == "f32") kind = TokenKind::F32Keyword;
      else if (buf == "f64") kind = TokenKind::F64Keyword;
      else if (buf == "void") kind = TokenKind::VoidKeyword;
      else if (buf == "bool") kind = TokenKind::BoolKeyword;
      else if (buf == "char") kind = TokenKind::CharKeyword;
      else if (buf == "ptr") kind = TokenKind::PtrKeyword;
      else if (buf == "true" || buf == "false") kind = TokenKind::Bool;
      else if (buf == "null") kind = TokenKind::Null;
      else if (buf == "this") kind = TokenKind::This;
      else {
        std::smatch match;
        if (std::regex_match(buf, match, std::regex(R"(^\d+$)"))) kind = TokenKind::Integer;
        else if (std::regex_match(buf, match, std::regex(R"(^\d+i8$)"))) kind = TokenKind::I8;
        else if (std::regex_match(buf, match, std::regex(R"(^\d+i16$)"))) kind = TokenKind::I16;
        else if (std::regex_match(buf, match, std::regex(R"(^\d+i32$)"))) kind = TokenKind::I32;
        else if (std::regex_match(buf, match, std::regex(R"(^\d+i64$)"))) kind = TokenKind::I64;
        else if (std::regex_match(buf, match, std::regex(R"(^\d+u8$)"))) kind = TokenKind::U8;
        else if (std::regex_match(buf, match, std::regex(R"(^\d+u16$)"))) kind = TokenKind::U16;
        else if (std::regex_match(buf, match, std::regex(R"(^\d+u32$)"))) kind = TokenKind::I32;
        else if (std::regex_match(buf, match, std::regex(R"(^\d+u64$)"))) kind = TokenKind::U64;
        else if (std::regex_match(buf, match, std::regex(R"(^\d+?\.?\d+(e[\-+]?\d+)?$)"))) kind = TokenKind::Float;
        else if (std::regex_match(buf, match, std::regex(R"(^\d+?\.?\d+(e[\-+]?\d+)?f32$)"))) kind = TokenKind::F32;
        else if (std::regex_match(buf, match, std::regex(R"(^\d+?\.?\d+(e[\-+]?\d+)?f64$)"))) kind = TokenKind::F64;
        else if (std::regex_match(buf, match, std::regex(R"(^[^\s!-@\[-\^`\{-~][^\s!-/:-@\[-\^`\{-~]*$)"))) kind = TokenKind::Identify;
        else {
          errors.push_back("エラー: " + buf + " は不正な文字です。\n    at " + sequence.file + " " + std::to_string(line + 1) + "行目, " + std::to_string(letter - buf.size() + 1) + "文字目");
          return;
        }
      }
      sequence.tokens.push_back(Token(kind, buf, line + 1, letter - buf.size() + 1));
      buf.clear();
    };
    for(line = 0; std::getline(stream, str); ++line) {
      sequence.rawFileData.push_back(str);
      const auto len = str.size();
      for(letter = 0; letter < len; ++letter) {
        switch(str[letter]) {
          // 空白文字はスキップ
          case ' ':
          case '\t':
            clearBuf();
            break;
          case '(':
            clearBuf();
            sequence.tokens.push_back(Token(TokenKind::LParen, "(", line + 1, letter + 1));
            break;
          case ')':
            clearBuf();
            sequence.tokens.push_back(Token(TokenKind::RParen, ")", line + 1, letter + 1));
            break;
          case '{':
            clearBuf();
            sequence.tokens.push_back(Token(TokenKind::LBrace, "{", line + 1, letter + 1));
            break;
          case '}':
            clearBuf();
            sequence.tokens.push_back(Token(TokenKind::RBrase, "}", line + 1, letter + 1));
            break;
          case '[':
            clearBuf();
            sequence.tokens.push_back(Token(TokenKind::LBracket, "[", line + 1, letter + 1));
            break;
          case ']':
            clearBuf();
            sequence.tokens.push_back(Token(TokenKind::RBracket, "]", line + 1, letter + 1));
            break;
          case ';':
            clearBuf();
            sequence.tokens.push_back(Token(TokenKind::Semicolon, ";", line + 1, letter + 1));
            break;
          case ',':
            clearBuf();
            sequence.tokens.push_back(Token(TokenKind::Comma, ",", line + 1, letter + 1));
            break;
          case '@':
            clearBuf();
            sequence.tokens.push_back(Token(TokenKind::Copy, "@", line + 1, letter + 1));
            break;
          case '+':
            clearBuf();
            if(++letter >= len || (str[letter] != '+' && str[letter] != '=')) {
              --letter;
              sequence.tokens.push_back(Token(TokenKind::Plus, "+", line + 1, letter + 1));
            }
            else if(str[letter] == '+') {
              sequence.tokens.push_back(Token(TokenKind::Inc, "++", line + 1, letter));
            }
            else {
              sequence.tokens.push_back(Token(TokenKind::AddAsign, "+=", line + 1, letter));
            }
            break;
          case '-':
            clearBuf();
            if(++letter >= len || (str[letter] != '-' && str[letter] != '=')) {
              --letter;
              sequence.tokens.push_back(Token(TokenKind::Minus, "-", line + 1, letter + 1));
            }
            else if(str[letter] == '-') {
              sequence.tokens.push_back(Token(TokenKind::Dec, "--", line + 1, letter));
            }
            else {
              sequence.tokens.push_back(Token(TokenKind::SubAsign, "-=", line + 1, letter));
            }
            break;
          case '*':
            clearBuf();
            if(++letter >= len || str[letter] != '=') {
              --letter;
              sequence.tokens.push_back(Token(TokenKind::Asterisk, "*", line + 1, letter + 1));
            }
            else {
              sequence.tokens.push_back(Token(TokenKind::MulAsign, "*=", line + 1, letter));
            }
            break;
          case '/':
            clearBuf();
            if(++letter >= len || (str[letter] != '=' && str[letter] != '/')) {
              --letter;
              sequence.tokens.push_back(Token(TokenKind::Slash, "/", line + 1, letter + 1));
            }
            else if (str[letter] == '=') {
              sequence.tokens.push_back(Token(TokenKind::DivAsign, "/=", line + 1, letter));
            }
            else {
              sequence.tokens.push_back(Token(TokenKind::LineComment, str.substr(letter - 1), line + 1, letter));
              letter = len;
            }
            break;
          case '%':
            clearBuf();
            if(++letter >= len || str[letter] != '=') {
              --letter;
              sequence.tokens.push_back(Token(TokenKind::Percent, "%", line + 1, letter + 1));
            }
            else {
              sequence.tokens.push_back(Token(TokenKind::ModAsign, "%=", line + 1, letter));
            }
            break;
          case '&':
            clearBuf();
            if(++letter >= len || (str[letter] != '&' && str[letter] != '=')) {
              --letter;
              sequence.tokens.push_back(Token(TokenKind::BitAnd, "&", line + 1, letter + 1));
            }
            else if(str[letter] == '&') {
              sequence.tokens.push_back(Token(TokenKind::LogicalAnd, "&&", line + 1, letter));
            }
            else {
              sequence.tokens.push_back(Token(TokenKind::BitAndAsign, "&=", line + 1, letter));
            }
            break;
          case '|':
            clearBuf();
            if(++letter >= len || (str[letter] != '|' && str[letter] != '=')) {
              --letter;
              sequence.tokens.push_back(Token(TokenKind::BitOr, "|", line + 1, letter + 1));
            }
            else if(str[letter] == '|') {
              sequence.tokens.push_back(Token(TokenKind::LogicalOr, "||", line + 1, letter));
            }
            else {
              sequence.tokens.push_back(Token(TokenKind::BitOrAsign, "|=", line + 1, letter));
            }
            break;
          case '^':
            clearBuf();
            if(++letter >= len || str[letter] != '=') {
              --letter;
              sequence.tokens.push_back(Token(TokenKind::BitXor, "^", line + 1, letter + 1));
            }
            else {
              sequence.tokens.push_back(Token(TokenKind::BitXorAsign, "^=", line + 1, letter));
            }
            break;
          case '~':
            clearBuf();
            sequence.tokens.push_back(Token(TokenKind::BitNot, "~", line + 1, letter + 1));
            break;
          case '=':
            clearBuf();
            if(++letter >= len || str[letter] != '=') {
              --letter;
              sequence.tokens.push_back(Token(TokenKind::Asign, "=", line + 1, letter + 1));
            }
            else {
              sequence.tokens.push_back(Token(TokenKind::Equal, "==", line + 1, letter));
            }
            break;
          case '<':
            clearBuf();
            if(++letter >= len || (str[letter] != '=' && str[letter] != '<')) {
              --letter;
              sequence.tokens.push_back(Token(TokenKind::LessThan, "<", line + 1, letter + 1));
            }
            else if(str[letter] == '=') {
              sequence.tokens.push_back(Token(TokenKind::LessEqual, "<=", line + 1, letter));
            }
            else if(++letter >= len || str[letter] != '=') {
              --letter;
              sequence.tokens.push_back(Token(TokenKind::LShift, "<<", line + 1, letter));
            }
            else {
              sequence.tokens.push_back(Token(TokenKind::LShiftAsign, "<<=", line + 1, letter - 1));
            }
            break;
          case '>':
            clearBuf();
            if(++letter >= len || (str[letter] != '=' && str[letter] != '>')) {
              --letter;
              sequence.tokens.push_back(Token(TokenKind::GreaterThan, ">", line + 1, letter + 1));
            }
            else if(str[letter] == '=') {
              sequence.tokens.push_back(Token(TokenKind::GreaterEqual, ">=", line + 1, letter));
            }
            else if(++letter >= len || str[letter] != '=') {
              --letter;
              sequence.tokens.push_back(Token(TokenKind::RShift, ">>", line + 1, letter));
            }
            else {
              sequence.tokens.push_back(Token(TokenKind::RShiftAsign, ">>=", line + 1, letter - 1));
            }
            break;
          case '!':
            clearBuf();
            if(++letter >= len || str[letter] != '=') {
              --letter;
              sequence.tokens.push_back(Token(TokenKind::LogicalNot, "!", line + 1, letter + 1));
            }
            else {
              sequence.tokens.push_back(Token(TokenKind::NotEqual, "!=", line + 1, letter));
            }
            break;
          case '.':
            clearBuf();
            if(++letter >= len || str[letter] != '.') {
              --letter;
              sequence.tokens.push_back(Token(TokenKind::Dot, ".", line + 1, letter + 1));
            }
            else {
              sequence.tokens.push_back(Token(TokenKind::Range, "..", line + 1, letter));
            }
            break;
          case ':':
            clearBuf();
            if(++letter >= len || str[letter] != ':') {
              --letter;
              sequence.tokens.push_back(Token(TokenKind::Colon, ":", line + 1, letter + 1));
            }
            else {
              sequence.tokens.push_back(Token(TokenKind::Scope, "::", line + 1, letter));
            }
            break;
          case '\'': {
            // TODO: マルチバイト文字の対応
            clearBuf();
            auto let = letter;
            if(++letter >= len) {
              errors.push_back("エラー: 'が必要です。\n    at " + sequence.file + " " + std::to_string(line + 1) + "行目, " + std::to_string(let + 1) + "文字目");
              break;
            }
            char c = str[letter];
            if(c == '\'') {
              errors.push_back("エラー: 文字が必要です。\n    at " + sequence.file + " " + std::to_string(line + 1) + "行目, " + std::to_string(let + 1) + "文字目");
              break;
            }
            if(c == '\\') {
              if(++letter >= len) {
                errors.push_back("エラー: 文字が必要です。\n    at " + sequence.file + " " + std::to_string(line + 1) + "行目, " + std::to_string(let + 1) + "文字目");
                break;
              }
              switch(str[letter]) {
                case 'a': c = '\a'; break;
                case 'b': c = '\b'; break;
                case 'n': c = '\n'; break;
                case 'r': c = '\r'; break;
                case 'f': c = '\f'; break;
                case 't': c = '\t'; break;
                case 'v': c = '\v'; break;
                case '\\': c = '\\'; break;
                case '0': c = '\0'; break;
                case '"': c = '"'; break;
                case '\'': c = '\''; break;
                default: errors.push_back(std::string("エラー: ") + str[letter] + " は不正なエスケープです。\n    at " + sequence.file + " " + std::to_string(line + 1) + "行目, " + std::to_string(let + 1) + "文字目");
              }
            }
            if(++letter >= len) {
              errors.push_back("エラー: 'が必要です。\n    at " + sequence.file + " " + std::to_string(line + 1) + "行目, " + std::to_string(let + 1) + "文字目");
              break;
            }
            sequence.tokens.push_back(Token(TokenKind::Char, std::string() + c, line + 1, let + 1));
            break;
          }
          case '"': {
            // TODO: マルチバイト文字の対応
            clearBuf();
            std::string string;
            bool correct = false;
            bool esc = false;
            auto let = letter;
            for(++letter; letter < len; ++letter) {
              if(str[letter] == '\\') {
                esc = true;
              }
              else if(esc) {
                esc = false;
                switch(str[letter]) {
                  case 'a': string += '\a'; break;
                  case 'b': string += '\b'; break;
                  case 'n': string += '\n'; break;
                  case 'r': string += '\r'; break;
                  case 'f': string += '\f'; break;
                  case 't': string += '\t'; break;
                  case 'v': string += '\v'; break;
                  case '\\': string += '\\'; break;
                  case '0': string += '\0'; break;
                  case '"': string += '"'; break;
                  case '\'': string += '\''; break;
                  default: errors.push_back(std::string("エラー: ") + str[letter] + " は不正なエスケープです。\n    at " + sequence.file + " " + std::to_string(line + 1) + "行目, " + std::to_string(let + 1) + "文字目");
                }
              }
              else if(str[letter] == '"') {
                sequence.tokens.push_back(Token(TokenKind::String, string, line + 1, let + 1));
                correct = true;
                break;
              }
              else {
                string += str[letter];
              }
            }
            if(esc || !correct) {
              errors.push_back("エラー: \"が必要です。\n    at " + sequence.file + " " + std::to_string(line + 1) + "行目, " + std::to_string(let + 1) + "文字目");
            }
            break;
          }
          default:
            buf += str[letter];
        }
      }
      clearBuf();
    }

    done = true;
    if(errors.empty()) return ok(std::move(sequence));
    return error(errors);
  }
}