#include "dump.h"

#include <iostream>

namespace pick::dump
{
    namespace
    {
        constexpr auto INDENT = "    ";
        void _dump(const Module& mod, const std::string& prefix)
        {
            std::cout << prefix << "::" << mod.name << " #mod\n";
            std::cout << "source path: " << mod.srcPath << "\n";
            tokenDump(mod.tokenSequence);
            syntaxDump(mod.syntaxTree);
        }
        void _dump(const ModuleNode& node, const std::string& prefix)
        {
            std::cout << prefix << "::" << node.name << " #node\n";
            for (const auto& mod : node.modules) {
                _dump(mod.second, prefix + "::" + node.name);
            }
            for (const auto& child : node.children) {
                _dump(child.second, prefix + "::" + node.name);
            }
        }
        void _tokenDump(const token::Token& token)
        {
            using namespace token;
            switch (token.kind) {
            case TokenKind::FnKeyword: std::cout << "fn keyword"; break;
            case TokenKind::DefKeyword: std::cout << "def keyword"; break;
            case TokenKind::MutKeyword: std::cout << "mut keyword"; break;
            case TokenKind::ImportKeyword: std::cout << "import keyword"; break;
            case TokenKind::ExternKeyword: std::cout << "extern keyword"; break;
            case TokenKind::ReturnKeyword: std::cout << "return keyword"; break;
            case TokenKind::IfKeyword: std::cout << "if keyword"; break;
            case TokenKind::ElseKeyword: std::cout << "else keyword"; break;
            case TokenKind::SwitchKeyword: std::cout << "switch keyword"; break;
            case TokenKind::WhileKeyword: std::cout << "while keyword"; break;
            case TokenKind::LoopKeyword: std::cout << "loop keyword"; break;
            case TokenKind::ForKeyword: std::cout << "for keyword"; break;
            case TokenKind::PubKeyword: std::cout << "pub keyword"; break;
            case TokenKind::PriKeyword: std::cout << "pri keyword"; break;
            case TokenKind::ClassKeyword: std::cout << "class keyword"; break;
            case TokenKind::ConstructKeyword: std::cout << "construct keyword"; break;
            case TokenKind::DestructKeyword: std::cout << "destruct keyword"; break;
            case TokenKind::TypeKeyword: std::cout << "type keyword"; break;
            case TokenKind::I8Keyword: std::cout << "i8 keyword"; break;
            case TokenKind::I16Keyword: std::cout << "i16 keyword"; break;
            case TokenKind::I32Keyword: std::cout << "i32 keyword"; break;
            case TokenKind::I64Keyword: std::cout << "i64 keyword"; break;
            case TokenKind::U8Keyword: std::cout << "u8 keyword"; break;
            case TokenKind::U16Keyword: std::cout << "u16 keyword"; break;
            case TokenKind::U32Keyword: std::cout << "u32 keyword"; break;
            case TokenKind::U64Keyword: std::cout << "u64 keyword"; break;
            case TokenKind::F32Keyword: std::cout << "f32 keyword"; break;
            case TokenKind::F64Keyword: std::cout << "f64 keyword"; break;
            case TokenKind::VoidKeyword: std::cout << "void keyword"; break;
            case TokenKind::BoolKeyword: std::cout << "bool keyword"; break;
            case TokenKind::CharKeyword: std::cout << "char keyword"; break;
            case TokenKind::IntegerLiteral: std::cout << "integer literal(" << token.value << ")"; break;
            case TokenKind::FloatLiteral: std::cout << "float literal(" << token.value << ")"; break;
            case TokenKind::True: std::cout << "true"; break;
            case TokenKind::False: std::cout << "false"; break;
            case TokenKind::This: std::cout << "this"; break;
            case TokenKind::I8Literal: std::cout << "i8 literal(" << token.value << ")"; break;
            case TokenKind::I16Literal: std::cout << "i16 literal(" << token.value << ")"; break;
            case TokenKind::I32Literal: std::cout << "i32 literal(" << token.value << ")"; break;
            case TokenKind::I64Literal: std::cout << "i64 literal(" << token.value << ")"; break;
            case TokenKind::U8Literal: std::cout << "u8 literal(" << token.value << ")"; break;
            case TokenKind::U16Literal: std::cout << "u16 literal(" << token.value << ")"; break;
            case TokenKind::U32Literal: std::cout << "u32 literal(" << token.value << ")"; break;
            case TokenKind::U64Literal: std::cout << "u64 literal(" << token.value << ")"; break;
            case TokenKind::F32Literal: std::cout << "f32 literal(" << token.value << ")"; break;
            case TokenKind::F64Literal: std::cout << "f64 literal(" << token.value << ")"; break;
            case TokenKind::CharLiteral: std::cout << "char literal(" << token.value << ")"; break;
            case TokenKind::StringLiteral: std::cout << "string literal(" << token.value << ")"; break;
            case TokenKind::LParen: std::cout << "left paren"; break;
            case TokenKind::RParen: std::cout << "right paren"; break;
            case TokenKind::LBrace: std::cout << "left brace"; break;
            case TokenKind::RBrace: std::cout << "right brace"; break;
            case TokenKind::LBracket: std::cout << "left bracket"; break;
            case TokenKind::RBracket: std::cout << "right bracket"; break;
            case TokenKind::Colon: std::cout << "colon"; break;
            case TokenKind::Semicolon: std::cout << "semicolon"; break;
            case TokenKind::Dot: std::cout << "dot"; break;
            case TokenKind::Comma: std::cout << "comma"; break;
            case TokenKind::PlusSign: std::cout << "plus sign"; break;
            case TokenKind::MinusSign: std::cout << "minus sign"; break;
            case TokenKind::Asterisk: std::cout << "asterisk"; break;
            case TokenKind::Slash: std::cout << "slash"; break;
            case TokenKind::Percent: std::cout << "percent"; break;
            case TokenKind::AsignOperator: std::cout << "asign operator"; break;
            case TokenKind::AddAsignOperator: std::cout << "add asign operator"; break;
            case TokenKind::SubAsignOperator: std::cout << "sub asign operator"; break;
            case TokenKind::MulAsignOperator: std::cout << "mul asign operator"; break;
            case TokenKind::DivAsignOperator: std::cout << "div asign operator"; break;
            case TokenKind::ModAsignOperator: std::cout << "mod asign operator"; break;
            case TokenKind::EqualOperator: std::cout << "equal operator"; break;
            case TokenKind::NotOperator: std::cout << "not operator"; break;
            case TokenKind::GreaterEqualOperator: std::cout << "greater equal operator"; break;
            case TokenKind::GreaterThanOperator: std::cout << "greater than operator"; break;
            case TokenKind::LessEqualOperator: std::cout << "less equal operator"; break;
            case TokenKind::LessThanOperator: std::cout << "less than operator"; break;
            case TokenKind::NotEqualOperator: std::cout << "not equeal operator"; break;
            case TokenKind::IncOperator: std::cout << "inc operator"; break;
            case TokenKind::DecOperator: std::cout << "dec operator"; break;
            case TokenKind::ExpOperator: std::cout << "exp operator"; break;
            case TokenKind::ScopeOperator: std::cout << "scope operator"; break;
            case TokenKind::RangeOperator: std::cout << "range operator"; break;
            case TokenKind::LineComment: std::cout << "line comment(" << token.value << ")"; break;
            case TokenKind::Identify: std::cout << "identify(" << token.value << ")"; break;
            default:
                assert(false);
            }
            std::cout << "[" << token.letter << "]";
        }

        void nodeDump(const syntax::Node* node, const std::string& indent);
        void literalDump(const syntax::Literal* literal, const std::string& indent)
        {
            using namespace syntax;
            switch (literal->type) {
            case LiteralType::Integer: std::cout << indent << "Integer Literal: " << literal->integer; break;
            case LiteralType::I8: std::cout << indent << "I8 Literal: " << literal->i8; break;
            case LiteralType::I16: std::cout << indent << "I16 Literal: " << literal->i16; break;
            case LiteralType::I32: std::cout << indent << "I32 Literal: " << literal->i32; break;
            case LiteralType::I64: std::cout << indent << "I64 Literal: " << literal->i64; break;
            case LiteralType::U8: std::cout << indent << "U8 Literal: " << literal->u8; break;
            case LiteralType::U16: std::cout << indent << "U16 Literal: " << literal->u16; break;
            case LiteralType::U32: std::cout << indent << "U32 Literal: " << literal->u32; break;
            case LiteralType::U64: std::cout << indent << "U64 Literal: " << literal->u64; break;
            case LiteralType::Float: std::cout << indent << "Float Literal: " << literal->f; break;
            case LiteralType::F32: std::cout << indent << "F32 Literal: " << literal->f32; break;
            case LiteralType::F64: std::cout << indent << "F64 Literal: " << literal->f64; break;
            case LiteralType::True: std::cout << indent << "True"; break;
            case LiteralType::False: std::cout << indent << "False"; break;
            case LiteralType::Char: std::cout << indent << "Char Literal: " << literal->c; break;
            case LiteralType::String: std::cout << indent << "String Literal: " << *literal->string; break;
            case LiteralType::Array:
                std::cout << indent << "Array Literal: [";
                for (const auto& elem : literal->array->elems) {
                    std::cout << "\n";
                    nodeDump(elem, indent + INDENT);
                }
                if (!literal->array->elems.empty()) std::cout << "\n" << indent;
                std::cout << "]";
                break;
            case LiteralType::This: std::cout << indent << "This"; break;
            default: assert(false);
            }
        }
        void _varDump(const syntax::Variable* var)
        {
            std::cout << "::" << var->name;
            if (var->scope) _varDump(var->scope);
        }
        void varDump(const syntax::Variable* var, const std::string& indent)
        {
            std::cout << indent << "Variable: " << var->name;
            if (var->scope) {
                _varDump(var->scope);
            }
        }
        void blockDump(const syntax::Block* block, const std::string& indent)
        {
            std::cout << indent << "Block:";
            for (const auto& expr : block->exprs) {
                std::cout << '\n';
                nodeDump(expr, indent + INDENT);
            }
        }
        void ifElseDump(const syntax::IfElse* ifElse, const std::string& indent)
        {
            std::cout << indent << "If:\n";
            std::cout << indent << INDENT << "Cond:\n";
            nodeDump(ifElse->cond, indent + INDENT + INDENT);
            std::cout << '\n' << indent << INDENT << "Expr:\n";
            nodeDump(ifElse->ifExpr, indent + INDENT + INDENT);
            if (ifElse->elseExpr) {
                std::cout << '\n' << indent << INDENT << "Else:\n";
                nodeDump(ifElse->elseExpr, indent + INDENT + INDENT);
            }
        }
        void whileDump(const syntax::While* whi, const std::string& indent)
        {
            std::cout << indent << "While:\n";
            std::cout << indent << INDENT << "Cond:\n";
            nodeDump(whi->cond, indent + INDENT + INDENT);
            std::cout << '\n' << indent << INDENT << "Expr:\n";
            nodeDump(whi->expr, indent + INDENT + INDENT);
        }
        void primaryDump(const syntax::Primary* primary, const std::string& indent)
        {
            using namespace syntax;
            switch (primary->type) {
            case PrimaryType::Literal:
                literalDump(primary->literal, indent);
                break;
            case PrimaryType::Variable:
                varDump(primary->var, indent);
                break;
            case PrimaryType::Expression:
                nodeDump(primary->expr, indent);
                break;
            case PrimaryType::Block:
                blockDump(primary->block, indent);
                break;
            case PrimaryType::IfElse:
                ifElseDump(primary->ifElse, indent);
                break;
            case PrimaryType::While:
                whileDump(primary->whi, indent);
                break;
            default:
                assert(false);
            }
        }
        void backUnaryDump(const syntax::BackUnary* back, const std::string& indent)
        {
            using namespace syntax;
            switch (back->op) {
            case BackUnaryOperator::Inc:
                std::cout << indent << "Back Inc:\n";
                nodeDump(back->expr, indent + INDENT);
                break;
            case BackUnaryOperator::Dec:
                std::cout << indent << "Back Dec:\n";
                nodeDump(back->expr, indent + INDENT);
                break;
            case BackUnaryOperator::Member:
                std::cout << indent << "Member:\n";
                nodeDump(back->expr, indent + INDENT);
                std::cout << '\n';
                varDump(back->member, indent + INDENT);
                break;
            case BackUnaryOperator::Array:
                std::cout << indent << "Array:\n";
                nodeDump(back->expr, indent + INDENT);
                std::cout << '\n';
                nodeDump(back->suffix, indent + INDENT);
                break;
            case BackUnaryOperator::Call:
                std::cout << indent << "Call:\n";
                nodeDump(back->expr, indent + INDENT);
                if (!back->args->args.empty()) {
                    std::cout << '\n' << indent << "    Args:";
                    for (const auto& arg : back->args->args) {
                        std::cout << '\n';
                        nodeDump(arg, indent + INDENT + INDENT);
                    }
                }
                break;
            default:
                assert(false);
            }
        }
        void frontUnaryDump(const syntax::FrontUnary* front, const std::string& indent)
        {
            using namespace syntax;
            switch (front->op) {
            case FrontUnaryOperator::Inc: std::cout << indent << "Front Inc:\n"; break;
            case FrontUnaryOperator::Dec: std::cout << indent << "Front Dec:\n"; break;
            case FrontUnaryOperator::Plus: std::cout << indent << "Plus:\n"; break;
            case FrontUnaryOperator::Minus: std::cout << indent << "Minus:\n"; break;
            default: assert(false);
            }
            nodeDump(front->expr, indent + INDENT);
        }
        void factorDump(const syntax::Factor* factor, const std::string& indent)
        {
            using namespace syntax;
            switch (factor->op) {
            case FactorOperator::Mul: std::cout << indent << "Mul:\n"; break;
            case FactorOperator::Div: std::cout << indent << "Div:\n"; break;
            case FactorOperator::Mod: std::cout << indent << "Mod:\n"; break;
            default: assert(false);
            }
            nodeDump(factor->left, indent + INDENT);
            std::cout << '\n';
            nodeDump(factor->right, indent + INDENT);
        }
        void termDump(const syntax::Term* term, const std::string& indent)
        {
            using namespace syntax;
            switch (term->op) {
            case TermOperator::Add: std::cout << indent << "Add:\n"; break;
            case TermOperator::Sub: std::cout << indent << "Sub:\n"; break;
            default: assert(false);
            }
            nodeDump(term->left, indent + INDENT);
            std::cout << '\n';
            nodeDump(term->right, indent + INDENT);
        }
        void compDump(const syntax::Comparison* comp, const std::string& indent)
        {
            using namespace syntax;
            switch (comp->op) {
            case ComparisonOperator::Equal: std::cout << indent << "Equal:\n"; break;
            case ComparisonOperator::NotEqual: std::cout << indent << "Not Equal:\n"; break;
            case ComparisonOperator::GreaterEqual: std::cout << indent << "Greater Equal:\n"; break;
            case ComparisonOperator::GreaterThan: std::cout << indent << "Greater Than:\n"; break;
            case ComparisonOperator::LessEqual: std::cout << indent << "Less Equal:\n"; break;
            case ComparisonOperator::LessThan: std::cout << indent << "Less Than:\n"; break;
            default: assert(false);
            }
            nodeDump(comp->left, indent + INDENT);
            std::cout << '\n';
            nodeDump(comp->right, indent + INDENT);
        }
        void asignDump(const syntax::Asign* asign, const std::string& indent)
        {
            using namespace syntax;
            switch (asign->op) {
            case AsignOperator::Asign: std::cout << indent << "Asign:\n"; break;
            case AsignOperator::Add: std::cout << indent << "Add Asign:\n"; break;
            case AsignOperator::Sub: std::cout << indent << "Sub Asign:\n"; break;
            case AsignOperator::Mul: std::cout << indent << "Mul Asign:\n"; break;
            case AsignOperator::Div: std::cout << indent << "Div Asign:\n"; break;
            case AsignOperator::Mod: std::cout << indent << "Mod Asign:\n"; break;
            default: assert(false);
            }
            nodeDump(asign->left, indent + INDENT);
            std::cout << '\n';
            nodeDump(asign->right, indent + INDENT);
        }
        void typeDump(const syntax::Type* type, const std::string& indent)
        {
            using namespace syntax;
            std::cout << indent << "Type: ";
            switch (type->type) {
            case TypeType::I8: std::cout << "i8"; break;
            case TypeType::I16: std::cout << "i16"; break;
            case TypeType::I32: std::cout << "i32"; break;
            case TypeType::I64: std::cout << "i64"; break;
            case TypeType::U8: std::cout << "u8"; break;
            case TypeType::U16: std::cout << "u16"; break;
            case TypeType::U32: std::cout << "u32"; break;
            case TypeType::U64: std::cout << "u64"; break;
            case TypeType::F32: std::cout << "f32"; break;
            case TypeType::F64: std::cout << "f64"; break;
            case TypeType::Char: std::cout << "char"; break;
            case TypeType::Bool: std::cout << "bool"; break;
            case TypeType::Void: std::cout << "void"; break;
            case TypeType::Variable:
                std::cout << '\n';
                varDump(type->var, indent + INDENT);
                break;
            case TypeType::Array:
                std::cout << '\n';
                std::cout << indent << INDENT << "array:\n";
                typeDump(type->elem, indent + INDENT + INDENT);
                break;
            default: assert(false);
            }
        }
        void retDump(const syntax::Return* ret, const std::string& indent)
        {
            std::cout << indent << "Return:\n";
            nodeDump(ret->val, indent + INDENT);
        }
        void varDefDump(const syntax::VariableDefine* varDef, const std::string& indent)
        {
            std::cout << indent << "Variable Define:\n";
            varDump(varDef->var, indent + INDENT);
            if (varDef->type) {
                std::cout << '\n';
                typeDump(varDef->type, indent + INDENT);
            }
            if (varDef->init) {
                std::cout << '\n';
                nodeDump(varDef->init, indent + INDENT);
            }
        }
        void argDefDump(const syntax::ArgumentDefine* argDef, const std::string& indent)
        {
            std::cout << indent << "Argument Define:\n";
            varDump(argDef->var, indent + INDENT);
            if (argDef->type) {
                std::cout << '\n';
                typeDump(argDef->type, indent + INDENT);
            }
            if (argDef->init) {
                std::cout << '\n';
                nodeDump(argDef->init, indent + INDENT);
            }
        }
        void impModDump(const syntax::ImportModule* impMod, const std::string& indent)
        {
            std::cout << indent << "Import Module:\n";
            varDump(impMod->name, indent + INDENT);
        }
        void extDecDump(const syntax::ExternDeclare* extDec, const std::string& indent)
        {
            std::cout << indent << "Extern Declare:\n";
            varDump(extDec->name, indent + INDENT);
            std::cout << '\n';
            if (!extDec->args.empty()) {
                std::cout << indent << INDENT << "args:";
                for (const auto& arg : extDec->args) {
                    std::cout << '\n';
                    argDefDump(arg, indent + INDENT + INDENT);
                }
                std::cout << '\n';
            }
            std::cout << indent << INDENT << "return type:\n";
            typeDump(extDec->retType, indent + INDENT + INDENT);
        }
        void fnDefDump(const syntax::FunctionDefine* fnDef, const std::string& indent)
        {
            std::cout << indent << "Function Define:\n";
            if (fnDef->name) {
                varDump(fnDef->name, indent + INDENT);
                std::cout << '\n';
            }
            if (!fnDef->args.empty()) {
                std::cout << indent << INDENT << "args:";
                for (const auto& arg : fnDef->args) {
                    std::cout << '\n';
                    argDefDump(arg, indent + INDENT + INDENT);
                }
                std::cout << '\n';
            }
            if (fnDef->retType) {
                std::cout << indent << INDENT << "return type:\n";
                typeDump(fnDef->retType, indent + INDENT + INDENT);
                std::cout << '\n';
            }
            nodeDump(fnDef->expr, indent + INDENT);
        }
        void clsDefDump(const syntax::ClassDefine* clsDef, const std::string& indent)
        {
            std::cout << indent << "Class Define:\n";
            if (clsDef->construct) {
                std::cout << indent << "    construct:\n";
                fnDefDump(clsDef->construct, indent + INDENT + INDENT);
                std::cout << '\n';
            }
            if (clsDef->destruct) {
                std::cout << indent << "    destruct:\n";
                fnDefDump(clsDef->destruct, indent + INDENT + INDENT);
                std::cout << '\n';
            }
            if (!clsDef->members.empty()) {
                std::cout << indent << "    members:\n";
                for (const auto& member : clsDef->members) {
                    varDefDump(member, indent + INDENT + INDENT);
                }
            }
        }
        void alsDefDump(const syntax::AliasDefine* alsDef, const std::string& indent)
        {
            std::cout << indent << "Alias Define:\n";
            std::cout << indent << "    name:\n";
            varDump(alsDef->name, indent + INDENT + INDENT);
            std::cout << '\n';
            std::cout << indent << "    type:\n";
            varDump(alsDef->original, indent + INDENT + INDENT);
        }
        void nodeDump(const syntax::Node* node, const std::string& indent)
        {
            using namespace syntax;
            switch (node->kind) {
            case NodeKind::BackUnary:
                backUnaryDump(node->back, indent);
                break;
            case NodeKind::FrontUnary:
                frontUnaryDump(node->front, indent);
                break;
            case NodeKind::Factor:
                factorDump(node->factor, indent);
                break;
            case NodeKind::Term:
                termDump(node->term, indent);
                break;
            case NodeKind::Comparison:
                compDump(node->comp, indent);
                break;
            case NodeKind::Asign:
                asignDump(node->asign, indent);
                break;
            case NodeKind::Primary:
                primaryDump(node->primary, indent);
                break;
            case NodeKind::Literal:
                literalDump(node->literal, indent);
                break;
            case NodeKind::Variable:
                varDump(node->var, indent);
                break;
            case NodeKind::Type:
                typeDump(node->type, indent);
                break;
            case NodeKind::Block:
                blockDump(node->block, indent);
                break;
            case NodeKind::Return:
                retDump(node->ret, indent);
                break;
            case NodeKind::ImportModule:
                impModDump(node->impMod, indent);
                break;
            case NodeKind::ExternDeclare:
                extDecDump(node->extDec, indent);
                break;
            case NodeKind::VariableDefine:
                varDefDump(node->varDef, indent);
                break;
            case NodeKind::ArgumentDefine:
                argDefDump(node->argDef, indent);
                break;
            case NodeKind::FunctionDefine:
                fnDefDump(node->fnDef, indent);
                break;
            case NodeKind::ClassDefine:
                clsDefDump(node->clsDef, indent);
                break;
            case NodeKind::AliasDefine:
                alsDefDump(node->alsDef, indent);
                break;
            default:
                assert(false);
            }
        }
    }
    void moduleDump(const ModuleTree& mod)
    {
        for (const auto& node : mod.nodes) {
            _dump(node.second, "");
        }
    }
    void tokenDump(const token::TokenSequence& sequence)
    {
        std::cout << "token sequence: \n    #1 ";
        size_t line = 1;
        for (const auto& token : sequence.tokens) {
            if (token.line > line) {
                line = token.line;
                std::cout << "\n    #" << line << " ";
            }
            _tokenDump(token);
            std::cout << ", ";
        }
    }
    void syntaxDump(const syntax::SyntaxTree& tree)
    {
        std::cout << "\nsyntax tree: \n";
        for (const auto& expr : tree.exprs) {
            nodeDump(expr, INDENT);
            std::cout << '\n';
        }
    }
    template<typename T>
    inline size_t indexOf(const std::vector<T>& vec, const T& val)
    {
        auto itr = std::find(vec.begin(), vec.end(), val);
        assert(itr != vec.end());
        return std::distance(vec.begin(), itr);
    }
    void blockDump(const ir::Function* fn, const ir::InstructionBlock* block, const std::string& indent)
    {
        using namespace ir;
        int instCount = 0;
        for (auto& inst : block->insts) {
            std::cout << '\n' << indent;
            switch (inst->type) {
            case InstructionType::Add:
                std::cout << "add #" << indexOf(fn->regs, inst->binary->dist) << " #" << indexOf(fn->regs, inst->binary->left) << " #" << indexOf(fn->regs, inst->binary->right);
                break;
            case InstructionType::Sub:
                std::cout << "sub #" << indexOf(fn->regs, inst->binary->dist) << " #" << indexOf(fn->regs, inst->binary->left) << " #" << indexOf(fn->regs, inst->binary->right);
                break;
            case InstructionType::Mul:
                std::cout << "mul #" << indexOf(fn->regs, inst->binary->dist) << " #" << indexOf(fn->regs, inst->binary->left) << " #" << indexOf(fn->regs, inst->binary->right);
                break;
            case InstructionType::Div:
                std::cout << "div #" << indexOf(fn->regs, inst->binary->dist) << " #" << indexOf(fn->regs, inst->binary->left) << " #" << indexOf(fn->regs, inst->binary->right);
                break;
            case InstructionType::Mod:
                std::cout << "mod #" << indexOf(fn->regs, inst->binary->dist) << " #" << indexOf(fn->regs, inst->binary->left) << " #" << indexOf(fn->regs, inst->binary->right);
                break;
            case InstructionType::Equal:
                std::cout << "equal #" << indexOf(fn->regs, inst->binary->dist) << " #" << indexOf(fn->regs, inst->binary->left) << " #" << indexOf(fn->regs, inst->binary->right);
                break;
            case InstructionType::NotEqual:
                std::cout << "not equal #" << indexOf(fn->regs, inst->binary->dist) << " #" << indexOf(fn->regs, inst->binary->left) << " #" << indexOf(fn->regs, inst->binary->right);
                break;
            case InstructionType::GreaterEqual:
                std::cout << "greater equal #" << indexOf(fn->regs, inst->binary->dist) << " #" << indexOf(fn->regs, inst->binary->left) << " #" << indexOf(fn->regs, inst->binary->right);
                break;
            case InstructionType::GreaterThan:
                std::cout << "greater than #" << indexOf(fn->regs, inst->binary->dist) << " #" << indexOf(fn->regs, inst->binary->left) << " #" << indexOf(fn->regs, inst->binary->right);
                break;
            case InstructionType::LessEqual:
                std::cout << "less equal #" << indexOf(fn->regs, inst->binary->dist) << " #" << indexOf(fn->regs, inst->binary->left) << " #" << indexOf(fn->regs, inst->binary->right);
                break;
            case InstructionType::LessThan:
                std::cout << "less than #" << indexOf(fn->regs, inst->binary->dist) << " #" << indexOf(fn->regs, inst->binary->left) << " #" << indexOf(fn->regs, inst->binary->right);
                break;
            case InstructionType::Neg:
                std::cout << "neg #" << indexOf(fn->regs, inst->neg->dist) << " #" << indexOf(fn->regs, inst->neg->src);
                break;
            case InstructionType::Imm:
                switch (inst->imm->type) {
                case ImmType::I8:
                    std::cout << "imm #" << indexOf(fn->regs, inst->imm->dist) << " i8 " << (int)inst->imm->i8;
                    break;
                case ImmType::I16:
                    std::cout << "imm #" << indexOf(fn->regs, inst->imm->dist) << " i16 " << inst->imm->i16;
                    break;
                case ImmType::I32:
                    std::cout << "imm #" << indexOf(fn->regs, inst->imm->dist) << " i32 " << inst->imm->i32;
                    break;
                case ImmType::I64:
                    std::cout << "imm #" << indexOf(fn->regs, inst->imm->dist) << " i64 " << inst->imm->i64;
                    break;
                case ImmType::U8:
                    std::cout << "imm #" << indexOf(fn->regs, inst->imm->dist) << " u8 " << (unsigned int)inst->imm->u8;
                    break;
                case ImmType::U16:
                    std::cout << "imm #" << indexOf(fn->regs, inst->imm->dist) << " u16 " << inst->imm->u16;
                    break;
                case ImmType::U32:
                    std::cout << "imm #" << indexOf(fn->regs, inst->imm->dist) << " u32 " << inst->imm->u32;
                    break;
                case ImmType::U64:
                    std::cout << "imm #" << indexOf(fn->regs, inst->imm->dist) << " u64 " << inst->imm->u64;
                    break;
                case ImmType::F32:
                    std::cout << "imm #" << indexOf(fn->regs, inst->imm->dist) << " f32 " << inst->imm->f32;
                    break;
                case ImmType::F64:
                    std::cout << "imm #" << indexOf(fn->regs, inst->imm->dist) << " f64 " << inst->imm->f64;
                    break;
                case ImmType::Char:
                    std::cout << "imm #" << indexOf(fn->regs, inst->imm->dist) << " char " << inst->imm->c;
                    break;
                case ImmType::Bool:
                    std::cout << "imm #" << indexOf(fn->regs, inst->imm->dist) << " char " << inst->imm->b;
                    break;
                default:
                    assert(false);
                }
                break;
            case InstructionType::Load:
                std::cout << "load #" << indexOf(fn->regs, inst->load->dist);
                switch (inst->load->type) {
                case LoadType::GlobalVariable:
                    std::cout << " global %" << inst->load->globalVar;
                    break;
                case LoadType::Array:
                    std::cout << " array #" << indexOf(fn->regs, inst->load->array.base) << "[#" << indexOf(fn->regs, inst->load->array.suffix) << "]";
                    break;
                default:
                    assert(false);
                }
                break;
            case InstructionType::Store:
                std::cout << "store #" << indexOf(fn->regs, inst->store->base);
                if (inst->store->index) {
                    std::cout << "[#" << indexOf(fn->regs, inst->store->index) << "]";
                }
                std::cout << " #" << indexOf(fn->regs, inst->store->value);
                break;
            case InstructionType::Alloc:
                std::cout << "alloc #" << indexOf(fn->regs, inst->alloc->dist) << " size: " << inst->alloc->size;
                break;
            case InstructionType::Br:
                std::cout << "br:\n";
                std::cout << indent << INDENT << "cond: #" << indexOf(fn->regs, inst->br->cond) << "\n";
                std::cout << indent << INDENT << "if: block#" << indexOf(fn->blocks, inst->br->ifBlock);
                if (inst->br->elseBlock) {
                    std::cout << "\n" << indent << INDENT << "else: block#" << indexOf(fn->blocks, inst->br->elseBlock);
                }
                break;
            case InstructionType::Jmp:
                std::cout << "jmp: block#" << indexOf(fn->blocks, inst->jmp->block);
                break;
            case InstructionType::Phi:
                std::cout << "phi: #" << indexOf(fn->regs, inst->phi->dist) << " [";
                for (const auto& node : inst->phi->nodes) {
                    std::cout << "(#" << indexOf(fn->regs, node.regs) << " from block#" << indexOf(fn->blocks, node.from) << ") ";
                }
                std::cout << "]";
                break;
            case InstructionType::Call:
                std::cout << "call:\n";
                std::cout << indent << INDENT << "dist: #" << indexOf(fn->regs, inst->call->dist) << "\n";
                std::cout << indent << INDENT << "fn:   #" << indexOf(fn->regs, inst->call->fn) << "\n";
                std::cout << indent << INDENT << "args: [";
                for (const auto& arg : inst->call->args) {
                    std::cout << "#" << indexOf(fn->regs, arg) << " ";
                }
                std::cout << "]";
                break;
            case InstructionType::Return:
                if (inst->ret->reg) {
                    std::cout << "ret #" << indexOf(fn->regs, inst->ret->reg);
                }
                else {
                    std::cout << "ret void";
                }
                break;
            default:
                assert(false);
            }
            ++instCount;
        }
    }
    void typeDump(const ir::Type& type)
    {
        using namespace ir;
        switch (type.type) {
        case Types::Undefine:
            std::cout << "undefine";
            break;
        case Types::Auto:
            std::cout << "auto";
            break;
        case Types::LangDefine:
            switch (type.lang) {
            case LangDefineTypes::I8: std::cout << "i8"; break;
            case LangDefineTypes::I16: std::cout << "i16"; break;
            case LangDefineTypes::I32: std::cout << "i32"; break;
            case LangDefineTypes::I64: std::cout << "i64"; break;
            case LangDefineTypes::U8: std::cout << "u8"; break;
            case LangDefineTypes::U16: std::cout << "u16"; break;
            case LangDefineTypes::U32: std::cout << "u32"; break;
            case LangDefineTypes::U64: std::cout << "u64"; break;
            case LangDefineTypes::F32: std::cout << "f32"; break;
            case LangDefineTypes::F64: std::cout << "f64"; break;
            case LangDefineTypes::Bool: std::cout << "bool"; break;
            case LangDefineTypes::Char: std::cout << "char"; break;
            case LangDefineTypes::Void: std::cout << "void"; break;
            }
            break;
        case Types::UserDefine:
            assert(false);
            break;
        case Types::Function:
            std::cout << "fn (";
            if (!type.fn->argTypes.empty()) {
                typeDump(type.fn->argTypes[0]);
                for (int i = 1, l = type.fn->argTypes.size(); i < l; ++i) {
                    std::cout << ", ";
                    typeDump(type.fn->argTypes[i]);
                }
            }
            std::cout << ") ";
            typeDump(*type.fn->retType);
            break;
        case Types::Array:
            std::cout << "array [";
            typeDump(*type.array.type);
            std::cout << " x " << type.array.size << "]";
            break;
        default:
            assert(false);
        }
    }
    void irDump(const ir::IntermediateRepresentation& ir)
    {
        using namespace ir;
        std::cout << "intermediate representation:";
        int fnCount = 0;
        for (auto& fn : ir.functions) {
            if (fn->blocks.empty()) {
                std::cout << "\nextern function #" << fnCount;
                continue;
            }
            std::cout << "\nfunction #" << fnCount << ": \n";
            int regCount = 0;
            std::cout << INDENT << "registers:\n";
            for (const auto reg : fn->regs) {
                std::cout << INDENT << INDENT << "#" << regCount << " begin: " << reg->lifeBegin << ", end: " << reg->lifeEnd << ", type: ";
                typeDump(reg->symbol.type);
                std::cout << '\n';
                ++regCount;
            }
            int varCount = 0;
            std::cout << INDENT << "variables:\n";
            for (const auto& var : fn->vars) {
                std::cout << INDENT << INDENT << "#" << varCount << " " << var.first << "\n";
                ++varCount;
            }
            int blockCount = 0;
            for (auto& block : fn->blocks) {
                std::cout << INDENT << "block #" << blockCount << ":";
                blockDump(fn, block, std::string(INDENT) + INDENT);
                std::cout << "\n";
                ++blockCount;
            }
            ++fnCount;
        }
        std::cout << '\n';
    }
}