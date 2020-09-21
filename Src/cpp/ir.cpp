#include "ir.h"

#include <optional>
#include <unordered_set>
#include <sstream>

namespace pick::ir
{
    namespace
    {
        template<typename K, typename V>
        inline bool exists(const std::unordered_map<K, V>& map, const K& key)
        {
            return map.find(key) != map.end();
        }
        Type syntaxTypeToIrType(const syntax::Type* type)
        {
            using namespace syntax;
            Type result{};
            if (type == nullptr) {
                result.type = Types::Auto;
                return result;
            }
            result.type = Types::LangDefine;
            switch (type->type) {
            case TypeType::I8: result.lang = LangDefineTypes::I8; break;
            case TypeType::I16: result.lang = LangDefineTypes::I16; break;
            case TypeType::I32: result.lang = LangDefineTypes::I32; break;
            case TypeType::I64: result.lang = LangDefineTypes::I64; break;
            case TypeType::U8: result.lang = LangDefineTypes::U8; break;
            case TypeType::U16: result.lang = LangDefineTypes::U16; break;
            case TypeType::U32: result.lang = LangDefineTypes::U32; break;
            case TypeType::U64: result.lang = LangDefineTypes::U64; break;
            case TypeType::F32: result.lang = LangDefineTypes::F32; break;
            case TypeType::F64: result.lang = LangDefineTypes::F64; break;
            case TypeType::Bool: result.lang = LangDefineTypes::Bool; break;
            case TypeType::Char: result.lang = LangDefineTypes::Char; break;
            case TypeType::Void: result.lang = LangDefineTypes::Void; break;
            case TypeType::Variable:
                assert(false);
                break;
            case TypeType::Array:
                result.type = Types::Array;
                result.array.type = new Type(syntaxTypeToIrType(type->elem));
                result.array.size = 0;
                break;
            default:
                assert(false);
                break;
            }
            return result;
        }
        bool operator==(const Type& a, const Type& b)
        {
            if (a.type != b.type) return false;
            switch (a.type) {
            case Types::Auto: return true;
            case Types::LangDefine:
                return a.lang == b.lang;
            case Types::UserDefine:
                assert(false);
                return false;
            case Types::Function:
                assert(false);
                return false;
            case Types::Array:
                return (*a.array.type == *b.array.type) && (a.array.size == b.array.size);
            }
        }
        bool operator!=(const Type& a, const Type& b)
        {
            return !(a == b);
        }
        std::optional<std::pair<Symbol, std::string>> findSymbol(SymbolTable* cur, const syntax::Variable* curName)
        {
            std::string name = cur->name + "::";
            auto parent = cur->parentSymbolTable;
            while (parent) {
                name.insert(0, parent->name + "::");
                parent = parent->parentSymbolTable;
            }
            while (true) {
                if (curName->scope) {
                    if (exists(cur->subSymbolTable, curName->name)) {
                        name += curName->name + "::";
                        cur = cur->subSymbolTable[curName->name];
                        curName = curName->scope;
                    }
                    else {
                        return std::nullopt;
                    }
                }
                else {
                    if (exists(cur->symbols, curName->name)) {
                        return std::make_pair(cur->symbols[curName->name], name + curName->name);
                    }
                    else {
                        return std::nullopt;
                    }
                }
            }
        }
        std::optional<std::pair<Symbol, std::string>> findSymbol(SymbolTable* table, const syntax::Variable* var, const IntermediateRepresentation& ir)
        {
            // 自分の子モジュールから検索
            auto symbol = findSymbol(table, var);
            // 自分の兄弟モジュールから検索
            if (!symbol) {
                if (table->parentSymbolTable) {
                    symbol = findSymbol(table->parentSymbolTable, var);
                }
            }
            // ルートモジュールから検索
            if (!symbol) {
                symbol = findSymbol(ir.rootSymbolTable, var);
            }
            return symbol;
        }
        void calcLifeTime(Function* fn)
        {
            std::unordered_set<Register*> vars;
            for (const auto& arg : fn->args) {
                vars.insert(arg);
            }
            size_t lifeTime = 0;
            for (auto& block : fn->blocks) {
                for (auto& inst : block->insts) {
                    switch (inst->type) {
                    case InstructionType::Add:
                    case InstructionType::Sub:
                    case InstructionType::Mul:
                    case InstructionType::Div:
                    case InstructionType::Mod:
                    case InstructionType::Equal:
                    case InstructionType::NotEqual:
                    case InstructionType::GreaterEqual:
                    case InstructionType::GreaterThan:
                    case InstructionType::LessEqual:
                    case InstructionType::LessThan:
                        assert(vars.find(inst->binary->dist) == vars.end());
                        assert(vars.find(inst->binary->left) != vars.end());
                        assert(vars.find(inst->binary->right) != vars.end());
                        inst->binary->dist->lifeBegin = inst->binary->dist->lifeEnd = lifeTime;
                        vars.insert(inst->binary->dist);
                        inst->binary->left->lifeEnd = lifeTime;
                        inst->binary->right->lifeEnd = lifeTime;
                        break;
                    case InstructionType::Neg:
                        if (vars.find(inst->neg->dist) == vars.end()) {
                            inst->neg->dist->lifeBegin = inst->neg->dist->lifeEnd = lifeTime;
                            vars.insert(inst->neg->dist);
                        }
                        else {
                            inst->neg->dist->lifeEnd = lifeTime;
                        }
                        assert(vars.find(inst->neg->src) != vars.end());
                        inst->neg->src->lifeEnd = lifeTime;
                        break;
                    case InstructionType::Imm:
                        if (vars.find(inst->imm->dist) == vars.end()) {
                            inst->imm->dist->lifeBegin = inst->imm->dist->lifeEnd = lifeTime;
                            vars.insert(inst->imm->dist);
                        }
                        else {
                            inst->imm->dist->lifeEnd = lifeTime;
                        }
                        break;
                    case InstructionType::Load:
                        if (vars.find(inst->load->dist) == vars.end()) {
                            inst->load->dist->lifeBegin = inst->load->dist->lifeEnd = lifeTime;
                            vars.insert(inst->load->dist);
                        }
                        else {
                            inst->load->dist->lifeEnd = lifeTime;
                        }
                        if (inst->load->type == LoadType::Array) {
                            assert(vars.find(inst->load->array.base) != vars.end());
                            assert(vars.find(inst->load->array.suffix) != vars.end());
                            inst->load->array.base->lifeEnd = lifeTime;
                            inst->load->array.suffix->lifeEnd = lifeTime;
                        }
                        break;
                    case InstructionType::Store:
                        assert(vars.find(inst->store->value) != vars.end());
                        if (inst->store->base) {
                            if (vars.find(inst->store->base) == vars.end()) {
                                inst->store->base->lifeBegin = inst->store->base->lifeEnd = lifeTime;
                                vars.insert(inst->store->base);
                            }
                            inst->store->base->lifeEnd = lifeTime;
                        }
                        inst->store->value->lifeEnd = lifeTime;
                        if (inst->store->index) {
                            assert(vars.find(inst->store->index) != vars.end());
                            inst->store->index->lifeEnd = lifeTime;
                        }
                        break;
                    case InstructionType::Alloc:
                        assert(vars.find(inst->alloc->dist) == vars.end());
                        inst->alloc->dist->lifeBegin = inst->alloc->dist->lifeEnd = lifeTime;
                        vars.insert(inst->alloc->dist);
                        break;
                    case InstructionType::Br:
                        assert(vars.find(inst->br->cond) != vars.end());
                        inst->br->cond->lifeEnd = lifeTime;
                        break;
                    case InstructionType::Jmp:
                        break;
                    case InstructionType::Phi:
                        assert(vars.find(inst->phi->dist) == vars.end());
                        inst->phi->dist->lifeBegin = inst->phi->dist->lifeEnd = lifeTime;
                        vars.insert(inst->phi->dist);
                        break;
                    case InstructionType::Call:
                        assert(vars.find(inst->call->dist) == vars.end());
                        assert(vars.find(inst->call->fn) != vars.end());
                        inst->call->dist->lifeBegin = inst->call->dist->lifeEnd = lifeTime;
                        inst->call->fn->lifeEnd = lifeTime;
                        for (auto& arg : inst->call->args) {
                            assert(vars.find(arg) != vars.end());
                            arg->lifeEnd = lifeTime;
                        }
                        vars.insert(inst->call->dist);
                        break;
                    case InstructionType::Return:
                        if (inst->ret->reg) {
                            assert(vars.find(inst->ret->reg) != vars.end());
                            inst->ret->reg->lifeEnd = lifeTime;
                        }
                        break;
                    default:
                        assert(false);
                    }
                    ++lifeTime;
                }
            }
        }
    }
    size_t sizeOfType(const Type& type)
    {
        switch (type.type) {
        case Types::LangDefine:
            switch (type.lang) {
            case LangDefineTypes::I8: return 1;
            case LangDefineTypes::I16: return 2;
            case LangDefineTypes::I32: return 4;
            case LangDefineTypes::I64: return 8;
            case LangDefineTypes::U8: return 1;
            case LangDefineTypes::U16: return 2;
            case LangDefineTypes::U32: return 4;
            case LangDefineTypes::U64: return 8;
            case LangDefineTypes::Bool: return 1;
            case LangDefineTypes::Char: return 1;
            case LangDefineTypes::Void: return 0;
            default: assert(false);
            }
            break;
        case Types::UserDefine:
            assert(false);
            break;
        case Types::Function:
            assert(false);
            break;
        case Types::Array:
            return sizeOfType(*type.array.type) * type.array.size;
            break;
        }
    }
    Result<Register*, std::vector<std::string>> IR::literalGenerate(const syntax::Literal* literal, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table)
    {
        using namespace syntax;
        std::vector<std::string> errors;
        auto reg = new Register{};
        auto inst = new Instruction{};
        inst->type = InstructionType::Imm;
        inst->imm = new ImmInstruction{};
        inst->imm->dist = reg;
        switch (literal->type) {
        case LiteralType::Integer:
            reg->symbol.type.type = Types::LangDefine;
            reg->symbol.type.lang = LangDefineTypes::I32;
            inst->imm->type = ImmType::I32;
            inst->imm->i32 = literal->integer;
            break;
        case LiteralType::I8:
            reg->symbol.type.type = Types::LangDefine;
            reg->symbol.type.lang = LangDefineTypes::I8;
            inst->imm->type = ImmType::I8;
            inst->imm->i8 = literal->i8;
            break;
        case LiteralType::I16:
            reg->symbol.type.type = Types::LangDefine;
            reg->symbol.type.lang = LangDefineTypes::I16;
            inst->imm->type = ImmType::I16;
            inst->imm->i16 = literal->i16;
            break;
        case LiteralType::I32:
            reg->symbol.type.type = Types::LangDefine;
            reg->symbol.type.lang = LangDefineTypes::I32;
            inst->imm->type = ImmType::I32;
            inst->imm->i32 = literal->i32;
            break;
        case LiteralType::I64:
            reg->symbol.type.type = Types::LangDefine;
            reg->symbol.type.lang = LangDefineTypes::I64;
            inst->imm->type = ImmType::I64;
            inst->imm->i64 = literal->u64;
            break;
        case LiteralType::U8:
            reg->symbol.type.type = Types::LangDefine;
            reg->symbol.type.lang = LangDefineTypes::U8;
            inst->imm->type = ImmType::U8;
            inst->imm->u8 = literal->u8;
            break;
        case LiteralType::U16:
            reg->symbol.type.type = Types::LangDefine;
            reg->symbol.type.lang = LangDefineTypes::U16;
            inst->imm->type = ImmType::U16;
            inst->imm->u16 = literal->u16;
            break;
        case LiteralType::U32:
            reg->symbol.type.type = Types::LangDefine;
            reg->symbol.type.lang = LangDefineTypes::U32;
            inst->imm->type = ImmType::U32;
            inst->imm->u32 = literal->u32;
            break;
        case LiteralType::U64:
            reg->symbol.type.type = Types::LangDefine;
            reg->symbol.type.lang = LangDefineTypes::U64;
            inst->imm->type = ImmType::U64;
            inst->imm->u64 = literal->u64;
            break;
        case LiteralType::Float:
            reg->symbol.type.type = Types::LangDefine;
            reg->symbol.type.lang = LangDefineTypes::F64;
            inst->imm->type = ImmType::F64;
            inst->imm->f64 = literal->f;
            break;
        case LiteralType::F32:
            reg->symbol.type.type = Types::LangDefine;
            reg->symbol.type.lang = LangDefineTypes::F32;
            inst->imm->type = ImmType::F32;
            inst->imm->f32 = literal->f32;
            break;
        case LiteralType::F64:
            reg->symbol.type.type = Types::LangDefine;
            reg->symbol.type.lang = LangDefineTypes::F64;
            inst->imm->type = ImmType::F64;
            inst->imm->f64 = literal->f64;
            break;
        case LiteralType::True:
            reg->symbol.type.type = Types::LangDefine;
            reg->symbol.type.lang = LangDefineTypes::Bool;
            inst->imm->type = ImmType::Bool;
            inst->imm->b = true;
            break;
        case LiteralType::False:
            reg->symbol.type.type = Types::LangDefine;
            reg->symbol.type.lang = LangDefineTypes::Bool;
            inst->imm->type = ImmType::Bool;
            inst->imm->b = false;
            break;
        case LiteralType::Char:
            reg->symbol.type.type = Types::LangDefine;
            reg->symbol.type.lang = LangDefineTypes::Char;
            inst->imm->type = ImmType::Char;
            inst->imm->c = literal->c;
            break;
        case LiteralType::Array:
            delete inst->imm;
            reg->symbol.type.type = Types::Array;
            reg->symbol.type.array.type = new Type{};
            reg->symbol.type.array.type->type = Types::Auto;
            reg->symbol.type.array.size = literal->array->elems.size();
            inst->type = InstructionType::Alloc;
            inst->alloc = new AllocInstruction{};
            inst->alloc->dist = reg;
            inst->alloc->size = literal->array->elems.size();
            break;
        case LiteralType::String:
            assert(literal->string->back() == '\0');
            delete inst->imm;
            reg->symbol.type.type = Types::Array;
            reg->symbol.type.array.type = new Type{};
            reg->symbol.type.array.type->type = Types::LangDefine;
            reg->symbol.type.array.type->lang = LangDefineTypes::Char;
            reg->symbol.type.array.size = literal->string->size();
            inst->type = InstructionType::Alloc;
            inst->alloc = new AllocInstruction{};
            inst->alloc->dist = reg;
            inst->alloc->size = literal->string->size();
            break;
        default:
            // TODO: 残りの型の実装
            assert(false);
        }

        regs += reg;
        blocks.back()->insts += inst;

        if (literal->type == LiteralType::Array) {
            size_t index = 0;
            size_t sizeOfElem = 0;
            std::optional<Type> type;
            for (const auto& elem : literal->array->elems) {
                auto node = nodeGenerate(elem, blocks, regs, vars, table);
                if (!node) errors += node.err();
                if (type && type.value() != node.get()->symbol.type) {
                    errors += "エラー: 配列の要素の型が一致しません。";
                }
                else {
                    type = node.get()->symbol.type;
                    reg->symbol.type.array.type = new Type(type.value());
                    sizeOfElem = sizeOfType(type.value());
                }
                auto store = new Instruction{};
                store->type = InstructionType::Store;
                store->store = new StoreInstruction{};
                store->store->base = inst->alloc->dist;
                store->store->index = new Register{};
                store->store->index->symbol.type.type = Types::LangDefine;
                store->store->index->symbol.type.lang = LangDefineTypes::U64;
                auto imm = new Instruction{};
                imm->type = InstructionType::Imm;
                imm->imm = new ImmInstruction{};
                imm->imm->type = ImmType::U64;
                imm->imm->u64 = index;
                imm->imm->dist = store->store->index;
                blocks.back()->insts += imm;
                regs += store->store->index;
                store->store->value = node.get();
                blocks.back()->insts += store;
                ++index;
            }
            inst->alloc->size *= sizeOfElem;
        }
        else if (literal->type == LiteralType::String) {
            size_t index = 0;
            for (auto c : *literal->string) {
                auto val = new Register{};
                val->symbol.type.type = Types::LangDefine;
                val->symbol.type.lang = LangDefineTypes::Char;
                regs += val;
                auto valCpy = new Instruction{};
                valCpy->type = InstructionType::Imm;
                valCpy->imm = new ImmInstruction{};
                valCpy->imm->type = ImmType::Char;
                valCpy->imm->c = c;
                valCpy->imm->dist = val;
                blocks.back()->insts += valCpy;
                auto store = new Instruction{};
                store->type = InstructionType::Store;
                store->store = new StoreInstruction{};
                store->store->base = inst->alloc->dist;
                store->store->index = new Register{};
                store->store->index->symbol.type.type = Types::LangDefine;
                store->store->index->symbol.type.lang = LangDefineTypes::U64;
                auto imm = new Instruction{};
                imm->type = InstructionType::Imm;
                imm->imm = new ImmInstruction{};
                imm->imm->type = ImmType::U64;
                imm->imm->u64 = index;
                imm->imm->dist = store->store->index;
                blocks.back()->insts += imm;
                regs += store->store->index;
                store->store->value = val;
                blocks.back()->insts += store;
                ++index;
            }
        }
        if (errors.empty()) return ok(reg);
        else return error(errors);
    }
    Result<Register*, std::vector<std::string>> IR::variableGenerate(const syntax::Variable* var, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table)
    {
        auto reg = new Register{};
        std::vector<std::string> errors;

        // グローバル変数
        if (!exists(vars, var->name)) {
            auto symbol = findSymbol(table, var, ir);
            if (!symbol) {
                std::stringstream ss;
                ss << "エラー: 変数 " << var->name;
                auto cur = var->scope;
                while (cur->scope) {
                    ss << "::" << cur->name;
                    cur = cur->scope;
                }
                ss << " が見つかりません。";
                errors += ss.str();
            }
            else {
                reg->symbol.type = symbol.value().first.type;
                auto inst = new Instruction{};
                inst->type = InstructionType::Load;
                inst->load = new LoadInstruction{};
                inst->load->type = LoadType::GlobalVariable;
                inst->load->dist = reg;
                inst->load->globalVar = symbol.value().second;
                blocks.back()->insts += inst;
            }
        }
        else if (vars[var->name] == nullptr) {
            errors += "エラー: 変数 " + var->name + " は初期化されていません。";
        }
        else {
            reg = vars[var->name];
        }
        if (reg) regs += reg;
        if (errors.empty()) return ok(reg);
        else return error(errors);
    }
    Result<Register*, std::vector<std::string>> IR::blockGenerate(const syntax::Block* block, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table)
    {
        Register* reg = nullptr;
        std::vector<std::string> errors;
        auto newBlock = new InstructionBlock{};
        auto inst = new Instruction{};
        inst->type = InstructionType::Jmp;
        inst->jmp = new JmpInstruction{};
        inst->jmp->block = newBlock;
        blocks.back()->insts += inst;
        blocks += newBlock;
        for (const auto& expr : block->exprs) {
            auto res = nodeGenerate(expr, blocks, regs, vars, table);
            if (res) reg = res.get();
            else errors += res.err();
        }
        if (errors.empty()) return ok(reg);
        else return error(errors);
    }
    Result<Register*, std::vector<std::string>> IR::ifElseGenerate(const syntax::IfElse* ifElse, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table)
    {
        using namespace syntax;
        std::vector<std::string> errors;
        Register* reg = nullptr;
        auto inst = new Instruction{};
        inst->type = InstructionType::Br;
        inst->br = new BrInstruction{};
        auto res = nodeGenerate(ifElse->cond, blocks, regs, vars, table);
        if (res) inst->br->cond = res.get();
        else errors += res.err();

        auto condBlock = blocks.back();
        auto phiBlock = new InstructionBlock{};
        inst->br->ifBlock = new InstructionBlock{};
        blocks += inst->br->ifBlock;
        auto ifReg = nodeGenerate(ifElse->ifExpr, blocks, regs, vars, table);
        if (!ifReg) errors += ifReg.err();

        auto jmpToPhiBlock = new Instruction{};
        jmpToPhiBlock->type = InstructionType::Jmp;
        jmpToPhiBlock->jmp = new JmpInstruction{};
        jmpToPhiBlock->jmp->block = phiBlock;

        auto ifEndBlock = blocks.back();
        if (ifEndBlock->insts.empty() || (ifEndBlock->insts.back()->type != InstructionType::Jmp && ifEndBlock->insts.back()->type != InstructionType::Br)) {
            ifEndBlock->insts += jmpToPhiBlock;
        }
        if (ifElse->elseExpr) {
            auto ifVars = vars;

            inst->br->elseBlock = new InstructionBlock{};
            blocks += inst->br->elseBlock;
            auto elseReg = nodeGenerate(ifElse->elseExpr, blocks, regs, vars, table);
            if (!elseReg) errors += elseReg.err();
            auto elseEndBlock = blocks.back();
            if (elseEndBlock->insts.empty() || (elseEndBlock->insts.back()->type != InstructionType::Jmp && elseEndBlock->insts.back()->type != InstructionType::Br)) {
                elseEndBlock->insts += jmpToPhiBlock;
            }
            if (ifReg && elseReg) {
                for (const auto& var : ifVars) {
                    assert(exists(vars, var.first));
                    if (vars[var.first] != var.second) {
                        auto phi = new Instruction{};
                        phi->type = InstructionType::Phi;
                        phi->phi = new PhiInstruction{};
                        phi->phi->dist = new Register{};
                        phi->phi->nodes += PhiNode{ var.second, ifEndBlock };
                        phi->phi->nodes += PhiNode{ vars[var.first], elseEndBlock };
                        phi->phi->dist->symbol.type = var.second->symbol.type;
                        phiBlock->insts += phi;
                        regs += phi->phi->dist;
                    }
                }

                auto phi = new Instruction{};
                phi->type = InstructionType::Phi;
                phi->phi = new PhiInstruction{};
                phi->phi->dist = new Register{};
                phi->phi->nodes += PhiNode{ ifReg.get(), ifEndBlock };
                phi->phi->nodes += PhiNode{ elseReg.get(), elseEndBlock };
                if (ifReg.get()->symbol.type != elseReg.get()->symbol.type) {
                    errors += "エラー: if式とelse式の返す値の型が一致しません。";
                }
                phi->phi->dist->symbol.type = ifReg.get()->symbol.type;
                phiBlock->insts += phi;
                condBlock->insts += inst;
                blocks += phiBlock;
                regs += phi->phi->dist;
                reg = phi->phi->dist;
            }
        }
        else {
            inst->br->elseBlock = phiBlock;
            condBlock->insts += inst;
            blocks += phiBlock;
            reg = new Register{};
            reg->symbol.type.type = Types::LangDefine;
            reg->symbol.type.lang = LangDefineTypes::Void;
        }

        if (reg) regs += reg;
        if (errors.empty()) return ok(reg);
        else return error(errors);
    }
    Result<Register*, std::vector<std::string>> IR::whileGenerate(const syntax::While* whi, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table)
    {
        using namespace syntax;
        std::vector<std::string> errors;

        auto entryBlock = blocks.back();

        auto condBlock = new InstructionBlock{};
        blocks += condBlock;

        auto jmpToFirst = new Instruction{};
        jmpToFirst->type = InstructionType::Jmp;
        jmpToFirst->jmp = new JmpInstruction{};
        jmpToFirst->jmp->block = condBlock;

        auto firstVars = vars;

        auto inst = new Instruction{};
        inst->type = InstructionType::Br;
        inst->br = new BrInstruction{};
        auto res = nodeGenerate(whi->cond, blocks, regs, vars, table);
        if (res) inst->br->cond = res.get();
        else errors += res.err();
        blocks.back()->insts += inst;

        inst->br->ifBlock = new InstructionBlock{};
        blocks += inst->br->ifBlock;
        auto reg = nodeGenerate(whi->expr, blocks, regs, vars, table);
        if (!reg) errors += reg.err();
        auto endBlock = blocks.back();

        inst->br->elseBlock = new InstructionBlock{};
        blocks += inst->br->elseBlock;

        entryBlock->insts += jmpToFirst;
        endBlock->insts += jmpToFirst;

        for (const auto& var : firstVars) {
            assert(exists(vars, var.first));
            if (vars[var.first] != var.second) {
                auto phi = new Instruction{};
                phi->type = InstructionType::Phi;
                phi->phi = new PhiInstruction{};
                phi->phi->dist = new Register{};
                phi->phi->nodes += PhiNode{ var.second, entryBlock };
                phi->phi->nodes += PhiNode{ vars[var.first], inst->br->ifBlock };
                phi->phi->dist->symbol.type = var.second->symbol.type;
                condBlock->insts += phi;
                regs += phi->phi->dist;
            }
        }

        if (errors.empty()) return ok(reg.get());
        else return error(errors);
    }
    Result<Register*, std::vector<std::string>> IR::primaryGenerate(const syntax::Primary* primary, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table)
    {
        using namespace syntax;
        switch (primary->type) {
        case PrimaryType::Literal:
            return literalGenerate(primary->literal, blocks, regs, vars, table);
        case PrimaryType::Variable:
            return variableGenerate(primary->var, blocks, regs, vars, table);
        case PrimaryType::Expression:
            return nodeGenerate(primary->expr, blocks, regs, vars, table);
        case PrimaryType::Block:
            return blockGenerate(primary->block, blocks, regs, vars, table);
        case PrimaryType::IfElse:
            return ifElseGenerate(primary->ifElse, blocks, regs, vars, table);
        case PrimaryType::While:
            return whileGenerate(primary->whi, blocks, regs, vars, table);
        default:
            assert(false);
        }
    }
    Result<Register*, std::vector<std::string>> IR::backUnaryGenerate(const syntax::BackUnary* back, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table)
    {
        using namespace syntax;
        auto expr = nodeGenerate(back->expr, blocks, regs, vars, table);
        if (!expr) return error(expr.err());
        std::vector<std::string> errors;
        auto reg = new Register{};
        auto inst = new Instruction{};
        switch (back->op) {
        case BackUnaryOperator::Call:
        {
            inst->type = InstructionType::Call;
            inst->call = new CallInstruction{};
            inst->call->fn = expr.get();
            for (const auto& arg : back->args->args) {
                auto res = nodeGenerate(arg, blocks, regs, vars, table);
                if (res) inst->call->args += res.get();
                else errors += res.err();
            }
            reg->symbol.type = *inst->call->fn->symbol.type.fn->retType;
            inst->call->dist = reg;
            break;
        }
        case BackUnaryOperator::Array:
        {
            inst->type = InstructionType::Load;
            inst->load = new LoadInstruction{};
            inst->load->type = LoadType::Array;
            inst->load->array.base = expr.get();
            auto suffix = nodeGenerate(back->suffix, blocks, regs, vars, table);
            if (suffix) inst->load->array.suffix = suffix.get();
            else errors += suffix.err();
            reg->symbol.type = *inst->load->array.base->symbol.type.array.type;
            inst->load->dist = reg;
            break;
        }
        default:
            assert(false);
        }
        blocks.back()->insts += inst;
        regs += reg;
        if (errors.empty()) return ok(reg);
        else return error(errors);
    }
    Result<Register*, std::vector<std::string>> IR::frontUnaryGenerate(const syntax::FrontUnary* front, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table)
    {
        using namespace syntax;
        auto expr = nodeGenerate(front->expr, blocks, regs, vars, table);
        if (!expr) return error(expr.err());

        Register* reg = nullptr;
        std::vector<std::string> errors;
        switch (front->op) {
        case FrontUnaryOperator::Plus:
            reg = expr.get();
            break;
        case FrontUnaryOperator::Minus:
        {
            reg = new Register{};
            auto inst = new Instruction{};
            inst->type = InstructionType::Neg;
            inst->neg = new NegInstruction{};
            inst->neg->dist = reg;
            inst->neg->src = expr.get();
            reg->symbol.type = expr.get()->symbol.type;
            blocks.back()->insts += inst;
            regs += reg;
            break;
        }
        default:
            assert(false);
        }

        if (errors.empty()) return ok(reg);
        else return error(errors);
    }
    Result<Register*, std::vector<std::string>> IR::factorGenerate(const syntax::Factor* factor, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table)
    {
        using namespace syntax;
        auto reg = new Register{};
        std::vector<std::string> errors;
        auto left = nodeGenerate(factor->left, blocks, regs, vars, table);
        auto right = nodeGenerate(factor->right, blocks, regs, vars, table);
        if (!left) errors += left.err();
        if (!right) errors += right.err();
        if (left && right && left.get()->symbol.type != right.get()->symbol.type) {
            errors += "エラー: 左右の型が一致しません。";
        }

        if (left && right) {
            reg->symbol.type = left.get()->symbol.type;
            auto inst = new Instruction{};
            inst->binary = new BinaryOpInstruction{};
            inst->binary->dist = reg;
            inst->binary->left = left.get();
            inst->binary->right = right.get();
            switch (factor->op) {
            case FactorOperator::Mul:
                inst->type = InstructionType::Mul;
                break;
            case FactorOperator::Div:
                inst->type = InstructionType::Div;
                break;
            case FactorOperator::Mod:
                inst->type = InstructionType::Mod;
                break;
            default:
                assert(false);
            }
            blocks.back()->insts += inst;
        }
        
        regs += reg;

        if (errors.empty()) return ok(reg);
        else return error(errors);
    }
    Result<Register*, std::vector<std::string>> IR::termGenerate(const syntax::Term* term, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table)
    {
        using namespace syntax;
        auto reg = new Register{};
        std::vector<std::string> errors;
        auto left = nodeGenerate(term->left, blocks, regs, vars, table);
        auto right = nodeGenerate(term->right, blocks, regs, vars, table);
        if (!left) errors += left.err();
        if (!right) errors += right.err();
        if (left && right && left.get()->symbol.type != right.get()->symbol.type) {
            errors += "エラー: 左右の型が一致しません。";
        }

        if (left && right) {
            reg->symbol.type = left.get()->symbol.type;
            auto inst = new Instruction{};
            inst->binary = new BinaryOpInstruction{};
            inst->binary->dist = reg;
            inst->binary->left = left.get();
            inst->binary->right = right.get();
            switch (term->op) {
            case TermOperator::Add:
                inst->type = InstructionType::Add;
                break;
            case TermOperator::Sub:
                inst->type = InstructionType::Sub;
                break;
            default:
                assert(false);
            }
            blocks.back()->insts += inst;
        }
        
        regs += reg;

        if (errors.empty()) return ok(reg);
        else return error(errors);
    }
    Result<Register*, std::vector<std::string>> IR::compGenerate(const syntax::Comparison* comp, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table)
    {
        using namespace syntax;
        auto reg = new Register{};
        std::vector<std::string> errors;
        reg->symbol.type.type = Types::LangDefine;
        reg->symbol.type.lang = LangDefineTypes::Bool;
        auto left = nodeGenerate(comp->left, blocks, regs, vars, table);
        auto right = nodeGenerate(comp->right, blocks, regs, vars, table);
        if (!left) errors += left.err();
        if (!right) errors += right.err();
        if (left && right && left.get()->symbol.type != right.get()->symbol.type) {
            errors += "エラー: 左右の型が一致しません。";
        }

        if (left && right) {
            auto inst = new Instruction{};
            inst->binary = new BinaryOpInstruction{};
            inst->binary->dist = reg;
            inst->binary->left = left.get();
            inst->binary->right = right.get();
            switch (comp->op) {
            case ComparisonOperator::Equal:
                inst->type = InstructionType::Equal;
                break;
            case ComparisonOperator::NotEqual:
                inst->type = InstructionType::NotEqual;
                break;
            case ComparisonOperator::GreaterEqual:
                inst->type = InstructionType::GreaterEqual;
                break;
            case ComparisonOperator::GreaterThan:
                inst->type = InstructionType::GreaterThan;
                break;
            case ComparisonOperator::LessEqual:
                inst->type = InstructionType::LessEqual;
                break;
            case ComparisonOperator::LessThan:
                inst->type = InstructionType::LessThan;
                break;
            default:
                assert(false);
            }
            blocks.back()->insts += inst;
        }
        
        regs += reg;

        if (errors.empty()) return ok(reg);
        else return error(errors);
    }
    Result<Register*, std::vector<std::string>> IR::asignGenerate(const syntax::Asign* asign, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table)
    {
        using namespace syntax;
        Register* reg = nullptr;
        std::vector<std::string> errors;
        auto left = nodeGenerate(asign->left, blocks, regs, vars, table);
        auto right = nodeGenerate(asign->right, blocks, regs, vars, table);
        if (!left) errors += left.err();
        if (!right) errors += right.err();
        if (left && right && left.get()->symbol.type != right.get()->symbol.type) {
            errors += "エラー: 左右の型が一致しません。";
        }
        
        if (left && right) {
            reg = left.get();
            for (auto& var : vars) {
                if (var.second == reg) {
                    reg = new Register{};
                    reg->symbol = var.second->symbol;
                    vars[var.first] = reg;
                    regs += reg;
                }
            }
            auto inst = new Instruction{};
            inst->type = InstructionType::Store;
            inst->store = new StoreInstruction{};
            inst->store->base = reg;
            inst->store->index = nullptr;
            inst->store->value = right.get();
            switch (asign->op) {
            case AsignOperator::Asign:
                break;
            default:
                assert(false);
            }
            blocks.back()->insts += inst;
        }

        if(reg) regs += reg;
        
        if (errors.empty()) return ok(reg);
        else return error(errors);
    }
    Result<Register*, std::vector<std::string>> IR::returnGenerate(const syntax::Return* ret, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table)
    {
        auto res = nodeGenerate(ret->val, blocks, regs, vars, table);
        if (res) {
            auto inst = new Instruction{};
            inst->type = InstructionType::Return;
            inst->ret = new ReturnInstruction{};
            inst->ret->reg = res.get();
            blocks.back()->insts += inst;
            return ok(inst->ret->reg);
        }
        else return error(res.err());
    }
    Result<Register*, std::vector<std::string>> IR::varDefGenerate(const syntax::VariableDefine* varDef, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table)
    {
        Register* reg = nullptr;
        std::vector<std::string> errors;
        if (varDef->var->scope != nullptr) {
            errors += "エラー: 変数名にスコープ演算子は使用できません。";
        }
        else if (exists(vars, varDef->var->name)) {
            errors += "エラー: 変数 " + varDef->var->name + " は既に定義されています。";
        }
        if (varDef->init) {
            auto res = nodeGenerate(varDef->init, blocks, regs, vars, table);
            if (res) reg = res.get();
            else errors += res.err();
        }
        vars[varDef->var->name] = reg;

        if (errors.empty()) return ok(reg);
        else return error(errors);
    }
    Result<Register*, std::vector<std::string>> IR::nodeGenerate(const syntax::Node* node, std::vector<InstructionBlock*>& blocks, std::vector<Register*>& regs, std::unordered_map<std::string, Register*>& vars, SymbolTable* table)
    {
        using namespace syntax;
        switch (node->kind) {
        case NodeKind::BackUnary:
            return backUnaryGenerate(node->back, blocks, regs, vars, table);
        case NodeKind::FrontUnary:
            return frontUnaryGenerate(node->front, blocks, regs, vars, table);
        case NodeKind::Factor:
            return factorGenerate(node->factor, blocks, regs, vars, table);
        case NodeKind::Term:
            return termGenerate(node->term, blocks, regs, vars, table);
        case NodeKind::Comparison:
            return compGenerate(node->comp, blocks, regs, vars, table);
        case NodeKind::Asign:
            return asignGenerate(node->asign, blocks, regs, vars, table);
        case NodeKind::Primary:
            return primaryGenerate(node->primary, blocks, regs, vars, table);
        case NodeKind::Literal:
            return literalGenerate(node->literal, blocks, regs, vars, table);
        case NodeKind::Variable:
            return variableGenerate(node->var, blocks, regs, vars, table);
        case NodeKind::Type:
            assert(false);
            break;
        case NodeKind::Return:
            return returnGenerate(node->ret, blocks, regs, vars, table);
        case NodeKind::Block:
            return blockGenerate(node->block, blocks, regs, vars, table);
        case NodeKind::VariableDefine:
            return varDefGenerate(node->varDef, blocks, regs, vars, table);
        case NodeKind::ArgumentDefine:
            assert(false);
            break;
        case NodeKind::FunctionDefine:
            assert(false);
            break;
        case NodeKind::ClassDefine:
            assert(false);
            break;
        case NodeKind::AliasDefine:
            assert(false);
            break;
        default:
            assert(false);
        }
    }
    Result<Function*, std::vector<std::string>> IR::functionGenerate(const syntax::FunctionDefine* fnDef, SymbolTable* symbolTable)
    {
        using namespace syntax;
        auto fn = new Function{};
        fn->symbolTable = symbolTable;
        fn->blocks += new InstructionBlock{};
        for (const auto& arg : fnDef->args) {
            assert(arg->var->scope == nullptr);
            auto reg = new Register{};
            reg->symbol.type = syntaxTypeToIrType(arg->type);
            fn->args += reg;
            fn->blocks.back()->insts;
            fn->regs += reg;
            fn->vars[arg->var->name] = reg;
        }
        std::vector<std::string> errors;
        auto reg = nodeGenerate(fnDef->expr, fn->blocks, fn->regs, fn->vars, fn->symbolTable);
        if (!reg) errors += reg.err();
        else if (fn->blocks.back()->insts.empty() || fn->blocks.back()->insts.back()->type != InstructionType::Return) {
            auto inst = new Instruction{};
            inst->type = InstructionType::Return;
            inst->ret = new ReturnInstruction{};
            inst->ret->reg = reg.get();
            fn->blocks.back()->insts += inst;
        }
        ir.functions += fn;
        calcLifeTime(fn);
        if (errors.empty()) return ok(fn);
        else return error(errors);
    }
    Result<Function*, std::vector<std::string>> IR::globalVarDefGenerate(Symbol& symbol, SymbolTable* symbolTable)
    {
        std::vector<std::string> errors;
        symbol.init = new Function{};
        symbol.init->symbolTable = symbolTable;
        symbol.init->blocks += new InstructionBlock{};
        auto res = nodeGenerate(symbol.syntax->varDef->init, symbol.init->blocks, symbol.init->regs, symbol.init->vars, symbol.init->symbolTable);
        if (res) {
            auto global = findSymbol(symbolTable, symbol.syntax->varDef->var, ir);
            if (!global) {
                std::stringstream ss;
                ss << "エラー: 変数 " << symbol.syntax->varDef->var->name;
                auto cur = symbol.syntax->varDef->var->scope;
                while (cur->scope) {
                    ss << "::" << cur->name;
                    cur = cur->scope;
                }
                ss << " が見つかりません。";
                errors += ss.str();
            }
            else {
                auto inst = new Instruction{};
                inst->type = InstructionType::Store;
                inst->store = new StoreInstruction{};
                inst->store->value = res.get();
                inst->store->globalVar = global.value().second;
                symbol.init->blocks.back()->insts += inst;
                symbol.type = res.get()->symbol.type;
            }
        }
        else errors += res.err();
        calcLifeTime(symbol.init);
        if (errors.empty()) return ok(symbol.init);
        else return error(errors);
    }
    Result<_, std::vector<std::string>> IR::generate(SymbolTable* table)
    {
        using namespace syntax;
        std::vector<std::string> errors;
        for(auto& symbol : table->symbols) {
            switch (symbol.second.type.type) {
            case Types::Undefine:
            {
                assert(symbol.second.syntax->kind == NodeKind::VariableDefine);
                auto res = globalVarDefGenerate(symbol.second, table);
                if (!res) errors += res.err();
                break;
            }
            case Types::Function:
                if (symbol.second.syntax->kind == NodeKind::FunctionDefine) {
                    if (symbol.second.syntax->fnDef->name == nullptr) {
                        errors += "エラー: 名前を付ける必要があります。";
                    }
                    else {
                        auto res = functionGenerate(symbol.second.syntax->fnDef, table);
                        if (!res) errors += res.err();
                        else symbol.second.function = res.get();
                    }
                }
                else {
                    assert(symbol.second.syntax->kind == NodeKind::ExternDeclare);
                    auto fn = new Function{};
                    fn->symbolTable = table;
                    fn->externName = symbol.first;
                    symbol.second.function = fn;
                    ir.functions += fn;
                }
                break;
            case Types::SymbolTable:
            {
                assert(symbol.second.syntax->kind == NodeKind::ImportModule);
                auto findTable = [](SymbolTable* current, syntax::Variable* curName) -> SymbolTable* {
                    while (true) {
                        if (exists(current->subSymbolTable, curName->name)) {
                            current = current->subSymbolTable[curName->name];
                            if (curName->scope != nullptr) {
                                curName = curName->scope;
                            }
                            else {
                                return current;
                            }
                        }
                        else {
                            return nullptr;
                        }
                    }
                };
                symbol.second.table = findTable(table->parentSymbolTable, symbol.second.syntax->impMod->name);
                if (symbol.second.table == nullptr) {
                    symbol.second.table = findTable(ir.rootSymbolTable, symbol.second.syntax->impMod->name);
                }
                if (symbol.second.table == nullptr) {
                    auto cur = symbol.second.syntax->impMod->name;
                    std::stringstream ss;
                    ss << "エラー: モジュール ";
                    ss << cur->name;
                    while (cur = cur->scope) {
                        ss << "::" << cur->name;
                    }
                    ss << " が見つかりませんでした。";
                    errors += ss.str();
                }
                break;
            }
            default:
                assert(false);
            }
        }
        for (auto& sub : table->subSymbolTable) {
            auto res = generate(sub.second);
            if (!res) errors += res.err();
        }
        if (errors.empty()) return ok();
        else return error(errors);
    }
    IR::IR(const ModuleTree& tree) : tree(tree) {}
    Result<SymbolTable*, std::vector<std::string>> globalGenerate(const syntax::SyntaxTree& tree)
    {
        using namespace syntax;
        std::vector<std::string> errors;
        auto table = new SymbolTable{};
        for (const auto& expr : tree.exprs) {
            switch (expr->kind) {
            case NodeKind::ExternDeclare:
                if (expr->extDec->name->scope != nullptr) {
                    errors += "エラー: externする関数にはスコープ演算子を使用できません。";
                    break;
                }
                if (exists(table->symbols, expr->extDec->name->name)) {
                    errors += "エラー: シンボル " + expr->extDec->name->name + " は既に定義されています。";
                    break;
                }
                table->symbols[expr->extDec->name->name].syntax = expr;
                table->symbols[expr->extDec->name->name].type.type = Types::Function;
                table->symbols[expr->extDec->name->name].type.fn = new FunctionType{};
                for (const auto& arg : expr->extDec->args) {
                    table->symbols[expr->extDec->name->name].type.fn->argTypes += syntaxTypeToIrType(arg->type);
                }
                table->symbols[expr->extDec->name->name].type.fn->retType = new Type(syntaxTypeToIrType(expr->extDec->retType));
                break;
            case NodeKind::ImportModule:
            {
                auto name = expr->impMod->name;
                while (name->scope != nullptr) {
                    name = name->scope;
                }
                table->symbols[name->name].syntax = expr;
                table->symbols[name->name].type.type = Types::SymbolTable;
                break;
            }
            case NodeKind::VariableDefine:
                assert(expr->varDef->var->scope == nullptr);
                if (exists(table->symbols, expr->varDef->var->name)) {
                    errors += "エラー: シンボル " + expr->varDef->var->name + " は既に定義されています。";
                    break;
                }
                table->symbols[expr->varDef->var->name].syntax = expr;
                table->symbols[expr->varDef->var->name].type.type = Types::Undefine;
                break;
            case NodeKind::FunctionDefine:
                assert(expr->fnDef->name->scope == nullptr);
                if (exists(table->symbols, expr->fnDef->name->name)) {
                    errors += "エラー: シンボル " + expr->fnDef->name->name + " は既に定義されています。";
                    break;
                }
                table->symbols[expr->fnDef->name->name].syntax = expr;
                table->symbols[expr->fnDef->name->name].type.type = Types::Function;
                table->symbols[expr->fnDef->name->name].type.fn = new FunctionType{};
                for (const auto& arg : expr->fnDef->args) {
                    table->symbols[expr->fnDef->name->name].type.fn->argTypes += syntaxTypeToIrType(arg->type);
                }
                table->symbols[expr->fnDef->name->name].type.fn->retType = new Type(syntaxTypeToIrType(expr->fnDef->retType));
                break;
            case NodeKind::ClassDefine:
                // TODO: 実装
                assert(false);
                break;
            case NodeKind::AliasDefine:
                // TODO: 実装
                assert(false);
                break;
            case NodeKind::Asign:
                errors += "構文エラー: グローバルな領域では型の定義かシンボルの定義のみ行えます。";
                break;
            default:
                assert(false);
            }
        }
        if (errors.empty()) return ok(table);
        else return error(errors);
    }
    Result<SymbolTable*, std::vector<std::string>> globalGenerate(const ModuleNode& node)
    {
        std::vector<std::string> errors;
        auto table = new SymbolTable{};
        table->name = node.name;
        for (const auto& mod : node.modules) {
            assert(mod.first == mod.second.name);
            auto res = globalGenerate(mod.second.syntaxTree);
            if (!res) errors += res.err();
            else {
                if (mod.first == "index") table->symbols = res.get()->symbols;
                else {
                    res.get()->name = mod.first;
                    table->subSymbolTable[mod.first] = res.get();
                    table->subSymbolTable[mod.first]->parentSymbolTable = table;
                }
            }
        }
        for (const auto& child : node.children) {
            auto res = globalGenerate(child.second);
            if (res) {
                table->subSymbolTable[child.first] = res.get();
                table->subSymbolTable[child.first]->parentSymbolTable = table;
            }
            else errors += res.err();
        }
        if (errors.empty()) return ok(table);
        else return error(errors);
    }
    Result<IntermediateRepresentation, std::vector<std::string>> IR::generate()
    {
        std::vector<std::string> errors;
        ir.rootSymbolTable = new SymbolTable{};
        for (const auto& node : tree.nodes) {
            assert(node.first == node.second.name);
            auto res = globalGenerate(node.second);
            if (res) {
                ir.rootSymbolTable->subSymbolTable[node.first] = res.get();
                ir.rootSymbolTable->subSymbolTable[node.first]->parentSymbolTable = ir.rootSymbolTable;
            }
            else errors += res.err();
        }
        auto res = generate(ir.rootSymbolTable);
        if (!res) errors += res.err();
        if (errors.empty()) return ok(ir);
        else return error(errors);
    }
}