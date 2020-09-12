#pragma once

#include "token.h"

namespace pick::syntax
{
    enum struct NodeKind
    {
        BackUnary,
        FrontUnary,
        Factor,
        Term,
        Comparison,
        Asign,
        Primary,
        Literal,
        Variable,
        Type,
        Block,
        Return,
        ExternDeclare,
        ImportModule,
        VariableDefine,
        ArgumentDefine,
        FunctionDefine,
        ClassDefine,
        AliasDefine,
    };
    struct Node;
    struct Literal;
    struct Variable;
    struct Block;
    struct IfElse;
    struct While;
    enum struct PrimaryType
    {
        Literal,
        Variable,
        Block,
        IfElse,
        While,
        Expression,
    };
    struct Primary
    {
        PrimaryType type;
        union
        {
            Literal* literal;
            Variable* var;
            Block* block;
            IfElse* ifElse;
            While* whi;
            Node* expr;
        };
    };
    enum struct BackUnaryOperator
    {
        Inc,
        Dec,
        Member,
        Array,
        Call
    };
    struct Arguments
    {
        std::vector<Node*> args;
    };
    struct BackUnary
    {
        BackUnaryOperator op;
        Node* expr;
        Node* suffix;
        Variable* member;
        Arguments* args;
    };
    enum struct FrontUnaryOperator
    {
        Plus,
        Minus,
        Inc,
        Dec
    };
    struct FrontUnary
    {
        FrontUnaryOperator op;
        Node* expr;
    };
    enum struct FactorOperator
    {
        Mul,
        Div,
        Mod
    };
    struct Factor
    {
        FactorOperator op;
        Node* left;
        Node* right;
    };
    enum struct TermOperator
    {
        Add,
        Sub,
    };
    struct Term
    {
        TermOperator op;
        Node* left;
        Node* right;
    };
    enum struct ComparisonOperator
    {
        Equal,
        NotEqual,
        GreaterEqual,
        GreaterThan,
        LessEqual,
        LessThan,
    };
    struct Comparison
    {
        ComparisonOperator op;
        Node* left;
        Node* right;
    };
    enum struct AsignOperator
    {
        Asign,
        Add,
        Sub,
        Mul,
        Div,
        Mod
    };
    struct Asign
    {
        AsignOperator op;
        Node* left;
        Node* right;
    };
    enum struct LiteralType
    {
        Integer,
        I8,
        I16,
        I32,
        I64,
        U8,
        U16,
        U32,
        U64,
        Float,
        F32,
        F64,
        True,
        False,
        Char,
        String,
        Array,
        This,
    };
    struct ArrayLiteral
    {
        std::vector<Node*> elems;
    };
    struct Literal
    {
        LiteralType type;
        union
        {
            int32_t integer;
            int8_t i8;
            int16_t i16;
            int32_t i32;
            int64_t i64;
            uint8_t u8;
            uint16_t u16;
            uint32_t u32;
            uint64_t u64;
            double f;
            float f32;
            double f64;
            char c;
            std::string* string;
            ArrayLiteral* array;
        };
    };
    struct Variable
    {
        std::string name;
        Variable* scope;
    };
    enum struct TypeType
    {
        Variable,
        I8,
        I16,
        I32,
        I64,
        U8,
        U16,
        U32,
        U64,
        F32,
        F64,
        Char,
        Bool,
        Void,
        Array,
    };
    struct Type
    {
        TypeType type;
        union {
            Variable* var;
            Type* elem;
        };
    };
    struct Block
    {
        std::vector<Node*> exprs;
    };
    struct IfElse
    {
        Node* cond;
        Node* ifExpr;
        Node* elseExpr;
    };
    struct While
    {
        Node* cond;
        Node* expr;
    };
    struct Return
    {
        Node* val;
    };
    struct VariableDefine
    {
        Variable* var;
        Type* type;
        Node* init;
    };
    struct ArgumentDefine
    {
        Variable* var;
        Type* type;
        Node* init;
    };
    struct FunctionDefine
    {
        Variable* name;
        std::vector<ArgumentDefine*> args;
        Type* retType;
        Node* expr;
    };
    struct ExternDeclare
    {
        Variable* name;
        std::vector<ArgumentDefine*> args;
        Type* retType;
    };
    struct ImportModule
    {
        Variable* name;
    };
    struct ClassDefine
    {
        std::vector<VariableDefine*> members;
        FunctionDefine* construct;
        FunctionDefine* destruct;
    };
    struct AliasDefine
    {
        Variable* name;
        Variable* original;
    };
    struct Node
    {
        NodeKind kind;
        union
        {
            BackUnary* back;
            FrontUnary* front;
            Factor* factor;
            Term* term;
            Comparison* comp;
            Asign* asign;
            Primary* primary;
            Literal* literal;
            Variable* var;
            Type* type;
            Block* block;
            Return* ret;
            ImportModule* impMod;
            ExternDeclare* extDec;
            VariableDefine* varDef;
            ArgumentDefine* argDef;
            FunctionDefine* fnDef;
            ClassDefine* clsDef;
            AliasDefine* alsDef;
        };
    };

    struct SyntaxTree
    {
        std::vector<Node*> exprs;
    };
    class Parser
    {
        SyntaxTree tree;
        std::vector<std::string> errors;
        bool done;

        token::TokenSequence sequence;
        size_t currentSequence;
        Result<token::Token, std::vector<std::string>> next();
        Result<token::Token, std::vector<std::string>> current();
        void back();
        std::vector<std::string> createErrorMessage(const std::string& message, const token::Token* source = nullptr);
        void skip(size_t size = 1);
        // End Of Sequence
        bool eos();
        bool isBackUnaryOperator(token::TokenKind kind);
        bool isFrontUnaryOperator(token::TokenKind kind);
        bool isFactorOperator(token::TokenKind kind);
        bool isTermOperator(token::TokenKind kind);
        bool isCompOperator(token::TokenKind kind);
        bool isAsignOperator(token::TokenKind kind);
        bool isLiteral(token::TokenKind kind);
        Result<Node*, std::vector<std::string>> backUnary();
        Result<Node*, std::vector<std::string>> frontUnary();
        Result<Node*, std::vector<std::string>> factor();
        Result<Node*, std::vector<std::string>> term();
        Result<Node*, std::vector<std::string>> comp();
        Result<Node*, std::vector<std::string>> asign();
        Result<Node*, std::vector<std::string>> literal();
        Result<Variable*, std::vector<std::string>> variable();
        Result<Type*, std::vector<std::string>> type();
        Result<Return*, std::vector<std::string>> ret();
        Result<VariableDefine*, std::vector<std::string>> varDef();
        Result<ArgumentDefine*, std::vector<std::string>> argDef();
        Result<ExternDeclare*, std::vector<std::string>> externDec();
        Result<ImportModule*, std::vector<std::string>> importMod();
        Result<FunctionDefine*, std::vector<std::string>> fnDef();
        Result<ClassDefine*, std::vector<std::string>> clsDef();
        Result<AliasDefine*, std::vector<std::string>> alsDef();
        Result<Node*, std::vector<std::string>> block();
        Result<Node*, std::vector<std::string>> expr();
        Result<Node*, std::vector<std::string>> primary();
    public:
        Parser(const token::TokenSequence& sequence);
        Result<SyntaxTree, std::vector<std::string>> parse();
    };
}