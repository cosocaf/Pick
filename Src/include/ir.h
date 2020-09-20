#pragma once

#include <unordered_map>

#include "module.h"

namespace pick::ir
{
    struct Type;
    enum struct Types
    {
        Undefine,
        Auto,
        LangDefine,
        UserDefine,
        Function,
        Array,
        SymbolTable,
    };
    enum struct LangDefineTypes
    {
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
        Bool,
        Char,
        Void,
    };
    enum struct UserDefineTypes
    {
        Class,
        Alias,
    };
    struct FunctionType
    {
        std::vector<Type> argTypes;
        Type* retType;
    };
    struct Array
    {
        Type* type;
        size_t size;
    };
    struct Type
    {
        Types type;
        union
        {
            LangDefineTypes lang;
            FunctionType* fn;
            Array array;
        };
    };

    struct Function;
    struct SymbolTable;
    struct Symbol
    {
        std::string name;
        syntax::Node* syntax;
        Type type;
        Function* init;
        union
        {
            Function* function;
            SymbolTable* table;
        };
    };
    struct SymbolTable
    {
        std::string name;
        std::unordered_map<std::string, Symbol> symbols;
        std::unordered_map<std::string, SymbolTable*> subSymbolTable;
        SymbolTable* parentSymbolTable;
    };
    struct Register
    {
        Symbol symbol;
        size_t lifeBegin;
        size_t lifeEnd;
    };
    struct InstructionBlock;
    enum struct InstructionType
    {
        Add,
        Sub,
        Mul,
        Div,
        Mod,
        Neg,
        Equal,
        NotEqual,
        GreaterEqual,
        GreaterThan,
        LessEqual,
        LessThan,
        Imm,
        Load,
        Store,
        Alloc,
        Br,
        Jmp,
        Phi,
        Call,
        Return,
    };
    struct BinaryOpInstruction
    {
        Register* dist;
        Register* left;
        Register* right;
    };
    struct NegInstruction
    {
        Register* dist;
        Register* src;
    };
    enum struct ImmType
    {
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
        Bool,
        Char,
    };
    struct ImmInstruction
    {
        Register* dist;
        ImmType type;
        union
        {
            int8_t i8;
            int16_t i16;
            int32_t i32;
            int64_t i64;
            uint8_t u8;
            uint16_t u16;
            uint32_t u32;
            uint64_t u64;
            float f32;
            double f64;
            bool b;
            char c;
        };
    };
    enum struct LoadType
    {
        GlobalVariable,
        Array,
    };
    struct LoadArray
    {
        Register* base;
        Register* suffix;
    };
    struct LoadInstruction
    {
        Register* dist;
        LoadType type;
        std::string globalVar;
        LoadArray array;
    };
    struct StoreInstruction
    {
        Register* base;
        Register* index;        // nullptrなら左辺値参照、そうでなければ配列参照
        Register* value;
        std::string globalVar;  // baseがnullptrであれば使用する
    };
    struct AllocInstruction
    {
        Register* dist;
        size_t size;
    };
    struct BrInstruction
    {
        Register* cond;
        InstructionBlock* ifBlock;
        InstructionBlock* elseBlock;
    };
    struct JmpInstruction
    {
        InstructionBlock* block;
    };
    struct PhiNode
    {
        Register* regs;
        InstructionBlock* from;
    };
    struct PhiInstruction
    {
        Register* dist;
        std::vector<PhiNode> nodes;
    };
    struct CallInstruction
    {
        Register* dist;
        Register* fn;
        std::vector<Register*> args;
    };
    struct ReturnInstruction
    {
        Register* reg;
    };
    struct Instruction
    {
        InstructionType type;
        union
        {
            BinaryOpInstruction* binary;
            NegInstruction* neg;
            ImmInstruction* imm;
            LoadInstruction* load;
            StoreInstruction* store;
            AllocInstruction* alloc;
            BrInstruction* br;
            JmpInstruction* jmp;
            PhiInstruction* phi;
            CallInstruction* call;
            ReturnInstruction* ret;
        };
    };
    struct InstructionBlock
    {
        std::vector<Instruction*> insts;
    };
    struct Function
    {
        SymbolTable* symbolTable;
        std::vector<Register*> regs;
        std::unordered_map<std::string, Register*> vars;
        std::vector<Register*> args;
        // blocks[0]が関数のエントリポイント
        // blocksが空の場合はextern関数として扱う
        std::vector<InstructionBlock*> blocks;
        // extern関数の名前
        std::string externName;
    };
    struct IntermediateRepresentation
    {
        // ::スコープのシンボルテーブル
        SymbolTable* rootSymbolTable;
        std::vector<Function*> functions;
    };
    class IR
    {
        IntermediateRepresentation ir;
        ModuleTree tree;
        Result<Register*, std::vector<std::string>> literalGenerate(const syntax::Literal* literal, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table);
        Result<Register*, std::vector<std::string>> variableGenerate(const syntax::Variable* var, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table);
        Result<Register*, std::vector<std::string>> blockGenerate(const syntax::Block* block, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table);
        Result<Register*, std::vector<std::string>> ifElseGenerate(const syntax::IfElse* ifElse, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table);
        Result<Register*, std::vector<std::string>> whileGenerate(const syntax::While* whi, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table);
        Result<Register*, std::vector<std::string>> primaryGenerate(const syntax::Primary* primary, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table);
        Result<Register*, std::vector<std::string>> backUnaryGenerate(const syntax::BackUnary* back, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table);
        Result<Register*, std::vector<std::string>> frontUnaryGenerate(const syntax::FrontUnary* front, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table);
        Result<Register*, std::vector<std::string>> factorGenerate(const syntax::Factor* factor, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table);
        Result<Register*, std::vector<std::string>> termGenerate(const syntax::Term* term, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table);
        Result<Register*, std::vector<std::string>> compGenerate(const syntax::Comparison* comp, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table);
        Result<Register*, std::vector<std::string>> asignGenerate(const syntax::Asign* asign, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table);
        Result<Register*, std::vector<std::string>> returnGenerate(const syntax::Return* ret, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table);
        Result<Register*, std::vector<std::string>> varDefGenerate(const syntax::VariableDefine* varDef, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table);
        Result<Register*, std::vector<std::string>> nodeGenerate(const syntax::Node* node, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table);
        Result<Function*, std::vector<std::string>> functionGenerate(const syntax::FunctionDefine* fnDef, SymbolTable* symbolTable);
        Result<Function*, std::vector<std::string>> globalVarDefGenerate(Symbol& symbol, SymbolTable* symbolTable);
        Result<_, std::vector<std::string>> generate(SymbolTable* table);
        Result<SymbolTable*, std::vector<std::string>> generate(const syntax::SyntaxTree& tree);
        Result<SymbolTable*, std::vector<std::string>> generate(const ModuleNode& node);
    public:
        IR(const ModuleTree& tree);
        Result<IntermediateRepresentation, std::vector<std::string>> generate();
    };

    size_t sizeOfType(const Type& type);
}