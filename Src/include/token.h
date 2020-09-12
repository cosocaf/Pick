#pragma once

#include "vector_utils.h"
#include "result.h"

namespace pick::token
{
    enum struct TokenKind
    {
        FnKeyword,
        DefKeyword,
        MutKeyword,
        ExternKeyword,
        ImportKeyword,
        ReturnKeyword,
        IfKeyword,
        ElseKeyword,
        SwitchKeyword,
        WhileKeyword,
        LoopKeyword,
        ForKeyword,
        PubKeyword,
        PriKeyword,
        ClassKeyword,
        ConstructKeyword,
        DestructKeyword,
        TypeKeyword,
        I8Keyword,
        I16Keyword,
        I32Keyword,
        I64Keyword,
        U8Keyword,
        U16Keyword,
        U32Keyword,
        U64Keyword,
        F32Keyword,
        F64Keyword,
        VoidKeyword,
        BoolKeyword,
        CharKeyword,
        IntegerLiteral,
        FloatLiteral,
        True,
        False,
        This,
        I8Literal,
        I16Literal,
        I32Literal,
        I64Literal,
        U8Literal,
        U16Literal,
        U32Literal,
        U64Literal,
        F32Literal,
        F64Literal,
        CharLiteral,
        StringLiteral,
        LParen,         // (
        RParen,         // )
        LBrace,         // {
        RBrace,         // }
        LBracket,       // [
        RBracket,       // ]
        Colon,
        Semicolon,
        Dot,
        Comma,
        PlusSign,
        MinusSign,
        Asterisk,
        Slash,
        Percent,
        AsignOperator,
        AddAsignOperator,
        SubAsignOperator,
        MulAsignOperator,
        DivAsignOperator,
        ModAsignOperator,
        EqualOperator,
        NotOperator,
        GreaterEqualOperator,
        GreaterThanOperator,
        LessEqualOperator,
        LessThanOperator,
        NotEqualOperator,
        IncOperator,
        DecOperator,
        ExpOperator,
        ScopeOperator,
        RangeOperator,
        LineComment,
        Identify
    };
    struct Token
    {
        TokenKind kind;
        size_t line;
        size_t letter;
        std::string value;
    public:
        Token(TokenKind kind, size_t line, size_t letter, const std::string& value);
    };
    struct TokenSequence
    {
        std::string fileName;
        std::vector<Token> tokens;
    };
    class Tokenizer
    {
        TokenSequence sequence;
        std::vector<std::string> errors;
        bool done;

        size_t line;
        size_t letter;
        std::string buf;

        void token();
    public:
        Tokenizer(const std::string& path);
        Result<TokenSequence, std::vector<std::string>> tokenize();
    };
}