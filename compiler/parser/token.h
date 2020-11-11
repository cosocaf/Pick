#ifndef PICKC_PARSER_TOKEN_H_
#define PICKC_PARSER_TOKEN_H_

#include <vector>
#include <string>
#include <fstream>

#include "utils/result.h"
#include "utils/option.h"

namespace pickc::parser
{
  enum struct TokenKind
  {
    DefKeyword,       // def
    MutKeyword,       // mut
    FnKeyword,        // fn
    ClassKeyword,     // class
    TypeKeyword,      // type
    ExternKeyword,    // extern
    ImportKeyword,    // import
    ReturnKeyword,    // return
    IfKeyword,        // if
    ElseKeyword,      // else
    WhileKeyword,     // while
    ForKeyword,       // for
    LoopKeyword,      // loop
    BreakKeyword,     // break
    ContinueKeyword,  // continue
    PubKeyword,       // pub
    PriKeyword,       // pri
    ConstructKeyword, // construct
    DestructKeyword,  // destruct
    I8Keyword,        // i8
    I16Keyword,       // i16
    I32Keyword,       // i32
    I64Keyword,       // i64
    U8Keyword,        // u8
    U16Keyword,       // u16
    U32Keyword,       // u32
    U64Keyword,       // u64
    F32Keyword,       // f32
    F64Keyword,       // f64
    VoidKeyword,      // void
    BoolKeyword,      // bool
    CharKeyword,      // char
    PtrKeyword,       // ptr
    LParen,           // (
    RParen,           // )
    LBrace,           // {
    RBrase,           // }
    LBracket,         // [
    RBracket,         // ]
    Semicolon,        // ;
    Plus,             // +
    Minus,            // -
    Asterisk,         // *
    Slash,            // /
    Percent,          // %
    Inc,              // ++
    Dec,              // --
    BitAnd,           // &
    BitOr,            // |
    BitXor,           // ^
    BitNot,           // ~
    LShift,           // <<
    RShift,           // >>
    Asign,            // =
    AddAsign,         // +=
    SubAsign,         // -=
    MulAsign,         // *=
    DivAsign,         // /=
    ModAsign,         // %=
    BitAndAsign,      // &=
    BitOrAsign,       // |=
    BitXorAsign,      // ^=
    LShiftAsign,      // <<=
    RShiftAsign,      // >>=
    LogicalAnd,       // &&
    LogicalOr,        // ||
    LogicalNot,       // !
    Equal,            // ==
    NotEqual,         // !=
    LessThan,         // <
    LessEqual,        // <=
    GreaterThan,      // >
    GreaterEqual,     // >=
    Dot,              // .
    Range,            // ..
    Colon,            // :
    Scope,            // ::
    Comma,            // ,
    Copy,             // @
    LineComment,      // //
    Integer,          // [0-9]+
    I8,               // [0-9]+i8
    I16,              // [0-9]+i16
    I32,              // [0-9]+i32
    I64,              // [0-9]+i64
    U8,               // [0-9]+u8
    U16,              // [0-9]+u16
    U32,              // [0-9]+u32
    U64,              // [0-9]+u64
    Float,            // [0-9].[0-9]
    F32,              // [0-9].[0-9]+f32
    F64,              // [0-9].[0-9]+f64
    Bool,             // true|false
    Null,             // null
    Char,             // 'x'
    String,           // "xxx"
    This,             // this
    Identify,         // xxx
  };
  struct Token
  {
    TokenKind kind;
    std::string value;
    size_t line;
    size_t letter;
    Token(TokenKind kind, const std::string& value, size_t line, size_t letter);
    Token(const Token& token);
    Token(Token&& token);
    Token& operator=(const Token& token);
    Token& operator=(Token&& token);
  };
  struct TokenSequence
  {
    std::string file;
    std::vector<std::string> rawFileData;
    std::vector<Token> tokens;
    std::string toOutputString(size_t line) const;
  };
  class Tokenizer
  {
    TokenSequence sequence;
    std::vector<std::string> errors;
    std::ifstream stream;
    bool done;
    Result<Option<Token>, std::string> findToken(std::string& str, size_t& line, size_t& letter);
  public:
    Tokenizer(const std::string& path);
    Result<TokenSequence, std::vector<std::string>> tokenize();
  };
}

#endif // PICKC_PARSER_TOKEN_H_