#include "x86_64.h"

#include <optional>

namespace pick::x86_64
{
    namespace
    {
        template<typename K, typename V>
        inline bool exists(const std::unordered_map<K, V>& map, const K& key)
        {
            return map.find(key) != map.end();
        }
        template<typename T>
        inline size_t indexOf(const std::vector<T>& vec, const T& val)
        {
            auto itr = std::find(vec.begin(), vec.end(), val);
            assert(itr != vec.end());
            return std::distance(vec.begin(), itr);
        }
        using RR = void(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
        using RA = void(std::vector<uint8_t>& code, Register dist, Register base, int32_t ref, size_t size);
        using RG = void(std::vector<uint8_t>& code, Register dist, uint32_t address, size_t size);
        using AR = void(std::vector<uint8_t>& code, Register base, int32_t ref, Register src, size_t size);
        using GR = void(std::vector<uint8_t>& code, uint32_t address, Register src, size_t size);
        using RI = void(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size);
        using AI = void(std::vector<uint8_t>& code, Register base, int32_t ref, int32_t imm, size_t size);
        inline void xBinaryOp(Routine& routine, size_t entry, std::vector<uint8_t>& code, const Operation& op, int32_t len, RR rr, RA ra, RG rg, AR ar, GR gr, RI ri, AI ai) {
            switch (op.op1.type) {
            case RMType::Register:
                if (op.op2) {
                    switch (op.op2->type) {
                    case RMType::Register:
                        rr(code, op.op1.reg, op.op2->reg, op.size);
                        break;
                    case RMType::Memory:
                        ra(code, op.op1.reg, op.op2->mem.base, op.op2->mem.disp + len, op.size);
                        break;
                    case RMType::Constant:
                        rg(code, op.op1.reg, 0, op.size);
                        routine.relocs += Relocation{ entry + code.size() - 4, RelocationType::ConstantSymbol, op.op2->address };
                        break;
                    case RMType::Runtime:
                        rg(code, op.op1.reg, 0, op.size);
                        routine.relocs += Relocation{ entry + code.size() - 4, RelocationType::RuntimeSymbol, op.op2->address };
                        break;
                    default:
                        assert(false);
                    }
                }
                else if (op.imm) {
                    ri(code, op.op1.reg, op.imm.value(), op.size);
                }
                else {
                    assert(false);
                }
                break;
            case RMType::Memory:
                if (op.op2) {
                    switch (op.op2->type) {
                    case RMType::Register:
                        ar(code, op.op1.mem.base, op.op1.mem.disp + len, op.op2->reg, op.size);
                        break;
                    case RMType::Memory:
                        assert(false);
                        break;
                    default:
                        assert(false);
                    }
                }
                else if (op.imm) {
                    ai(code, op.op1.mem.base, op.op1.mem.disp + len, op.imm.value(), op.size);
                }
                else {
                    assert(false);
                }
                break;
            case RMType::Runtime:
                if (op.op2) {
                    switch (op.op2.value().type) {
                    case RMType::Register:
                        gr(code, 0, op.op2.value().reg, op.size);
                        routine.relocs += Relocation{ entry + code.size() - 4, RelocationType::RuntimeSymbol, op.op1.address };
                        break;
                    case RMType::Memory:
                        assert(false);
                        break;
                    default:
                        assert(false);
                    }
                }
                else {
                    assert(false);
                }
                break;
            default:
                assert(false);
            }
        }
    }
    Result<std::vector<std::vector<Operation>>, std::vector<std::string>> X86_64Compiler::commonCompileRoutine(const ir::Function* fn, std::unordered_map<x86_64::Register, RegState>& regs, int32_t& offset)
    {
        using namespace ir;

        std::vector<std::vector<Operation>> result;
        std::vector<std::string> errors;
        size_t lifeTime = 0;
        std::unordered_map<ir::Register*, Var*> vars;
        Cond cond = Cond::Undef;

        auto freeReg = [&]() {
            auto itr = vars.begin();
            while (itr != vars.end()) {
                if ((*itr).first->lifeEnd < lifeTime) {
                    if ((*itr).second->type == VarType::Register) {
                        regs[(*itr).second->reg].inUse = nullptr;
                    }
                    itr = vars.erase(itr);
                }
                else {
                    ++itr;
                }
            }
        };

        /*
            RAX, R10を一時的なレジスタとして使用する。
            Spill同士の計算などの時。
            RDXは剰余で使用する。
        */
        auto nextReg = [&](ir::Register* dist) {
            freeReg();
            for (auto reg : {
                Register::RCX,
                Register::RBX,
                Register::RSI,
                Register::RDI,
                Register::R8,
                Register::R9,
                Register::R11,
                Register::R12,
                Register::R13,
                Register::R14,
                Register::R15
                }) {
                if (!regs[reg].inUse) {
                    auto var = new Var{};
                    var->type = VarType::Register;
                    var->reg = reg;
                    regs[reg].inUse = var;
                    regs[reg].hasBeenUsed = true;
                    vars[dist] = var;
                    return var;
                }
            }
            auto var = new Var{};
            var->type = VarType::Spill;
            var->offset = offset;
            vars[dist] = var;
            offset -= 8;
            return var;
        };
        auto saveRegs = [&](std::vector<Operation>& ops) {
            freeReg();
            std::vector<std::pair<Register, Var*>> push;
            for (auto& var : vars) {
                if (var.second->type == VarType::Register) {
                    for (auto reg : {
                        Register::RCX,
                        Register::RDX,
                        Register::R8,
                        Register::R9,
                        Register::R10,
                        Register::R11
                        }) {
                        if (var.second->reg == reg) {
                            assert(regs[var.second->reg].inUse == var.second);
                            var.second->type = VarType::Spill;
                            var.second->offset = offset;
                            ops += Operation::ar(Opecode::MOV, Register::RBP, offset, reg, 8);
                            push += std::make_pair(reg, var.second);
                            offset -= 8;
                        }
                    }
                }
            }
            return push;
        };
        auto restoreReg = [&](std::vector<Operation>& ops, std::vector<std::pair<Register, Var*>>& pushed)
        {
            /*for (auto i = pushed.rbegin(), end = pushed.rend(); i != end; ++i) {
                ops += Operation::r(Opecode::POP, i->first);
                i->second->type = VarType::Register;
                i->second->reg = i->first;
                offset += 8;
            }*/
        };

        auto binaryOp = [&](std::vector<Operation>& ops, Opecode opecode, const ir::Instruction* inst) {
            assert(
                inst->type == ir::InstructionType::Add ||
                inst->type == ir::InstructionType::Sub ||
                inst->type == ir::InstructionType::Mul ||
                inst->type == ir::InstructionType::Div ||
                inst->type == ir::InstructionType::Mod ||
                inst->type == ir::InstructionType::Equal ||
                inst->type == ir::InstructionType::NotEqual ||
                inst->type == ir::InstructionType::GreaterEqual ||
                inst->type == ir::InstructionType::GreaterThan ||
                inst->type == ir::InstructionType::LessEqual ||
                inst->type == ir::InstructionType::LessThan
            );
            assert(exists(vars, inst->binary->left));
            assert(exists(vars, inst->binary->right));
            Var* next = nullptr;
            if (exists(vars, inst->imm->dist)) {
                next = vars[inst->imm->dist];
                assert(next->type == VarType::Phi);
            }
            else {
                next = nextReg(inst->imm->dist);
            }
            switch (vars[inst->binary->left]->type) {
            case VarType::Register:
                switch (vars[inst->binary->right]->type) {
                case VarType::Register:
                    switch (next->type) {
                    case VarType::Register:
                        ops += Operation::rr(Opecode::MOV, next->reg, vars[inst->binary->left]->reg, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::rr(opecode, next->reg, vars[inst->binary->right]->reg, ir::sizeOfType(inst->binary->right->symbol.type));
                        break;
                    case VarType::Spill:
                    case VarType::Phi:
                        ops += Operation::ar(Opecode::MOV, Register::RBP, next->offset, vars[inst->binary->left]->reg, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::ar(opecode, Register::RBP, next->offset, vars[inst->binary->right]->reg, ir::sizeOfType(inst->binary->right->symbol.type));
                        break;
                    case VarType::RuntimeSymbol:
                        ops += Operation::ar(Opecode::MOV, RMType::Runtime, next->indexOfData, vars[inst->binary->left]->reg, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::ar(opecode, RMType::Runtime, next->indexOfData, vars[inst->binary->right]->reg, ir::sizeOfType(inst->binary->right->symbol.type));
                        break;
                    default:
                        assert(false);
                    }
                    break;
                case VarType::Spill:
                case VarType::Phi:
                    switch (next->type) {
                    case VarType::Register:
                        ops += Operation::rr(Opecode::MOV, next->reg, vars[inst->binary->left]->reg, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::ra(opecode, next->reg, Register::RBP, vars[inst->binary->right]->offset, ir::sizeOfType(inst->binary->right->symbol.type));
                        break;
                    case VarType::Spill:
                    case VarType::Phi:
                        ops += Operation::rr(Opecode::MOV, Register::RAX, vars[inst->binary->left]->reg, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::ra(opecode, Register::RAX, Register::RBP, vars[inst->binary->right]->offset, ir::sizeOfType(inst->binary->right->symbol.type));
                        ops += Operation::ar(Opecode::MOV, Register::RBP, next->offset, Register::RAX, ir::sizeOfType(inst->binary->dist->symbol.type));
                        break;
                    case VarType::RuntimeSymbol:
                        ops += Operation::rr(Opecode::MOV, Register::RAX, vars[inst->binary->left]->reg, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::ra(opecode, Register::RAX, Register::RBP, vars[inst->binary->right]->offset, ir::sizeOfType(inst->binary->right->symbol.type));
                        ops += Operation::ar(Opecode::MOV, RMType::Runtime, next->indexOfData, Register::RAX, ir::sizeOfType(inst->binary->left->symbol.type));
                        break;
                    default:
                        assert(false);
                    }
                    break;
                case VarType::RuntimeSymbol:
                    switch (next->type) {
                    case VarType::Register:
                        ops += Operation::rr(Opecode::MOV, next->reg, vars[inst->binary->left]->reg, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::ra(opecode, next->reg, RMType::Runtime, vars[inst->binary->right]->indexOfData, ir::sizeOfType(inst->binary->right->symbol.type));
                        break;
                    case VarType::Spill:
                    case VarType::Phi:
                        ops += Operation::rr(Opecode::MOV, Register::RAX, vars[inst->binary->left]->reg, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::ra(opecode, Register::RAX, RMType::Runtime, vars[inst->binary->right]->indexOfData, ir::sizeOfType(inst->binary->right->symbol.type));
                        ops += Operation::ar(Opecode::MOV, Register::RBP, next->offset, Register::RAX, ir::sizeOfType(inst->binary->dist->symbol.type));
                        break;
                    case VarType::RuntimeSymbol:
                        ops += Operation::rr(Opecode::MOV, Register::RAX, vars[inst->binary->left]->reg, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::ra(opecode, Register::RAX, RMType::Runtime, vars[inst->binary->right]->indexOfData, ir::sizeOfType(inst->binary->right->symbol.type));
                        ops += Operation::ar(Opecode::MOV, RMType::Runtime, next->indexOfData, Register::RAX, ir::sizeOfType(inst->binary->left->symbol.type));
                        break;
                    default:
                        assert(false);
                    }
                    break;
                default:
                    assert(false);
                }
                break;
            case VarType::Spill:
            case VarType::Phi:
                switch (vars[inst->binary->right]->type) {
                case VarType::Register:
                    switch (next->type) {
                    case VarType::Register:
                        ops += Operation::ra(Opecode::MOV, next->reg, Register::RBP, vars[inst->binary->left]->offset, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::rr(opecode, next->reg, vars[inst->binary->right]->reg, ir::sizeOfType(inst->binary->right->symbol.type));
                        break;
                    case VarType::Spill:
                    case VarType::Phi:
                        ops += Operation::ra(Opecode::MOV, Register::RAX, Register::RBP, vars[inst->binary->left]->offset, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::rr(opecode, Register::RAX, vars[inst->binary->right]->reg, ir::sizeOfType(inst->binary->right->symbol.type));
                        ops += Operation::ar(Opecode::MOV, Register::RBP, next->offset, Register::RAX, ir::sizeOfType(inst->binary->dist->symbol.type));
                        break;
                    case VarType::RuntimeSymbol:
                        ops += Operation::ra(Opecode::MOV, Register::RAX, Register::RBP, vars[inst->binary->left]->offset, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::rr(opecode, Register::RAX, vars[inst->binary->right]->reg, ir::sizeOfType(inst->binary->right->symbol.type));
                        ops += Operation::ar(Opecode::MOV, RMType::Runtime, next->indexOfData, Register::RAX, ir::sizeOfType(inst->binary->left->symbol.type));
                        break;
                    default:
                        assert(false);
                    }
                    break;
                case VarType::Spill:
                case VarType::Phi:
                    switch (next->type) {
                    case VarType::Register:
                        ops += Operation::ra(Opecode::MOV, next->reg, Register::RBP, vars[inst->binary->left]->offset, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::ra(opecode, next->reg, Register::RBP, vars[inst->binary->right]->offset, ir::sizeOfType(inst->binary->right->symbol.type));
                        break;
                    case VarType::Spill:
                    case VarType::Phi:
                        ops += Operation::ra(Opecode::MOV, Register::RAX, Register::RBP, vars[inst->binary->left]->offset, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::ra(opecode, Register::RAX, Register::RBP, vars[inst->binary->right]->offset, ir::sizeOfType(inst->binary->right->symbol.type));
                        ops += Operation::ar(Opecode::MOV, Register::RBP, next->offset, Register::RAX, ir::sizeOfType(inst->binary->dist->symbol.type));
                        break;
                    case VarType::RuntimeSymbol:
                        ops += Operation::ra(Opecode::MOV, Register::RAX, Register::RBP, vars[inst->binary->left]->offset, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::ra(opecode, Register::RAX, Register::RBP, vars[inst->binary->right]->offset, ir::sizeOfType(inst->binary->right->symbol.type));
                        ops += Operation::ar(Opecode::MOV, RMType::Runtime, next->indexOfData, Register::RAX, ir::sizeOfType(inst->binary->left->symbol.type));
                        break;
                    default:
                        assert(false);
                    }
                    break;
                case VarType::RuntimeSymbol:
                    switch (next->type) {
                    case VarType::Register:
                        ops += Operation::ra(Opecode::MOV, next->reg, Register::RBP, vars[inst->binary->left]->offset, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::ra(opecode, next->reg, RMType::Runtime, vars[inst->binary->right]->indexOfData, ir::sizeOfType(inst->binary->right->symbol.type));
                        break;
                    case VarType::Spill:
                    case VarType::Phi:
                        ops += Operation::ra(Opecode::MOV, Register::RAX, Register::RBP, vars[inst->binary->left]->offset, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::ra(opecode, Register::RAX, RMType::Runtime, vars[inst->binary->right]->indexOfData, ir::sizeOfType(inst->binary->right->symbol.type));
                        ops += Operation::ar(Opecode::MOV, Register::RBP, next->offset, Register::RAX, ir::sizeOfType(inst->binary->dist->symbol.type));
                        break;
                    case VarType::RuntimeSymbol:
                        ops += Operation::ra(Opecode::MOV, Register::RAX, Register::RBP, vars[inst->binary->left]->offset, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::ra(opecode, Register::RAX, RMType::Runtime, vars[inst->binary->right]->indexOfData, ir::sizeOfType(inst->binary->right->symbol.type));
                        ops += Operation::ar(Opecode::MOV, RMType::Runtime, next->indexOfData, Register::RAX, ir::sizeOfType(inst->binary->left->symbol.type));
                        break;
                    default:
                        assert(false);
                    }
                    break;
                default:
                    assert(false);
                }
                break;
            case VarType::RuntimeSymbol:
                switch (vars[inst->binary->right]->type) {
                case VarType::Register:
                    switch (next->type) {
                    case VarType::Register:
                        ops += Operation::ra(Opecode::MOV, next->reg, RMType::Runtime, vars[inst->binary->left]->indexOfData, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::rr(opecode, next->reg, vars[inst->binary->right]->reg, ir::sizeOfType(inst->binary->right->symbol.type));
                        break;
                    case VarType::Spill:
                    case VarType::Phi:
                        ops += Operation::ra(Opecode::MOV, Register::RAX, RMType::Runtime, vars[inst->binary->left]->indexOfData, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::rr(opecode, Register::RAX, vars[inst->binary->right]->reg, ir::sizeOfType(inst->binary->right->symbol.type));
                        ops += Operation::ar(Opecode::MOV, Register::RBP, next->offset, Register::RAX, ir::sizeOfType(inst->binary->dist->symbol.type));
                        break;
                    case VarType::RuntimeSymbol:
                        ops += Operation::ra(Opecode::MOV, Register::RAX, RMType::Runtime, vars[inst->binary->left]->indexOfData, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::rr(opecode, Register::RAX, vars[inst->binary->right]->reg, ir::sizeOfType(inst->binary->right->symbol.type));
                        ops += Operation::ar(Opecode::MOV, RMType::Runtime, next->indexOfData, Register::RAX, ir::sizeOfType(inst->binary->left->symbol.type));
                        break;
                    default:
                        assert(false);
                    }
                    break;
                case VarType::Spill:
                case VarType::Phi:
                    switch (next->type) {
                    case VarType::Register:
                        ops += Operation::ra(Opecode::MOV, next->reg, RMType::Runtime, vars[inst->binary->left]->indexOfData, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::ra(opecode, next->reg, Register::RBP, vars[inst->binary->right]->offset, ir::sizeOfType(inst->binary->right->symbol.type));
                        break;
                    case VarType::Spill:
                    case VarType::Phi:
                        ops += Operation::ra(Opecode::MOV, Register::RAX, RMType::Runtime, vars[inst->binary->left]->indexOfData, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::ra(opecode, Register::RAX, Register::RBP, vars[inst->binary->right]->offset, ir::sizeOfType(inst->binary->right->symbol.type));
                        ops += Operation::ar(Opecode::MOV, Register::RBP, next->offset, Register::RAX, ir::sizeOfType(inst->binary->dist->symbol.type));
                        break;
                    case VarType::RuntimeSymbol:
                        ops += Operation::ra(Opecode::MOV, Register::RAX, RMType::Runtime, vars[inst->binary->left]->indexOfData, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::ra(opecode, Register::RAX, Register::RBP, vars[inst->binary->right]->offset, ir::sizeOfType(inst->binary->right->symbol.type));
                        ops += Operation::ar(Opecode::MOV, RMType::Runtime, next->indexOfData, Register::RAX, ir::sizeOfType(inst->binary->left->symbol.type));
                        break;
                    default:
                        assert(false);
                    }
                    break;
                case VarType::RuntimeSymbol:
                    switch (next->type) {
                    case VarType::Register:
                        ops += Operation::ra(Opecode::MOV, next->reg, RMType::Runtime, vars[inst->binary->left]->indexOfData, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::ra(opecode, next->reg, RMType::Runtime, vars[inst->binary->right]->indexOfData, ir::sizeOfType(inst->binary->right->symbol.type));
                        break;
                    case VarType::Spill:
                    case VarType::Phi:
                        ops += Operation::ra(Opecode::MOV, Register::RAX, RMType::Runtime, vars[inst->binary->left]->indexOfData, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::ra(opecode, Register::RAX, RMType::Runtime, vars[inst->binary->right]->indexOfData, ir::sizeOfType(inst->binary->right->symbol.type));
                        ops += Operation::ar(Opecode::MOV, Register::RBP, next->offset, Register::RAX, ir::sizeOfType(inst->binary->dist->symbol.type));
                        break;
                    case VarType::RuntimeSymbol:
                        ops += Operation::ra(Opecode::MOV, Register::RAX, RMType::Runtime, vars[inst->binary->left]->indexOfData, ir::sizeOfType(inst->binary->left->symbol.type));
                        ops += Operation::ra(opecode, Register::RAX, RMType::Runtime, vars[inst->binary->right]->indexOfData, ir::sizeOfType(inst->binary->right->symbol.type));
                        ops += Operation::ar(Opecode::MOV, RMType::Runtime, next->indexOfData, Register::RAX, ir::sizeOfType(inst->binary->left->symbol.type));
                        break;
                    default:
                        assert(false);
                    }
                    break;
                default:
                    assert(false);
                }
                break;
            default:
                assert(false);
            }
        };

        int argCount = 0;
        for (const auto& arg : fn->args) {
            vars[arg] = new Var{};
            switch (argCount) {
            case 0:
                vars[arg]->type = VarType::Register;
                vars[arg]->reg = Register::RCX;
                regs[Register::RCX] = { vars[arg], true };
                break;
            case 1:
                vars[arg]->type = VarType::Register;
                vars[arg]->reg = Register::RDX;
                regs[Register::RDX] = { vars[arg], true };
                break;
            case 2:
                vars[arg]->type = VarType::Register;
                vars[arg]->reg = Register::R8;
                regs[Register::R8] = { vars[arg], true };
                break;
            case 3:
                vars[arg]->type = VarType::Register;
                vars[arg]->reg = Register::R9;
                regs[Register::R9] = { vars[arg], true };
                break;
            default:
                vars[arg]->type = VarType::Spill;
                vars[arg]->offset = offset;
                offset -= 8;
                assert(false);
            }
            ++argCount;
        }

        for (const auto& block : fn->blocks) {
            for (const auto& inst : block->insts) {
                if (inst->type == InstructionType::Phi) {
                    Var* var = nullptr;
                    for (const auto& node : inst->phi->nodes) {
                        if (exists(vars, node.regs)) {
                            assert(vars[node.regs]->type == VarType::Phi);
                            var = vars[node.regs];
                            break;
                        }
                    }
                    if (var == nullptr) {
                        var = new Var{};
                        var->type = VarType::Phi;
                        var->offset = offset;
                        offset -= 8;
                    }
                    vars[inst->phi->dist] = var;
                    for (const auto& node : inst->phi->nodes) {
                        vars[node.regs] = var;
                    }
                }
            }
        }

        for (const auto& block : fn->blocks) {
            std::vector<Operation> ops;
            for (const auto& inst : block->insts) {
                switch (inst->type) {
                case InstructionType::Imm:
                {
                    Var* next = nullptr;
                    if (exists(vars, inst->imm->dist)) {
                        next = vars[inst->imm->dist];
                        assert(next->type == VarType::Phi);
                    }
                    else {
                        next = nextReg(inst->imm->dist);
                    }
                    switch (inst->imm->type) {
                    case ImmType::I8:
                        switch (next->type) {
                        case VarType::Register:
                            ops += Operation::ri(Opecode::MOV, next->reg, inst->imm->i8, 1);
                            break;
                        case VarType::Spill:
                        case VarType::Phi:
                            ops += Operation::ai(Opecode::MOV, Register::RBP, next->offset, inst->imm->i8, 1);
                            break;
                        default:
                            assert(false);
                        }
                        break;
                    case ImmType::I16:
                        switch (next->type) {
                        case VarType::Register:
                            ops += Operation::ri(Opecode::MOV, next->reg, inst->imm->i16, 2);
                            break;
                        case VarType::Spill:
                        case VarType::Phi:
                            ops += Operation::ai(Opecode::MOV, Register::RBP, next->offset, inst->imm->i16, 2);
                            break;
                        default:
                            assert(false);
                        }
                        break;
                    case ImmType::I32:
                        switch (next->type) {
                        case VarType::Register:
                            ops += Operation::ri(Opecode::MOV, next->reg, inst->imm->i32, 4);
                            break;
                        case VarType::Spill:
                        case VarType::Phi:
                            ops += Operation::ai(Opecode::MOV, Register::RBP, next->offset, inst->imm->i32, 8);
                            break;
                        default:
                            assert(false);
                        }
                        break;
                    case ImmType::I64:
                        switch (next->type) {
                        case VarType::Register:
                            ops += Operation::ri(Opecode::MOV, next->reg, inst->imm->i64, 8);
                            break;
                        case VarType::Spill:
                        case VarType::Phi:
                            ops += Operation::ai(Opecode::MOV, Register::RBP, next->offset, inst->imm->i64, 8);
                            break;
                        default:
                            assert(false);
                        }
                        break;
                    case ImmType::U8:
                        switch (next->type) {
                        case VarType::Register:
                            ops += Operation::ri(Opecode::MOV, next->reg, inst->imm->u8, 1);
                            break;
                        case VarType::Spill:
                        case VarType::Phi:
                            ops += Operation::ai(Opecode::MOV, Register::RBP, next->offset, inst->imm->u8, 1);
                            break;
                        default:
                            assert(false);
                        }
                        break;
                    case ImmType::U16:
                        switch (next->type) {
                        case VarType::Register:
                            ops += Operation::ri(Opecode::MOV, next->reg, inst->imm->u16, 2);
                            break;
                        case VarType::Spill:
                        case VarType::Phi:
                            ops += Operation::ai(Opecode::MOV, Register::RBP, next->offset, inst->imm->u16, 2);
                            break;
                        default:
                            assert(false);
                        }
                        break;
                    case ImmType::U32:
                        switch (next->type) {
                        case VarType::Register:
                            ops += Operation::ri(Opecode::MOV, next->reg, inst->imm->u32, 4);
                            break;
                        case VarType::Spill:
                        case VarType::Phi:
                            ops += Operation::ai(Opecode::MOV, Register::RBP, next->offset, inst->imm->u32, 8);
                            break;
                        default:
                            assert(false);
                        }
                        break;
                    case ImmType::U64:
                        switch (next->type) {
                        case VarType::Register:
                            ops += Operation::ri(Opecode::MOV, next->reg, inst->imm->u64, 8);
                            break;
                        case VarType::Spill:
                        case VarType::Phi:
                            ops += Operation::ai(Opecode::MOV, Register::RBP, next->offset, inst->imm->u64, 8);
                            break;
                        default:
                            assert(false);
                        }
                        break;
                    case ImmType::Char:
                        switch (next->type) {
                        case VarType::Register:
                            ops += Operation::ri(Opecode::MOV, next->reg, inst->imm->c, 1);
                            break;
                        case VarType::Spill:
                        case VarType::Phi:
                            ops += Operation::ai(Opecode::MOV, Register::RBP, next->offset, inst->imm->c, 1);
                            break;
                        default:
                            assert(false);
                        }
                        break;
                    default:
                        assert(false);
                    }
                    break;
                }
                case InstructionType::Add:
                    binaryOp(ops, Opecode::ADD, inst);
                    break;
                case InstructionType::Sub:
                    binaryOp(ops, Opecode::SUB, inst);
                    break;
                case InstructionType::Mul:
                    binaryOp(ops, Opecode::IMUL, inst);
                    break;
                case InstructionType::Div:
                    binaryOp(ops, Opecode::IDIV, inst);
                    break;
                case InstructionType::Mod:
                    binaryOp(ops, Opecode::IMOD, inst);
                    break;
                case InstructionType::Equal:
                    binaryOp(ops, Opecode::CMP, inst);
                    cond = Cond::Equal;
                    break;
                case InstructionType::NotEqual:
                    binaryOp(ops, Opecode::CMP, inst);
                    cond = Cond::NotEqual;
                    break;
                case InstructionType::GreaterEqual:
                    binaryOp(ops, Opecode::CMP, inst);
                    cond = Cond::GreaterEqual;
                    break;
                case InstructionType::GreaterThan:
                    binaryOp(ops, Opecode::CMP, inst);
                    cond = Cond::GreaterThan;
                    break;
                case InstructionType::LessEqual:
                    binaryOp(ops, Opecode::CMP, inst);
                    cond = Cond::LessEqual;
                    break;
                case InstructionType::LessThan:
                    binaryOp(ops, Opecode::CMP, inst);
                    cond = Cond::LessThan;
                    break;
                case InstructionType::Neg:
                {
                    assert(exists(vars, inst->neg->src));
                    assert(!exists(vars, inst->neg->dist));
                    nextReg(inst->neg->dist);
                    switch (vars[inst->neg->src]->type) {
                    case VarType::Register:
                        switch (vars[inst->neg->dist]->type) {
                        case VarType::Register:
                            ops += Operation::rr(Opecode::MOV, vars[inst->neg->dist]->reg, vars[inst->neg->src]->reg, ir::sizeOfType(inst->neg->dist->symbol.type));
                            ops += Operation::r(Opecode::NEG, vars[inst->neg->dist]->reg);
                            break;
                        case VarType::Spill:
                        case VarType::Phi:
                            ops += Operation::ar(Opecode::MOV, Register::RBP, vars[inst->neg->dist]->offset, vars[inst->neg->src]->reg, ir::sizeOfType(inst->neg->dist->symbol.type));
                            ops += Operation::a(Opecode::NEG, Register::RBP, vars[inst->neg->dist]->offset);
                            break;
                        }
                        break;
                    case VarType::Spill:
                    case VarType::Phi:
                        switch (vars[inst->neg->dist]->type) {
                        case VarType::Register:
                            ops += Operation::ra(Opecode::MOV, vars[inst->neg->dist]->reg, Register::RBP, vars[inst->neg->src]->offset, ir::sizeOfType(inst->neg->dist->symbol.type));
                            ops += Operation::r(Opecode::NEG, vars[inst->neg->dist]->reg);
                            break;
                        case VarType::Spill:
                        case VarType::Phi:
                            ops += Operation::ra(Opecode::MOV, Register::RAX, Register::RBP, vars[inst->neg->src]->offset, ir::sizeOfType(inst->neg->dist->symbol.type));
                            ops += Operation::ar(Opecode::MOV, Register::RBP, vars[inst->neg->dist]->offset, Register::RAX, ir::sizeOfType(inst->neg->dist->symbol.type));
                            ops += Operation::a(Opecode::NEG, Register::RBP, vars[inst->neg->dist]->offset);
                            break;
                        }
                        break;
                    default:
                        assert(false);
                    }
                    break;
                }
                case InstructionType::Load:
                    switch (inst->load->type) {
                    case LoadType::GlobalVariable:
                    {
                        bool found = false;
                        for (size_t i = 0, l = x86_64.constSymbols.size(); i < l; ++i) {
                            if (x86_64.constSymbols[i].name == inst->load->globalVar) {
                                auto var = new Var{};
                                switch (x86_64.constSymbols[i].type) {
                                case ConstantSymbolType::Immediate:
                                    assert(false);
                                    break;
                                case ConstantSymbolType::Relocation:
                                    switch (x86_64.constSymbols[i].reloc.type) {
                                    case RelocationType::ConstantSymbol:
                                        var->type = VarType::ConstantSymbol;
                                        break;
                                    case RelocationType::RuntimeSymbol:
                                        var->type = VarType::RuntimeSymbol;
                                        break;
                                    case RelocationType::Function:
                                        var->type = VarType::FunctionSymbol;
                                        break;
                                    case RelocationType::Extern:
                                        var->type = VarType::FunctionSymbol;
                                        break;
                                    default:
                                        assert(false);
                                    }
                                    break;
                                default:
                                    assert(false);
                                }
                                var->indexOfData = x86_64.constSymbols[i].reloc.indexOfData;
                                vars[inst->load->dist] = var;
                                found = true;
                            }
                        }
                        if (!found) {
                            for (size_t i = 0, l = x86_64.runtimeSymbols.size(); i < l; ++i) {
                                if (x86_64.runtimeSymbols[i].name == inst->load->globalVar) {
                                    auto var = new Var{};
                                    var->type = VarType::RuntimeSymbol;
                                    var->indexOfData = i;
                                    vars[inst->load->dist] = var;
                                    found = true;
                                }
                            }
                        }
                        assert(found);
                        break;
                    }
                    case LoadType::Array:
                    {
                        assert(exists(vars, inst->load->array.base));
                        assert(exists(vars, inst->load->array.suffix));
                        assert(vars[inst->load->array.base]->type == VarType::Array);
                        Var* next = nullptr;
                        if (exists(vars, inst->load->dist)) {
                            next = vars[inst->load->dist];
                            assert(next->type == VarType::Phi);
                        }
                        else {
                            next = nextReg(inst->load->dist);
                        }
                        ops += Operation::rr(Opecode::MOV, Register::RAX, Register::RBP, 8);
                        ops += Operation::ri(Opecode::ADD, Register::RAX, vars[inst->load->array.base]->array.base, 8);
                        switch (next->type) {
                        case VarType::Register:
                            switch (vars[inst->load->array.suffix]->type) {
                            case VarType::Register:
                                ops += Operation::rr(Opecode::MOV, next->reg, vars[inst->load->array.suffix]->reg, 8);
                                ops += Operation::ri(Opecode::IMUL, next->reg, vars[inst->load->array.base]->array.elemSize, 8);
                                ops += Operation::rr(Opecode::ADD, Register::RAX, next->reg, 8);
                                ops += Operation::ra(Opecode::MOV, next->reg, Register::RAX, 8);
                                break;
                            case VarType::Spill:
                            case VarType::Phi:
                                ops += Operation::ra(Opecode::MOV, next->reg, Register::RBP, vars[inst->load->array.suffix]->offset, 8);
                                ops += Operation::ri(Opecode::IMUL, next->reg, vars[inst->load->array.base]->array.elemSize, 8);
                                ops += Operation::rr(Opecode::ADD, Register::RAX, next->reg, 8);
                                ops += Operation::ra(Opecode::MOV, next->reg, Register::RAX, 8);
                                break;
                            default:
                                assert(false);
                            }
                            break;
                        case VarType::Spill:
                        case VarType::Phi:
                            switch (vars[inst->load->array.suffix]->type) {
                            case VarType::Register:
                                ops += Operation::r(Opecode::PUSH, Register::RAX);
                                ops += Operation::rr(Opecode::MOV, Register::RAX, vars[inst->load->array.suffix]->reg, 8);
                                ops += Operation::ri(Opecode::IMUL, Register::RAX, -vars[inst->load->array.base]->array.elemSize, 8);
                                ops += Operation::ar(Opecode::SUB, Register::RSP, 8, Register::RAX, 8);
                                ops += Operation::r(Opecode::POP, Register::RAX);
                                ops += Operation::ra(Opecode::MOV, Register::RAX, Register::RAX, 8);
                                ops += Operation::ar(Opecode::MOV, Register::RBP, next->offset, Register::RAX, 8);
                                break;
                            case VarType::Spill:
                            case VarType::Phi:
                                ops += Operation::r(Opecode::PUSH, Register::RAX);
                                ops += Operation::ra(Opecode::MOV, Register::RAX, Register::RBP, vars[inst->load->array.suffix]->offset, 8);
                                ops += Operation::ri(Opecode::IMUL, Register::RAX, -vars[inst->load->array.base]->array.elemSize, 8);
                                ops += Operation::ar(Opecode::SUB, Register::RSP, 8, Register::RAX, 8);
                                ops += Operation::r(Opecode::POP, Register::RAX);
                                ops += Operation::ra(Opecode::MOV, Register::RAX, Register::RAX, 8);
                                ops += Operation::ar(Opecode::MOV, Register::RBP, next->offset, Register::RAX, 8);
                                break;
                            default:
                                assert(false);
                            }
                            break;
                        default:
                            assert(false);
                        }
                        break;
                    }
                    default:
                        assert(false);
                    }
                    break;
                case InstructionType::Store:
                    assert(exists(vars, inst->store->value));
                    // 配列
                    if (inst->store->index) {
                        assert(exists(vars, inst->store->base));
                        assert(exists(vars, inst->store->index));
                        assert(vars[inst->store->base]->type == VarType::Array);
                        switch (vars[inst->store->value]->type) {
                        case VarType::Register:
                            switch (vars[inst->store->index]->type) {
                            case VarType::Register:
                                ops += Operation::rr(Opecode::MOV, Register::RAX, vars[inst->store->index]->reg, ir::sizeOfType(inst->store->index->symbol.type));
                                ops += Operation::ri(Opecode::IMUL, Register::RAX, vars[inst->store->base]->array.elemSize, 8);
                                ops += Operation::rr(Opecode::ADD, Register::RAX, Register::RBP, 8);
                                ops += Operation::ri(Opecode::ADD, Register::RAX, vars[inst->store->base]->array.base, 8);
                                ops += Operation::ar(Opecode::MOV, Register::RAX, vars[inst->store->value]->reg, ir::sizeOfType(inst->store->value->symbol.type));
                                break;
                            case VarType::Spill:
                            case VarType::Phi:
                                ops += Operation::ra(Opecode::MOV, Register::RAX, Register::RBP, vars[inst->store->index]->offset, ir::sizeOfType(inst->store->index->symbol.type));
                                ops += Operation::ri(Opecode::IMUL, Register::RAX, vars[inst->store->base]->array.elemSize, 8);
                                ops += Operation::rr(Opecode::ADD, Register::RAX, Register::RBP, 8);
                                ops += Operation::ri(Opecode::ADD, Register::RAX, vars[inst->store->base]->array.base, 8);
                                ops += Operation::ar(Opecode::MOV, Register::RAX, vars[inst->store->value]->reg, ir::sizeOfType(inst->store->value->symbol.type));
                                break;
                            default:
                                assert(false);
                            }
                            break;
                        case VarType::Spill:
                        case VarType::Phi:
                            switch (vars[inst->store->index]->type) {
                            case VarType::Register:
                                ops += Operation::rr(Opecode::MOV, Register::RAX, vars[inst->store->index]->reg, ir::sizeOfType(inst->store->index->symbol.type));
                                ops += Operation::ri(Opecode::IMUL, Register::RAX, vars[inst->store->base]->array.elemSize, 8);
                                ops += Operation::rr(Opecode::ADD, Register::RAX, Register::RBP, 8);
                                ops += Operation::ri(Opecode::ADD, Register::RAX, vars[inst->store->base]->array.base, 8);
                                ops += Operation::ra(Opecode::MOV, Register::R10, Register::RBP, vars[inst->store->value]->offset, ir::sizeOfType(inst->store->value->symbol.type));
                                ops += Operation::ar(Opecode::MOV, Register::RAX, Register::R10, ir::sizeOfType(inst->store->value->symbol.type));
                                break;
                            case VarType::Spill:
                            case VarType::Phi:
                                ops += Operation::ra(Opecode::MOV, Register::RAX, Register::RBP, vars[inst->store->index]->offset, ir::sizeOfType(inst->store->index->symbol.type));
                                ops += Operation::ri(Opecode::IMUL, Register::RAX, vars[inst->store->base]->array.elemSize, 8);
                                ops += Operation::rr(Opecode::ADD, Register::RAX, Register::RBP, 8);
                                ops += Operation::ri(Opecode::ADD, Register::RAX, vars[inst->store->base]->array.base, 8);
                                ops += Operation::ra(Opecode::MOV, Register::R10, Register::RBP, vars[inst->store->value]->offset, ir::sizeOfType(inst->store->value->symbol.type));
                                ops += Operation::ar(Opecode::MOV, Register::RAX, Register::R10, ir::sizeOfType(inst->store->value->symbol.type));
                                break;
                            default:
                                assert(false);
                            }
                            break;
                        default:
                            assert(false);
                        }
                    }
                    // ローカル変数
                    else if(inst->store->base) {
                        Var* next = nullptr;
                        if (exists(vars, inst->store->base)) {
                            next = vars[inst->store->base];
                        }
                        else {
                            next = nextReg(inst->store->base);
                        }
                        switch (next->type) {
                        case VarType::Register:
                            switch (vars[inst->store->value]->type) {
                            case VarType::Register:
                                ops += Operation::rr(Opecode::MOV, next->reg, vars[inst->store->value]->reg, ir::sizeOfType(inst->store->base->symbol.type));
                                break;
                            case VarType::Spill:
                            case VarType::Phi:
                                ops += Operation::ra(Opecode::MOV, next->reg, Register::RBP, vars[inst->store->value]->offset, ir::sizeOfType(inst->store->base->symbol.type));
                                break;
                            default:
                                assert(false);
                            }
                            break;
                        case VarType::Spill:
                        case VarType::Phi:
                            switch (vars[inst->store->value]->type) {
                            case VarType::Register:
                                ops += Operation::ar(Opecode::MOV, Register::RBP, next->offset, vars[inst->store->value]->reg, ir::sizeOfType(inst->store->base->symbol.type));
                                break;
                            case VarType::Spill:
                            case VarType::Phi:
                                ops += Operation::ra(Opecode::MOV, Register::RAX, Register::RBP, vars[inst->store->value]->offset, ir::sizeOfType(inst->store->value->symbol.type));
                                ops += Operation::ar(Opecode::MOV, Register::RBP, next->offset, Register::RAX, ir::sizeOfType(inst->store->base->symbol.type));
                                break;
                            default:
                                assert(false);
                            }
                            break;
                        default:
                            assert(false);
                        }
                    }
                    // グローバル変数
                    else {
                        auto var = new Var{};
                        var->type = VarType::RuntimeSymbol;
                        bool found = false;
                        for (size_t i = 0, l = x86_64.runtimeSymbols.size(); i < l; ++i) {
                            if (x86_64.runtimeSymbols[i].name == inst->store->globalVar) {
                                var->type = VarType::RuntimeSymbol;
                                var->indexOfData = i;
                                found = true;
                            }
                        }
                        assert(found);
                        switch (vars[inst->store->value]->type) {
                        case VarType::Register:
                            ops += Operation::ar(Opecode::MOV, RMType::Runtime, var->indexOfData, vars[inst->store->value]->reg, x86_64.runtimeSymbols[var->indexOfData].sizeOfSymbol);
                            break;
                        case VarType::Spill:
                        case VarType::Phi:
                            ops += Operation::ra(Opecode::MOV, Register::RAX, Register::RBP, vars[inst->store->value]->offset, x86_64.runtimeSymbols[var->indexOfData].sizeOfSymbol);
                            ops += Operation::ar(Opecode::MOV, RMType::Runtime, var->indexOfData, Register::RAX, x86_64.runtimeSymbols[var->indexOfData].sizeOfSymbol);
                            break;
                        default:
                            assert(false);
                        }
                        break;
                    }
                    break;
                case InstructionType::Alloc:
                {
                    assert(!exists(vars, inst->alloc->dist));
                    auto var = new Var{};
                    offset -= (inst->alloc->size + 7) / 8 * 8;
                    var->type = VarType::Array;
                    var->array.base = offset;
                    var->array.elemSize = ir::sizeOfType(*inst->alloc->dist->symbol.type.array.type);
                    vars[inst->alloc->dist] = var;
                    offset -= 8;
                    break;
                }
                case InstructionType::Br:
                    assert(cond != Cond::Undef);
                    switch (cond) {
                    case Cond::Equal:
                        ops += Operation::jcc(Opecode::JNE, indexOf(fn->blocks, inst->br->elseBlock));
                        break;
                    case Cond::NotEqual:
                        ops += Operation::jcc(Opecode::JE, indexOf(fn->blocks, inst->br->elseBlock));
                        break;
                    case Cond::GreaterEqual:
                        ops += Operation::jcc(Opecode::JL, indexOf(fn->blocks, inst->br->elseBlock));
                        break;
                    case Cond::GreaterThan:
                        ops += Operation::jcc(Opecode::JLE, indexOf(fn->blocks, inst->br->elseBlock));
                        break;
                    case Cond::LessEqual:
                        ops += Operation::jcc(Opecode::JG, indexOf(fn->blocks, inst->br->elseBlock));
                        break;
                    case Cond::LessThan:
                        ops += Operation::jcc(Opecode::JGE, indexOf(fn->blocks, inst->br->elseBlock));
                        break;
                    default:
                        assert(false);
                    }
                    cond = Cond::Undef;
                    break;
                case InstructionType::Jmp:
                    ops += Operation::jcc(Opecode::JMP, indexOf(fn->blocks, inst->jmp->block));
                    break;
                case InstructionType::Phi:
                    break;
                case InstructionType::Call:
                {
                    auto pushed = saveRegs(ops);
                    int argCount = 0;
                    for (const auto& arg : inst->call->args) {
                        const auto argPush = [&](Register reg) {
                            switch (vars[arg]->type) {
                            case VarType::Register:
                                ops += Operation::rr(Opecode::MOV, reg, vars[arg]->reg, 8);
                                break;
                            case VarType::Spill:
                            case VarType::Phi:
                                ops += Operation::ra(Opecode::MOV, reg, Register::RBP, vars[arg]->offset, 8);
                                break;
                            case VarType::ConstantSymbol:
                                ops += Operation::ra(Opecode::MOV, reg, RMType::Constant, vars[arg]->indexOfData, 8);
                                break;
                            case VarType::RuntimeSymbol:
                                ops += Operation::ra(Opecode::MOV, reg, RMType::Runtime, vars[arg]->indexOfData, 8);
                                break;
                            case VarType::Array:
                                ops += Operation::addressof(reg, vars[arg]->array.base);
                                break;
                            default:
                                assert(false);
                            }
                        };
                        switch (argCount) {
                        case 0:
                            argPush(Register::RCX);
                            break;
                        case 1:
                            argPush(Register::RDX);
                            break;
                        case 2:
                            argPush(Register::R8);
                            break;
                        case 3:
                            argPush(Register::R9);
                            break;
                        default:
                            switch (vars[arg]->type) {
                            case VarType::Register:
                                ops += Operation::r(Opecode::PUSH, vars[arg]->reg);
                                break;
                            case VarType::Spill:
                            case VarType::Phi:
                                ops += Operation::a(Opecode::PUSH, Register::RBP, vars[arg]->offset);
                                break;
                            default:
                                assert(false);
                            }
                            break;
                        }
                        ++argCount;
                    }
                    switch (vars[inst->call->fn]->type) {
                    case VarType::Register:
                        assert(false);
                        break;
                    case VarType::Spill:
                    case VarType::Phi:
                        assert(false);
                        break;
                    case VarType::FunctionSymbol:
                        ops += Operation::call(vars[inst->call->fn]->indexOfData);
                        break;
                    default:
                        assert(false);
                    }
                    if (argCount >= 4) {
                        ops += Operation::ri(Opecode::ADD, Register::RSP, argCount - 4, 8);
                    }

                    restoreReg(ops, pushed);

                    if (inst->call->dist) {
                        auto next = nextReg(inst->call->dist);
                        switch (next->type) {
                        case VarType::Register:
                            ops += Operation::rr(Opecode::MOV, next->reg, Register::RAX, 8);
                            break;
                        case VarType::Spill:
                        case VarType::Phi:
                            ops += Operation::ar(Opecode::MOV, Register::RBP, next->offset, Register::RAX, 8);
                            break;
                        default:
                            assert(false);
                        }
                    }
                    break;
                }
                case InstructionType::Return:
                    assert(exists(vars, inst->ret->reg));
                    switch (vars[inst->ret->reg]->type) {
                    case VarType::Register:
                        ops += Operation::rr(Opecode::MOV, Register::RAX, vars[inst->ret->reg]->reg, ir::sizeOfType(inst->ret->reg->symbol.type));
                        break;
                    case VarType::Spill:
                    case VarType::Phi:
                        ops += Operation::ra(Opecode::MOV, Register::RAX, Register::RBP, vars[inst->ret->reg]->offset, ir::sizeOfType(inst->ret->reg->symbol.type));
                        break;
                    case VarType::RuntimeSymbol:
                        ops += Operation::ra(Opecode::MOV, Register::RAX, RMType::Runtime, vars[inst->ret->reg]->indexOfData, ir::sizeOfType(inst->ret->reg->symbol.type));
                        break;
                    default:
                        assert(false);
                    }
                    ops += Operation::ret();
                    break;
                default:
                    assert(false);
                }
                ++lifeTime;
            }
            result += ops;
        }

        if (errors.empty()) return ok(result);
        else return error(errors);
    }
    Result<_, std::vector<std::string>> X86_64Compiler::compileOperation(Routine& routine, int32_t len, const std::vector<uint8_t>& prologue, const std::vector<uint8_t>& epilogue)
    {
        struct BlockInfo
        {
            std::vector<uint8_t> code;
            size_t entryIndex;
            std::optional<size_t> jmpTo;
        };
        std::vector<BlockInfo> blocks;

        routine.code += prologue;

        for (const auto& ops : routine.ops) {
            BlockInfo block;
            if (blocks.empty()) block.entryIndex = routine.code.size();
            else block.entryIndex = blocks.back().entryIndex + blocks.back().code.size();
            for (const auto& op : ops) {
                assert(!block.jmpTo);
                switch (op.opecode) {
                case Opecode::_NONE:
                    break;
                case Opecode::ADD:
                    xBinaryOp(routine, block.entryIndex, block.code, op, len, addRR, addRA, addRA, addAR, addAR, addRI, addAI);
                    break;
                case Opecode::SUB:
                    xBinaryOp(routine, block.entryIndex, block.code, op, len, subRR, subRA, subRA, subAR, subAR, subRI, subAI);
                    break;
                case Opecode::IMUL:
                    xBinaryOp(routine, block.entryIndex, block.code, op, len, imulRR, imulRA, imulRA, imulAR, imulAR, imulRI, imulAI);
                    break;
                case Opecode::IDIV:
                    xBinaryOp(routine, block.entryIndex, block.code, op, len, idivRR, idivRA, idivRA, idivAR, idivAR, idivRI, idivAI);
                    break;
                case Opecode::IMOD:
                    xBinaryOp(routine, block.entryIndex, block.code, op, len, imodRR, imodRA, imodRA, imodAR, imodAR, imodRI, imodAI);
                    break;
                case Opecode::CMP:
                    xBinaryOp(routine, block.entryIndex, block.code, op, len, cmpRR, cmpRA, cmpRA, cmpAR, cmpAR, cmpRI, cmpAI);
                    break;
                case Opecode::MOV:
                    xBinaryOp(routine, block.entryIndex, block.code, op, len, movRR, movRA, movRA, movAR, movAR, movRI, movAI);
                    break;
                case Opecode::NEG:
                    switch (op.op1.type) {
                    case RMType::Register:
                        negR(block.code, op.op1.reg);
                        break;
                    case RMType::Memory:
                        negA(block.code, op.op1.mem.base, op.op1.mem.disp + len);
                        break;
                    default:
                        assert(false);
                    }
                    break;
                case Opecode::JMP:
                    assert(op.op1.type == RMType::Address);
                    /*
                        blocks.size() + 1 == op.op1.address
                        であればジャンプ先が次の命令になるので、
                        ジャンプ命令自体を削除しても問題ない。
                        他ジャンプ命令も同様。
                    */
                    if (blocks.size() + 1 != op.op1.address) {
                        block.jmpTo = op.op1.address;
                        jmpA(block.code, -1);
                    }
                    break;
                case Opecode::JE:
                    assert(op.op1.type == RMType::Address);
                    /*block.jmpTo = op.size;
                    je(block.code, -1);*/
                    if (blocks.size() + 1 != op.op1.address) {
                        block.jmpTo = op.op1.address;
                        je(block.code, -1);
                    }
                    break;
                case Opecode::JNE:
                    assert(op.op1.type == RMType::Address);
                    /*block.jmpTo = op.size;
                    jne(block.code, -1);*/
                    if (blocks.size() + 1 != op.op1.address) {
                        block.jmpTo = op.op1.address;
                        jne(block.code, -1);
                    }
                    break;
                case Opecode::JG:
                    assert(op.op1.type == RMType::Address);
                    /*block.jmpTo = op.size;
                    jg(block.code, -1);*/
                    if (blocks.size() + 1 != op.op1.address) {
                        block.jmpTo = op.op1.address;
                        jg(block.code, -1);
                    }
                    break;
                case Opecode::JGE:
                    assert(op.op1.type == RMType::Address);
                    /*block.jmpTo = op.size;
                    jge(block.code, -1);*/
                    if (blocks.size() + 1 != op.op1.address) {
                        block.jmpTo = op.op1.address;
                        jge(block.code, -1);
                    }
                    break;
                case Opecode::JL:
                    assert(op.op1.type == RMType::Address);
                    /*block.jmpTo = op.size;
                    jl(block.code, -1);*/
                    if (blocks.size() + 1 != op.op1.address) {
                        block.jmpTo = op.op1.address;
                        jl(block.code, -1);
                    }
                    break;
                case Opecode::JLE:
                    assert(op.op1.type == RMType::Address);
                    /*block.jmpTo = op.size;
                    jle(block.code, -1);*/
                    if (blocks.size() + 1 != op.op1.address) {
                        block.jmpTo = op.op1.address;
                        jle(block.code, -1);
                    }
                    break;
                case Opecode::CALL:
                    assert(op.op1.type == RMType::Address);
                    routine.relocs.push_back(Relocation{ block.entryIndex + block.code.size() + 1, RelocationType::Function, op.op1.address });
                    callA(block.code, -1);
                    break;
                case Opecode::RET:
                    block.code += epilogue;
                    break;
                case Opecode::PUSH:
                    switch (op.op1.type) {
                    case RMType::Register:
                        pushR(block.code, op.op1.reg);
                        break;
                    case RMType::Memory:
                        pushA(block.code, op.op1.mem.base, op.op1.mem.disp + len);
                        break;
                    default:
                        assert(false);
                    }
                    break;
                case Opecode::POP:
                    switch (op.op1.type) {
                    case RMType::Register:
                        popR(block.code, op.op1.reg);
                        break;
                    case RMType::Memory:
                        // ra(code, op.op1.reg, op.op2->mem.base, op.op2->mem.disp + len, op.size);
                        popA(block.code, op.op1.mem.base, op.op1.mem.disp + len);
                        break;
                    default:
                        assert(false);
                    }
                    break;
                case Opecode::ADDRESS_OF:
                    assert(op.op1.type == RMType::Address);
                    movRR(block.code, op.op1.reg, Register::RBP, 8);
                    addRI(block.code, op.op1.reg, op.op1.address + len, 8);
                    break;
                default:
                    assert(false);
                }
            }
            blocks += block;
        }

        for (auto& block : blocks) {
            if (block.jmpTo) {
                auto address = blocks[block.jmpTo.value()].entryIndex - block.entryIndex - block.code.size();
                block.code[block.code.size() - 4] = static_cast<uint8_t>(address);
                block.code[block.code.size() - 3] = static_cast<uint8_t>(address >> 8);
                block.code[block.code.size() - 2] = static_cast<uint8_t>(address >> 16);
                block.code[block.code.size() - 1] = static_cast<uint8_t>(address >> 24);
            }
            routine.code += block.code;
        }

        return ok();
    }
    Result<Routine, std::vector<std::string>> X86_64Compiler::compileRoutine(const ir::Function* fn)
    {
        using namespace ir;
        assert(!fn->blocks.empty());
        Routine routine;

        std::unordered_map<x86_64::Register, RegState> regs;
        int32_t offset = -8;
        
        auto res = commonCompileRoutine(fn, regs, offset);
        if (!res) return error(res.err());
        routine.ops = res.get();

        std::vector<uint8_t> prologue;
        pushR(prologue, Register::RBP);
        movRR(prologue, Register::RBP, Register::RSP, 8);
        int32_t len = 0;
        for (auto reg : {
            Register::RBX,
            Register::RDI,
            Register::RSI,
            Register::R12,
            Register::R13,
            Register::R14,
            Register::R15,
            }) {
            if (regs[reg].hasBeenUsed) {
                len -= 8;
                movAR(prologue, Register::RBP, len, reg, 8);
            }
        }

        if (len + offset != 0) {
            subRI(prologue, Register::RSP, -((len + offset - 15) / 16 * 16), 8);
        }

        std::vector<uint8_t> epilogue;
        if (len != 0) {
            int32_t _len = 0;
            for (auto reg : {
            Register::RBX,
            Register::RDI,
            Register::RSI,
            Register::R12,
            Register::R13,
            Register::R14,
            Register::R15,
                }) {
                if (regs[reg].hasBeenUsed) {
                    _len -= 8;
                    movRA(epilogue, reg, Register::RBP, _len, 8);
                }
            }
        }
        leave(epilogue);
        ret(epilogue);

        compileOperation(routine, len, prologue, epilogue);
        
        return ok(routine);
    }
    X86_64Compiler::X86_64Compiler(const ir::IntermediateRepresentation& ir) : ir(ir) {}
    void symbol(X86_64& x86_64, const std::string& prefix, const ir::SymbolTable* table, uint64_t& fnCount, uint64_t& constCount, uint64_t& runCount)
    {
        for (const auto& symbol : table->symbols) {
            if (symbol.second.type.type == ir::Types::SymbolTable) {
                continue;
            }
            else if (symbol.second.type.type == ir::Types::Function) {
                if (!symbol.second.function->blocks.empty()) {
                    x86_64.constSymbols += ConstantSymbol(prefix + symbol.first, Relocation{ 0, RelocationType::Function, fnCount });
                }
                else {
                    x86_64.constSymbols += ConstantSymbol(prefix + symbol.first, Relocation{ 0, RelocationType::Extern, fnCount, symbol.first });
                    x86_64.exts.insert(symbol.first);
                }
                ++fnCount;
            }
            else {
                x86_64.runtimeSymbols += RuntimeSymbol{ 0, prefix + symbol.first, 8, symbol.second.init };
                ++runCount;
            }
        }
        for (const auto& sub : table->subSymbolTable) {
            symbol(x86_64, prefix + sub.second->name + "::", sub.second, fnCount, constCount, runCount);
        }
    }
    Result<X86_64, std::vector<std::string>> X86_64Compiler::compile()
    {
        std::vector<std::string> errors;
        uint64_t fnCount = 0, constCount = 0, runCount = 0;
        symbol(x86_64, "::", ir.rootSymbolTable, fnCount, constCount, runCount);

        for (const auto& fn : ir.functions) {
            if (fn->blocks.empty()) {
                Routine routine;
                routine.relocs.push_back(Relocation{ routine.code.size() + 2, RelocationType::Extern, x86_64.exts.size(), fn->externName });
                jmpF(routine.code, 0);
                x86_64.routines += routine;
            }
            else {
                auto routine = compileRoutine(fn);
                if (routine) x86_64.routines += routine.get();
                else errors += routine.err();
            }
        }

        std::unordered_map<x86_64::Register, RegState> invokeMainRegs;
        int32_t invokeMainOffset = -8;
        for (auto& runtime : x86_64.runtimeSymbols) {
            using namespace ir;
            Routine routine;

            std::unordered_map<x86_64::Register, RegState> regs;
            int32_t offset = -8;

            auto res = commonCompileRoutine(runtime.init, regs, offset);
            if (!res) {
                errors += res.err();
                continue;
            }
            x86_64.invokeMain.ops += res.get();
            invokeMainOffset = std::min(invokeMainOffset, offset);
            for (const auto& reg : regs) {
                if (reg.second.hasBeenUsed) {
                    invokeMainRegs[reg.first].hasBeenUsed = true;
                }
            }
        }

        std::vector<uint8_t> invokeMainPrologue;
        pushR(invokeMainPrologue, Register::RBP);
        movRR(invokeMainPrologue, Register::RBP, Register::RSP, 8);

        /*for (auto& runtime : x86_64.runtimeSymbols) {
            for (auto& reloc : runtime.initRoutine.relocs) {
                reloc.indexOfData += x86_64.invokeMain.code.size();
                x86_64.invokeMain.relocs += reloc;
            }
        }*/

        int32_t invokeMainLen = 0;
        for (auto reg : {
            Register::RBX,
            Register::RDI,
            Register::RSI,
            Register::R12,
            Register::R13,
            Register::R14,
            Register::R15,
            }) {
            if (invokeMainRegs[reg].hasBeenUsed) {
                invokeMainLen -= 8;
                movAR(invokeMainPrologue, Register::RBP, invokeMainLen, reg, 8);
            }
        }

        if (invokeMainLen + invokeMainOffset != 0) {
            subRI(invokeMainPrologue, Register::RSP, -((invokeMainLen + invokeMainOffset - 15) / 16 * 16), 8);
        }

        std::vector<uint8_t> invokeMainEpilogue;
        std::optional<Relocation> indexOfMainSymbol;
        for (size_t i = 0, l = x86_64.constSymbols.size(); i < l; ++i) {
            std::string main("::main");
            if (std::equal(std::rbegin(main), std::rend(main), std::rbegin(x86_64.constSymbols[i].name))) {
                indexOfMainSymbol = x86_64.constSymbols[i].reloc;
                break;
            }
        }
        if (!indexOfMainSymbol) {
            errors += "エラー: mainシンボルが見つかりませんでした。";
        }
        else {
            x86_64.invokeMain.ops.back() += Operation::call(indexOfMainSymbol.value().indexOfData);
            /*x86_64.invokeMain.relocs += Relocation{ x86_64.invokeMain.code.size() + 1, RelocationType::Function, indexOfMainSymbol.value().indexOfData };
            callA(invokeMainEpilogue, -1);*/
        }

        if (invokeMainLen != 0) {
            int32_t len = 0;
            for (auto reg : {
            Register::RBX,
            Register::RDI,
            Register::RSI,
            Register::R12,
            Register::R13,
            Register::R14,
            Register::R15,
                }) {
                if (invokeMainRegs[reg].hasBeenUsed) {
                    len -= 8;
                    movRA(invokeMainEpilogue, reg, Register::RBP, len, 8);
                }
            }
        }
        leave(invokeMainEpilogue);
        ret(invokeMainEpilogue);

        x86_64.invokeMain.ops.back() += Operation::ret();
        compileOperation(x86_64.invokeMain, invokeMainLen, invokeMainPrologue, invokeMainEpilogue);

        if (errors.empty()) return ok(x86_64);
        else return error(errors);
    }

    void addRR(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        opRR(code, 0x00, 0x01, dist, src, size);
    }
    void addRA(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        opRA(code, 0x02, 0x03, dist, src, size);
    }
    void addRA(std::vector<uint8_t>& code, Register dist, uint32_t address, size_t size)
    {
        opRA(code, 0x02, 0x03, dist, address, size);
    }
    void addRA(std::vector<uint8_t>& code, Register dist, Register base, int32_t ref, size_t size)
    {
        opRA(code, 0x02, 0x03, dist, base, ref, size);
    }
    void addRA(std::vector<uint8_t>& code, Register dist, Register base, Register index, int scale, int32_t ref, size_t size)
    {
        opRA(code, 0x02, 0x03, dist, base, index, scale, ref, size);
    }
    void addRI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size)
    {
        opImmGrp1R(code, ImmGrp1::ADD, dist, imm, size);
    }
    void addAR(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        opAR(code, 0x02, 0x03, dist, src, size);
    }
    void addAR(std::vector<uint8_t>& code, uint32_t address, Register src, size_t size)
    {
        opAR(code, 0x00, 0x01, address, src, size);
    }
    void addAR(std::vector<uint8_t>& code, Register base, int32_t ref, Register src, size_t size)
    {
        opAR(code, 0x00, 0x01, base, ref, src, size);
    }
    void addAR(std::vector<uint8_t>& code, Register base, Register index, int scale, int32_t ref, Register src, size_t size)
    {
        opAR(code, 0x00, 0x01, base, ref, src, size);
    }
    void addAI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size)
    {
        opImmGrp1A(code, ImmGrp1::ADD, dist, imm, size);
    }
    void addAI(std::vector<uint8_t>& code, uint32_t address, int32_t imm, size_t size)
    {
        opImmGrp1A(code, ImmGrp1::ADD, address, imm, size);
    }
    void addAI(std::vector<uint8_t>& code, Register base, int32_t ref, int32_t imm, size_t size)
    {
        opImmGrp1A(code, ImmGrp1::ADD, base, ref, imm, size);
    }

    void subRR(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        opRR(code, 0x28, 0x29, dist, src, size);
    }
    void subRA(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        opRA(code, 0x2A, 0x2B, dist, src, size);
    }
    void subRA(std::vector<uint8_t>& code, Register dist, uint32_t address, size_t size)
    {
        opRA(code, 0x2A, 0x2B, dist, address, size);
    }
    void subRA(std::vector<uint8_t>& code, Register dist, Register base, int32_t ref, size_t size)
    {
        opRA(code, 0x2A, 0x2B, dist, base, ref, size);
    }
    void subRA(std::vector<uint8_t>& code, Register dist, Register base, Register index, int scale, int32_t ref, size_t size)
    {
        opRA(code, 0x2A, 0x2B, dist, base, index, scale, ref, size);
    }
    void subRI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size)
    {
        opImmGrp1R(code, ImmGrp1::SUB, dist, imm, size);
    }
    void subAR(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        opAR(code, 0x28, 0x29, dist, src, size);
    }
    void subAR(std::vector<uint8_t>& code, uint32_t address, Register src, size_t size)
    {
        opAR(code, 0x28, 0x29, address, src, size);
    }
    void subAR(std::vector<uint8_t>& code, Register base, int32_t ref, Register src, size_t size)
    {
        opAR(code, 0x28, 0x29, base, ref, src, size);
    }
    void subAR(std::vector<uint8_t>& code, Register base, Register index, int scale, int32_t ref, Register src, size_t size)
    {
        opAR(code, 0x28, 0x29, base, index, scale, ref, src, size);
    }
    void subAI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size)
    {
        opImmGrp1A(code, ImmGrp1::SUB, dist, imm, size);
    }
    void subAI(std::vector<uint8_t>& code, uint32_t address, int32_t imm, size_t size)
    {
        opImmGrp1A(code, ImmGrp1::SUB, address, imm, size);
    }
    void subAI(std::vector<uint8_t>& code, Register base, int32_t ref, int32_t imm, size_t size)
    {
        opImmGrp1A(code, ImmGrp1::SUB, base, ref, imm, size);
    }

    void imulRR(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        assert(size == 2 || size == 4 || size == 8);
        if (size == 2) code.push_back(0x66);
        uint8_t rex = 0;
        if (size == 8) rex |= 0b1000;
        if (!isX86Reg(src)) rex |= 0b0001;
        if (!isX86Reg(dist)) rex |= 0b0100;
        if (rex != 0) code.push_back(0b0100'0000 | rex);
        code.push_back(0x0F);
        code.push_back(0xAF);
        code.push_back(0b11'000'000 | modRM(src, dist));
    }
    void imulRA(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        assert(size == 2 || size == 4 || size == 8);
        if (size == 2) code.push_back(0x66);
        uint8_t rex = 0;
        if (size == 8) rex |= 0b1000;
        if (!isX86Reg(src)) rex |= 0b0001;
        if (!isX86Reg(dist)) rex |= 0b0100;
        if (rex != 0) code.push_back(0b0100'0000 | rex);
        code.push_back(0x0F);
        code.push_back(0xAF);
        if (src == Register::RBP || src == Register::R13) {
            code.push_back(0b01'000'000 | modRM(src, dist));
            code.push_back(0);
        }
        else {
            code.push_back(0b00'000'000 | modRM(src, dist));
            if (src == Register::RSP || src == Register::R12) {
                code.push_back(0b00'101'000);
            }
        }
    }
    void imulRA(std::vector<uint8_t>& code, Register dist, uint32_t address, size_t size)
    {
        assert(size == 2 || size == 4 || size == 8);
        if (size == 2) code.push_back(0x66);
        uint8_t rex = 0;
        if (size == 8) rex |= 0b1000;
        if (!isX86Reg(dist)) rex |= 0b0100;
        if (rex != 0) code.push_back(0b0100'0000 | rex);
        code.push_back(0x0F);
        code.push_back(0xAF);
        code.push_back(0b00'000'000 | modRM(Register::RSP, dist));
        code.push_back(0x25);
        code.push_back(address);
        code.push_back(address >> 8);
        code.push_back(address >> 16);
        code.push_back(address >> 24);
    }
    void imulRA(std::vector<uint8_t>& code, Register dist, Register base, int32_t ref, size_t size)
    {
        assert(size == 2 || size == 4 || size == 8);
        uint8_t rex = 0;
        if (size == 8) rex |= 0b1000;
        if (!isX86Reg(base)) rex |= 0b0001;
        if (!isX86Reg(dist)) rex |= 0b0100;
        if (rex != 0) code.push_back(0b0100'0000 | rex);
        code.push_back(0x0F);
        code.push_back(0xAF);
        if (ref <= INT8_MAX && ref >= INT8_MIN) {
            code.push_back(0b01'000'000 | modRM(base, dist));
            if (base == Register::RSP || base == Register::R12) {
                code.push_back(0b00'101'000);
            }
            code.push_back(ref);
        }
        else {
            code.push_back(0b10'000'000 | modRM(base, dist));
            if (base == Register::RSP || base == Register::R12) {
                code.push_back(0b00'101'000);
            }
            code.push_back(ref);
            code.push_back(ref >> 8);
            code.push_back(ref >> 16);
            code.push_back(ref >> 24);
        }
    }
    void imulRI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size)
    {
        assert(size == 2 || size == 4 || size == 8);
        if (size == 2) assert(imm <= INT16_MAX && imm >= INT16_MIN);

        if (size == 2) code.push_back(0x66);
        uint8_t rex = 0;
        if (size == 8) rex |= 0b1000;
        if (!isX86Reg(dist)) rex |= 0b0100;
        if (rex != 0) code.push_back(0b0100'0000 | rex);

        if(imm <= INT8_MAX && imm >= INT8_MIN){
            code.push_back(0x6B);
            code.push_back(0b11'000'000 | modRM(dist, dist));
            code.push_back(imm);
        }
        else {
            code.push_back(0x69);
            code.push_back(0b11'000'000 | modRM(dist, dist));
            code.push_back(imm);
            code.push_back(imm >> 8);
            code.push_back(imm >> 16);
            code.push_back(imm >> 24);
        }
    }
    void imulAR(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        imulRA(code, src, dist, size);
        movAR(code, dist, src, size);
    }
    void imulAR(std::vector<uint8_t>& code, uint32_t address, Register src, size_t size)
    {
        imulRA(code, src, address, size);
        movAR(code, address, src, size);
    }
    void imulAR(std::vector<uint8_t>& code, Register base, int32_t ref, Register src, size_t size)
    {
        imulRA(code, src, base, ref, size);
        movAR(code, base, ref, src, size);
    }
    void imulAI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size)
    {
        movRA(code, Register::RAX, dist, size);
        imulRI(code, Register::RAX, imm, size);
        movAR(code, dist, Register::RAX, size);
    }
    void imulAI(std::vector<uint8_t>& code, uint32_t address, int32_t imm, size_t size)
    {
        movRA(code, Register::RAX, address, size);
        imulRI(code, Register::RAX, imm, size);
        movAR(code, address, Register::RAX, size);
    }
    void imulAI(std::vector<uint8_t>& code, Register base, int32_t ref, int32_t imm, size_t size)
    {
        movRA(code, Register::RAX, base, ref, size);
        imulRI(code, Register::RAX, imm, size);
        movAR(code, base, ref, Register::RAX, size);
    }

    void idivRR(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        xidivRR(code, dist, src, size);
        if(dist != Register::RAX) movRR(code, dist, Register::RAX, size);
    }
    void idivRA(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        xidivRA(code, dist, src, size);
        if(dist != Register::RAX) movRR(code, dist, Register::RAX, size);
    }
    void idivRA(std::vector<uint8_t>& code, Register dist, uint32_t address, size_t size)
    {
        xidivRA(code, dist, address, size);
        if (dist != Register::RAX) movRR(code, dist, Register::RAX, size);
    }
    void idivRA(std::vector<uint8_t>& code, Register dist, Register base, int32_t ref, size_t size)
    {
        xidivRA(code, dist, base, ref, size);
        if (dist != Register::RAX) movRR(code, dist, Register::RAX, size);
    }
    void idivRI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size)
    {
        xidivRI(code, dist, imm, size);
        if (dist != Register::RAX) movRR(code, dist, Register::RAX, size);
    }
    void idivAR(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        xidivAR(code, dist, src, size);
        if (dist != Register::RAX) movAR(code, dist, Register::RAX, size);
    }
    void idivAR(std::vector<uint8_t>& code, uint32_t address, Register src, size_t size)
    {
        xidivAR(code, address, src, size);
        movAR(code, address, Register::RAX, size);
    }
    void idivAR(std::vector<uint8_t>& code, Register base, int32_t ref, Register src, size_t size)
    {
        xidivAR(code, base, ref, src, size);
        movAR(code, base, ref, Register::RAX, size);
    }
    void idivAI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size)
    {
        xidivAI(code, dist, imm, size);
        movAR(code, dist, Register::RAX, size);
    }
    void idivAI(std::vector<uint8_t>& code, uint32_t address, int32_t imm, size_t size)
    {
        xidivAI(code, address, imm, size);
        movAR(code, address, Register::RAX, size);
    }
    void idivAI(std::vector<uint8_t>& code, Register base, int32_t ref, int32_t imm, size_t size)
    {
        xidivAI(code, base, ref, imm, size);
        movAR(code, base, ref, Register::RAX, size);
    }

    void imodRR(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        xidivRR(code, dist, src, size);
        if (dist != Register::RDX) movRR(code, dist, Register::RDX, size);
    }
    void imodRA(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        xidivRA(code, dist, src, size);
        if (dist != Register::RDX) movRR(code, dist, Register::RDX, size);
    }
    void imodRA(std::vector<uint8_t>& code, Register dist, uint32_t address, size_t size)
    {
        xidivRA(code, dist, address, size);
        if (dist != Register::RDX) movRR(code, dist, Register::RDX, size);
    }
    void imodRA(std::vector<uint8_t>& code, Register dist, Register base, int32_t ref, size_t size)
    {
        xidivRA(code, dist, base, ref, size);
        if (dist != Register::RDX) movRR(code, dist, Register::RDX, size);
    }
    void imodRI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size)
    {
        xidivRI(code, dist, imm, size);
        if (dist != Register::RDX) movRR(code, dist, Register::RDX, size);
    }
    void imodAR(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        xidivAR(code, dist, src, size);
        if (dist != Register::RDX) movAR(code, dist, Register::RDX, size);
    }
    void imodAR(std::vector<uint8_t>& code, uint32_t address, Register src, size_t size)
    {
        xidivAR(code, address, src, size);
        movAR(code, address, Register::RDX, size);
    }
    void imodAR(std::vector<uint8_t>& code, Register base, int32_t ref, Register src, size_t size)
    {
        xidivAR(code, base, ref, src, size);
        movAR(code, base, ref, Register::RDX, size);
    }
    void imodAI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size)
    {
        xidivAI(code, dist, imm, size);
        movAR(code, dist, Register::RDX, size);
    }
    void imodAI(std::vector<uint8_t>& code, uint32_t address, int32_t imm, size_t size)
    {
        xidivAI(code, address, imm, size);
        movAR(code, address, Register::RDX, size);
    }
    void imodAI(std::vector<uint8_t>& code, Register base, int32_t ref, int32_t imm, size_t size)
    {
        xidivAI(code, base, ref, imm, size);
        movAR(code, base, ref, Register::RDX, size);
    }

    void cmpRR(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        opRR(code, 0x38, 0x39, dist, src, size);
    }
    void cmpRA(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        opRA(code, 0x3A, 0x3B, dist, src, size);
    }
    void cmpRA(std::vector<uint8_t>& code, Register dist, uint32_t address, size_t size)
    {
        opRA(code, 0x3A, 0x3B, dist, address, size);
    }
    void cmpRA(std::vector<uint8_t>& code, Register dist, Register base, int32_t ref, size_t size)
    {
        opRA(code, 0x3A, 0x3B, dist, base, ref, size);
    }
    void cmpRA(std::vector<uint8_t>& code, Register dist, Register base, Register index, int scale, int32_t ref, size_t size)
    {
        opRA(code, 0x3A, 0x3B, dist, base, index, scale, ref, size);
    }
    void cmpRI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size)
    {
        opImmGrp1R(code, ImmGrp1::CMP, dist, imm, size);
    }
    void cmpAR(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        opAR(code, 0x38, 0x39, dist, src, size);
    }
    void cmpAR(std::vector<uint8_t>& code, uint32_t address, Register src, size_t size)
    {
        opAR(code, 0x38, 0x39, address, src, size);
    }
    void cmpAR(std::vector<uint8_t>& code, Register base, int32_t ref, Register src, size_t size)
    {
        opAR(code, 0x38, 0x39, base, ref, src, size);
    }
    void cmpAR(std::vector<uint8_t>& code, Register base, Register index, int scale, int32_t ref, Register src, size_t size)
    {
        opAR(code, 0x38, 0x39, base, index, scale, ref, src, size);
    }
    void cmpAI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size)
    {
        opImmGrp1A(code, ImmGrp1::CMP, dist, imm, size);
    }
    void cmpAI(std::vector<uint8_t>& code, uint32_t address, int32_t imm, size_t size)
    {
        opImmGrp1A(code, ImmGrp1::CMP, address, imm, size);
    }
    void cmpAI(std::vector<uint8_t>& code, Register base, int32_t ref, int32_t imm, size_t size)
    {
        opImmGrp1A(code, ImmGrp1::CMP, base, ref, imm, size);
    }

    void movRR(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        opRR(code, 0x88, 0x89, dist, src, size);
    }
    void movRA(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        opRA(code, 0x8A, 0x89, dist, src, size);
    }
    void movRA(std::vector<uint8_t>& code, Register dist, uint32_t address, size_t size)
    {
        opRA(code, 0x8A, 0x8B, dist, address, size);
    }
    void movRA(std::vector<uint8_t>& code, Register dist, Register base, int32_t ref, size_t size)
    {
        opRA(code, 0x8A, 0x8B, dist, base, ref, size);
    }
    void movRA(std::vector<uint8_t>& code, Register dist, Register base, Register index, int scale, int32_t ref, size_t size)
    {
        
    }
    void movRI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        if (size == 1) assert(imm <= INT8_MAX && imm >= INT8_MIN);
        else if (size == 2) assert(imm <= INT16_MAX && imm >= INT16_MIN);

        if (size == 2) code.push_back(0x66);
        else if (size == 8) {
            if (imm <= INT32_MAX && imm >= INT32_MIN) size = 4;
            else code.push_back(isX86Reg(dist) ? 0x48 : 0x49);
        }

        if (size != 8 && !is8bitReg(dist)) code.push_back(isX86Reg(dist) ? 0x40 : 0x41);

        if (size == 1) {
            code.push_back(0b10'110'000 | modRM(dist, Register::RAX));
            code.push_back(static_cast<uint8_t>(imm));
        }
        else {
            code.push_back(0b10'111'000 | modRM(dist, Register::RAX));
            code.push_back(static_cast<uint8_t>(imm));
            code.push_back(static_cast<uint8_t>(imm >> 8));
            if (size >= 4) {
                code.push_back(static_cast<uint8_t>(imm >> 16));
                code.push_back(static_cast<uint8_t>(imm >> 24));
                if (size == 8) {
                    code.push_back(static_cast<uint8_t>(imm >> 32));
                    code.push_back(static_cast<uint8_t>(imm >> 40));
                    code.push_back(static_cast<uint8_t>(imm >> 48));
                    code.push_back(static_cast<uint8_t>(imm >> 56));
                }
            }
        }
    }
    void movAR(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        opAR(code, 0x88, 0x89, dist, src, size);
    }
    void movAR(std::vector<uint8_t>& code, uint32_t address, Register src, size_t size)
    {
        opAR(code, 0x88, 0x89, address, src, size);
    }
    void movAR(std::vector<uint8_t>& code, Register base, int32_t ref, Register src, size_t size)
    {
        opAR(code, 0x88, 0x89, base, ref, src, size);
    }
    void movAI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        if (size == 1) assert(imm <= INT8_MAX && imm >= INT8_MIN);
        else if (size == 2) assert(imm <= INT16_MAX && imm >= INT16_MIN);

        if (size == 2) code.push_back(0x66);

        if (size == 1) code.push_back(0xC6);
        else code.push_back(0xC7);

        if (dist == Register::RBP || dist == Register::R13) {
            code.push_back(0x45);
            code.push_back(0x00);
        }
        else {
            code.push_back(modRM(dist, Register::RAX));
            if (dist == Register::RSP || dist == Register::R12) {
                code.push_back(0x24);
            }
        }

        code.push_back(imm);
        if (size >= 2) {
            code.push_back(imm >> 8);
            if (size >= 4) {
                code.push_back(imm >> 16);
                code.push_back(imm >> 24);
            }
        }
    }
    void movAI(std::vector<uint8_t>& code, uint32_t address, int32_t imm, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        if (size == 1) assert(imm <= INT8_MAX && imm >= INT8_MIN);
        else if (size == 2) assert(imm <= INT16_MAX && imm >= INT16_MIN);

        if (size == 2) code.push_back(0x66);

        if (size == 1) code.push_back(0xC6);
        else code.push_back(0xC7);

        code.push_back(imm);
        if (size >= 2) {
            code.push_back(imm >> 8);
            if (size >= 4) {
                code.push_back(imm >> 16);
                code.push_back(imm >> 24);
            }
        }
    }
    void movAI(std::vector<uint8_t>& code, Register base, int32_t ref, int32_t imm, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        if (size == 1) assert(imm <= INT8_MAX && imm >= INT8_MIN);
        else if (size == 2) assert(imm <= INT16_MAX && imm >= INT16_MIN);

        if (size == 2) code.push_back(0x66);

        if (size == 1) code.push_back(0xC6);
        else code.push_back(0xC7);

        if (ref <= INT8_MAX && ref >= INT8_MIN) {
            code.push_back(0b01'000'000 | modRM(base, Register::RAX));
            if (base == Register::RSP) code.push_back(0x24);
            code.push_back(ref);
        }
        else {
            code.push_back(0b10'000'000 | modRM(base, Register::RAX));
            if (base == Register::RSP) code.push_back(0x24);
            code.push_back(ref);
            code.push_back(ref >> 8);
            code.push_back(ref >> 16);
            code.push_back(ref >> 24);
        }

        code.push_back(imm);
        if (size >= 2) {
            code.push_back(imm >> 8);
            if (size >= 4) {
                code.push_back(imm >> 16);
                code.push_back(imm >> 24);
            }
        }
    }

    void pushR(std::vector<uint8_t>& code, Register reg)
    {
        if (!isX86Reg(reg)) code.push_back(0x41);
        code.push_back(0b01'010'000 | modRM(reg, Register::RAX));
    }
    void pushA(std::vector<uint8_t>& code, uint32_t address)
    {
        code.push_back(0xFF);
        code.push_back(0x35);
        code.push_back(address);
        code.push_back(address >> 8);
        code.push_back(address >> 16);
        code.push_back(address >> 24);
    }
    void pushA(std::vector<uint8_t>& code, Register base, int32_t ref)
    {
        if (!isX86Reg(base)) code.push_back(0x41);
        code.push_back(0xFF);
        if (ref <= INT8_MAX && ref >= INT8_MIN) {
            code.push_back(0b01'110'000 | modRM(base, Register::RAX));
            code.push_back(ref);
        }
        else {
            code.push_back(0b10'110'000 | modRM(base, Register::RAX));
            code.push_back(ref);
            code.push_back(ref >> 8);
            code.push_back(ref >> 16);
            code.push_back(ref >> 24);
        }
    }
    void pushI(std::vector<uint8_t>& code, int8_t imm)
    {
        code.push_back(0x6A);
        code.push_back(imm);
    }
    void pushI(std::vector<uint8_t>& code, int32_t imm)
    {
        code.push_back(0x68);
        code.push_back(imm);
        code.push_back(imm >> 8);
        code.push_back(imm >> 16);
        code.push_back(imm >> 24);
    }

    void popR(std::vector<uint8_t>& code, Register reg)
    {
        if (!isX86Reg(reg)) code.push_back(0x41);
        code.push_back(0b01'011'000 | modRM(reg, Register::RAX));
    }
    void popA(std::vector<uint8_t>& code, uint32_t address)
    {
        code.push_back(0x8F);
        code.push_back(0x05);
        code.push_back(address);
        code.push_back(address >> 8);
        code.push_back(address >> 16);
        code.push_back(address >> 24);
    }
    void popA(std::vector<uint8_t>& code, Register base, int32_t ref)
    {
        if (!isX86Reg(base)) code.push_back(0x41);
        code.push_back(0x8F);
        if (ref <= INT8_MAX && ref >= INT8_MIN) {
            code.push_back(0b01'000'000 | modRM(base, Register::RAX));
            code.push_back(ref);
        }
        else {
            code.push_back(0b10'000'000 | modRM(base, Register::RAX));
            code.push_back(ref);
            code.push_back(ref >> 8);
            code.push_back(ref >> 16);
            code.push_back(ref >> 24);
        }
    }

    void negR(std::vector<uint8_t>& code, Register reg)
    {
        code.push_back(isX86Reg(reg) ? 0x48 : 0x49);
        code.push_back(0xF7);
        code.push_back(0b11'010'000 | modRM(reg, Register::RAX));
    }
    void negA(std::vector<uint8_t>& code, Register base, int32_t ref)
    {
        code.push_back(isX86Reg(base) ? 0x48 : 0x49);
        code.push_back(0xF7);
        if (ref <= INT8_MAX && ref >= INT8_MIN) {
            code.push_back(0b00'000'101 | modRM(base, Register::RAX));
            code.push_back(ref);
        }
        else {
            code.push_back(0b00'001'001 | modRM(base, Register::RAX));
            code.push_back(ref);
            code.push_back(ref >> 8);
            code.push_back(ref >> 16);
            code.push_back(ref >> 24);
        }
    }

    void callA(std::vector<uint8_t>& code, uint32_t address)
    {
        code.push_back(0xE8);
        code.push_back(address);
        code.push_back(address >> 8);
        code.push_back(address >> 16);
        code.push_back(address >> 24);
    }
    void callR(std::vector<uint8_t>& code, Register reg)
    {
        if (!isX86Reg(reg)) code.push_back(0x41);
        code.push_back(0xFF);
        code.push_back(0b11'010'000 | modRM(reg, Register::RAX));
    }

    void je(std::vector<uint8_t>& code, uint32_t address)
    {
        jcc(code, 0x74, 0x84, address);
    }
    void jne(std::vector<uint8_t>& code, uint32_t address)
    {
        jcc(code, 0x75, 0x85, address);
    }
    void jg(std::vector<uint8_t>& code, uint32_t address)
    {
        jcc(code, 0x7F, 0x8F, address);
    }
    void jge(std::vector<uint8_t>& code, uint32_t address)
    {
        jcc(code, 0x7D, 0x8D, address);
    }
    void jl(std::vector<uint8_t>& code, uint32_t address)
    {
        jcc(code, 0x7C, 0x8C, address);
    }
    void jle(std::vector<uint8_t>& code, uint32_t address)
    {
        jcc(code, 0x7E, 0x8E, address);
    }

    void jcc(std::vector<uint8_t>& code, uint8_t shortCode, uint8_t nearCode, uint32_t address)
    {
        if (address <= INT8_MAX) {
            code.push_back(shortCode);
            code.push_back(address);
        }
        else {
            code.push_back(0x0F);
            code.push_back(nearCode);
            code.push_back(address);
            code.push_back(address >> 8);
            code.push_back(address >> 16);
            code.push_back(address >> 24);
        }
    }

    void jmpF(std::vector<uint8_t>& code, uint32_t address)
    {
        code.push_back(0xFF);
        code.push_back(0x25);
        code.push_back(address);
        code.push_back(address >> 8);
        code.push_back(address >> 16);
        code.push_back(address >> 24);
    }
    void jmpA(std::vector<uint8_t>& code, uint32_t address)
    {
        if (address <= INT8_MAX) {
            code.push_back(0xEB);
            code.push_back(address);
        }
        else {
            code.push_back(0xE9);
            code.push_back(address);
            code.push_back(address >> 8);
            code.push_back(address >> 16);
            code.push_back(address >> 24);
        }
    }
    void jmpR(std::vector<uint8_t>& code, Register reg)
    {
        if (!isX86Reg(reg)) code.push_back(0x41);
        code.push_back(0xFF);
        code.push_back(0b11'100'000 | modRM(reg, Register::RAX));
    }

    void leave(std::vector<uint8_t>& code)
    {
        code.push_back(0xC9);
    }

    void ret(std::vector<uint8_t>& code)
    {
        code.push_back(0xC3);
    }

    void opRR(std::vector<uint8_t>& code, uint8_t lCode, uint8_t eCode, Register dist, Register src, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        if (size == 2) code.push_back(0x66);
        if (size == 8) {
            uint8_t rex = 0x48;
            if (!isX86Reg(dist)) rex |= 0b0001;
            if (!isX86Reg(src)) rex |= 0b0100;
            code.push_back(rex);
        }
        else if (!is8bitReg(dist) || !is8bitReg(src)) {
            uint8_t rex = 0x40;
            if (!isX86Reg(dist)) rex |= 0b0001;
            if (!isX86Reg(src)) rex |= 0b0100;
            code.push_back(rex);
        }
        code.push_back(size == 1 ? lCode : eCode);
        code.push_back(0b11'000'000 | modRM(dist, src));
    }
    void opRA(std::vector<uint8_t>& code, uint8_t lCode, uint8_t eCode, Register dist, Register src, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        if (size == 2) code.push_back(0x66);
        if (size == 8) {
            uint8_t rex = 0x48;
            if (!isX86Reg(dist)) rex |= 0b0001;
            if (!isX86Reg(src)) rex |= 0b0100;
            code.push_back(rex);
        }
        else if (!is8bitReg(dist) || !is8bitReg(src)) {
            uint8_t rex = 0x40;
            if (!isX86Reg(dist)) rex |= 0b0001;
            if (!isX86Reg(src)) rex |= 0b0100;
            code.push_back(rex);
        }
        code.push_back(size == 1 ? lCode : eCode);
        if (src == Register::RBP || src == Register::R13) {
            code.push_back(0b01'000'000 | modRM(src, dist));
            code.push_back(0);
        }
        else {
            code.push_back(0b00'000'000 | modRM(src, dist));
            if (src == Register::RSP || src == Register::R12) {
                code.push_back(0b00'101'000);
            }
        }
    }
    void opRA(std::vector<uint8_t>& code, uint8_t lCode, uint8_t eCode, Register dist, uint32_t address, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        if (size == 2) code.push_back(0x66);
        if (size == 8) code.push_back(isX86Reg(dist) ? 0x48 : 0x4C);
        else if (!is8bitReg(dist)) code.push_back(isX86Reg(dist) ? 0x40 : 0x44);
        code.push_back(size == 1 ? lCode : eCode);
        code.push_back(0b00'000'000 | modRM(Register::RSP, dist));
        code.push_back(0x25);
        code.push_back(address);
        code.push_back(address >> 8);
        code.push_back(address >> 16);
        code.push_back(address >> 24);
    }
    void opRA(std::vector<uint8_t>& code, uint8_t lCode, uint8_t eCode, Register dist, Register base, int32_t ref, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        if (size == 8) {
            uint8_t rex = 0x48;
            if (!isX86Reg(base)) rex |= 0b0001;
            if (!isX86Reg(dist)) rex |= 0b0100;
            code.push_back(rex);
        }
        else if (!is8bitReg(dist) || !is8bitReg(base)) {
            uint8_t rex = 0x40;
            if (!isX86Reg(base)) rex |= 0b0001;
            if (!isX86Reg(dist)) rex |= 0b0100;
            code.push_back(rex);
        }
        code.push_back(size == 1 ? lCode : eCode);
        if (ref <= INT8_MAX && ref >= INT8_MIN) {
            code.push_back(0b01'000'000 | modRM(base, dist));
            if (base == Register::RSP || base == Register::R12) {
                code.push_back(0b00'101'000);
            }
            code.push_back(ref);
        }
        else {
            code.push_back(0b10'000'000 | modRM(base, dist));
            if (base == Register::RSP || base == Register::R12) {
                code.push_back(0b00'101'000);
            }
            code.push_back(ref);
            code.push_back(ref >> 8);
            code.push_back(ref >> 16);
            code.push_back(ref >> 24);
        }
    }
    void opRA(std::vector<uint8_t>& code, uint8_t lCode, uint8_t eCode, Register dist, Register base, Register index, int scale, int32_t ref, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        assert(scale == 1 || scale == 2 || scale == 4 || scale == 8);
        
        if (size == 2) code += 0x66;
        uint8_t rex = 0;
        if (size == 8)          rex |= 0b1000;
        if (!isX86Reg(dist))    rex |= 0b0100;
        if (!isX86Reg(index))   rex |= 0b0010;
        if (!isX86Reg(base))    rex |= 0b0001;
        if (rex != 0) code += 0b0100 | rex;

        code += size == 1 ? lCode : eCode;

        uint8_t sib = modRM(base, index);
        switch (scale) {
        case 1: sib |= 0b00'000'000; break;
        case 2: sib |= 0b01'000'000; break;
        case 4: sib |= 0b10'000'000; break;
        case 8: sib |= 0b11'000'000; break;
        }
        if (ref <= INT8_MAX && ref >= INT8_MIN) {
            code += 0b01'000'000 | modRM(Register::RSP, dist);
            code += sib;
            code += ref;
        }
        else {
            code += 0b10'000'000 | modRM(Register::RSP, dist);
            code += sib;
            code += ref;
            code += ref >> 8;
            code += ref >> 16;
            code += ref >> 24;
        }
    }
    void opAR(std::vector<uint8_t>& code, uint8_t lCode, uint8_t eCode, Register dist, Register src, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        if (size == 8) {
            uint8_t rex = 0x48;
            if (!isX86Reg(dist)) rex |= 0b0001;
            if (!isX86Reg(src)) rex |= 0b0100;
            code.push_back(rex);
        }
        else if (!is8bitReg(dist) || !is8bitReg(src)) {
            uint8_t rex = 0x40;
            if (!isX86Reg(dist)) rex |= 0b0001;
            if (!isX86Reg(src)) rex |= 0b0100;
            code.push_back(rex);
        }
        code.push_back(size == 1 ? lCode : eCode);
        if (src == Register::RBP || src == Register::R13) {
            code.push_back(0b01'000'000 | modRM(dist, src));
            code.push_back(0);
        }
        else {
            code.push_back(0b00'000'000 | modRM(dist, src));
            if (src == Register::RSP || src == Register::R12) {
                code.push_back(0b00'101'000);
            }
        }
    }
    void opAR(std::vector<uint8_t>& code, uint8_t lCode, uint8_t eCode, uint32_t address, Register src, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        if (size == 2) code.push_back(0x66);
        if (size == 8) code.push_back(isX86Reg(src) ? 0x48 : 0x4C);
        else if (!is8bitReg(src)) code.push_back(isX86Reg(src) ? 0x40 : 0x44);
        code.push_back(size == 1 ? lCode : eCode);
        code.push_back(0b00'000'000 | modRM(Register::RSP, src));
        code.push_back(0x25);
        code.push_back(address);
        code.push_back(address >> 8);
        code.push_back(address >> 16);
        code.push_back(address >> 24);
    }
    void opAR(std::vector<uint8_t>& code, uint8_t lCode, uint8_t eCode, Register base, int32_t ref, Register src, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        if (size == 2) code.push_back(0x66);
        if (size == 8) {
            uint8_t rex = 0x48;
            if (!isX86Reg(base)) rex |= 0b0001;
            if (!isX86Reg(src)) rex |= 0b0100;
            code.push_back(rex);
        }
        else if (!is8bitReg(base) || !is8bitReg(src)) {
            uint8_t rex = 0x40;
            if (!isX86Reg(base)) rex |= 0b0001;
            if (!isX86Reg(src)) rex |= 0b0100;
            code.push_back(rex);
        }
        code.push_back(size == 1 ? lCode : eCode);
        if (ref <= INT8_MAX && ref >= INT8_MIN) {
            code.push_back(0b01'000'000 | modRM(base, src));
            if (base == Register::RSP || base == Register::R12) {
                code.push_back(0b00'101'000);
            }
            code.push_back(ref);
        }
        else {
            code.push_back(0b10'000'000 | modRM(base, src));
            if (base == Register::RSP || base == Register::R12) {
                code.push_back(0b00'101'000);
            }
            code.push_back(ref);
            code.push_back(ref >> 8);
            code.push_back(ref >> 16);
            code.push_back(ref >> 24);
        }
    }
    void opAR(std::vector<uint8_t>& code, uint8_t lCode, uint8_t eCode, Register base, Register index, int scale, int32_t ref, Register src, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        assert(scale == 1 || scale == 2 || scale == 4 || scale == 8);

        if (size == 2) code += 0x66;
        uint8_t rex = 0;
        if (size == 8)          rex |= 0b1000;
        if (!isX86Reg(src))     rex |= 0b0100;
        if (!isX86Reg(index))   rex |= 0b0010;
        if (!isX86Reg(base))    rex |= 0b0001;
        if (rex != 0) code += 0b0100 | rex;

        code += size == 1 ? lCode : eCode;

        uint8_t sib = modRM(base, index);
        switch (scale) {
        case 1: sib |= 0b00'000'000; break;
        case 2: sib |= 0b01'000'000; break;
        case 4: sib |= 0b10'000'000; break;
        case 8: sib |= 0b11'000'000; break;
        }
        if (ref <= INT8_MAX && ref >= INT8_MIN) {
            code += 0b01'000'000 | modRM(Register::RSP, src);
            code += sib;
            code += ref;
        }
        else {
            code += 0b10'000'000 | modRM(Register::RSP, src);
            code += sib;
            code += ref;
            code += ref >> 8;
            code += ref >> 16;
            code += ref >> 24;
        }
    }
    void opImmGrp1R(std::vector<uint8_t>& code, ImmGrp1 immGrp, Register dist, int32_t imm, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        if (size == 1) assert(imm <= INT8_MAX && imm >= INT8_MIN);
        else if (size == 2) assert(imm <= INT16_MAX && imm >= INT16_MIN);

        if (size == 2) code.push_back(0x66);
        if (size == 8) code.push_back(isX86Reg(dist) ? 0x48 : 0x49);
        else if (!isX86Reg(dist)) code.push_back(0x41);

        if (size == 1) {
            if (dist == Register::RAX) {
                switch (immGrp) {
                case ImmGrp1::ADD: code.push_back(0x04); break;
                case ImmGrp1::SUB: code.push_back(0x2C); break;
                case ImmGrp1::CMP: code.push_back(0x3C); break;
                default: assert(false);
                }
            }
            else code.push_back(0x80);
        }
        else if (imm <= INT8_MAX && imm >= INT8_MIN) code.push_back(0x83);
        else {
            if (dist == Register::RAX) {
                switch (immGrp) {
                case ImmGrp1::ADD: code.push_back(0x05); break;
                case ImmGrp1::SUB: code.push_back(0x2D); break;
                case ImmGrp1::CMP: code.push_back(0x3D); break;
                default: assert(false);
                }
            }
            else code.push_back(0x81);
        }
        code.push_back(0b11'000'000 | immGrp1(immGrp) | modRM(dist, Register::RAX));

        code.push_back(uint8_t(imm & 0xFF));
        if (!(imm <= INT8_MAX && imm >= INT8_MIN)) {
            code.push_back(uint8_t((imm >> 8) & 0xFF));
            if (size >= 4) {
                code.push_back(uint8_t((imm >> 16) & 0xFF));
                code.push_back(uint8_t((imm >> 24) & 0xFF));
            }
        }
    }
    void opImmGrp1A(std::vector<uint8_t>& code, ImmGrp1 immGrp, Register dist, int32_t imm, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        if (size == 1) assert(imm <= INT8_MAX && imm >= INT8_MIN);
        else if (size == 2) assert(imm <= INT16_MAX && imm >= INT16_MIN);

        if (size == 2) code.push_back(0x66);
        if (size == 8) code.push_back(isX86Reg(dist) ? 0x48 : 0x49);
        else if (!isX86Reg(dist)) code.push_back(0x41);

        if (size == 1) code.push_back(0x80);
        else if (imm <= INT8_MAX && imm >= INT8_MIN) code.push_back(0x83);
        else code.push_back(0x81);

        if (dist == Register::RBP || dist == Register::R13) {
            code.push_back(0x45);
            code.push_back(0x00);
        }
        else code.push_back(0b00'000'000 | immGrp1(immGrp) | modRM(dist, Register::RAX));

        code.push_back(uint8_t(imm & 0xFF));
        if (!(imm <= INT8_MAX && imm >= INT8_MIN)) {
            code.push_back(uint8_t((imm >> 8) & 0xFF));
            if (size >= 4) {
                code.push_back(uint8_t((imm >> 16) & 0xFF));
                code.push_back(uint8_t((imm >> 24) & 0xFF));
            }
        }
    }
    void opImmGrp1A(std::vector<uint8_t>& code, ImmGrp1 immGrp, uint32_t address, int32_t imm, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        if (size == 1) assert(imm <= INT8_MAX && imm >= INT8_MIN);
        else if (size == 2) assert(imm <= INT16_MAX && imm >= INT16_MIN);

        if (size == 2) code.push_back(0x66);

        if (size == 1) code.push_back(0x80);
        else if (imm <= INT8_MAX && imm >= INT8_MIN) code.push_back(0x83);
        else code.push_back(0x81);

        code.push_back(immGrp1(immGrp) | 0x04);
        code.push_back(0x25);

        code.push_back(address);
        code.push_back(address >> 8);
        code.push_back(address >> 16);
        code.push_back(address >> 24);

        if (imm <= INT8_MAX && imm >= INT8_MIN) {
            code.push_back(uint8_t(imm));
        }
        else {
            code.push_back(uint32_t(imm));
            code.push_back(uint32_t(imm) >> 8);
            code.push_back(uint32_t(imm) >> 16);
            code.push_back(uint32_t(imm) >> 24);
        }
    }
    void opImmGrp1A(std::vector<uint8_t>& code, ImmGrp1 immGrp, Register base, int32_t ref, int32_t imm, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        if (size == 1) assert(imm <= INT8_MAX && imm >= INT8_MIN);
        else if (size == 2) assert(imm <= INT16_MAX && imm >= INT16_MIN);

        if (size == 2) code.push_back(0x66);
        if (size == 1) code.push_back(0x80);
        else if (imm <= INT8_MAX && imm >= INT8_MIN) code.push_back(0x83);
        else code.push_back(0x81);

        if (ref <= INT8_MAX && ref >= INT8_MIN) {
            code.push_back(0b10'000'000 | immGrp1(immGrp) | modRM(base, Register::RAX));
            code.push_back(ref);
        }
        else {
            code.push_back(0b01'000'000 | immGrp1(immGrp) | modRM(base, Register::RAX));
            code.push_back(ref);
            code.push_back(ref >> 8);
            code.push_back(ref >> 16);
            code.push_back(ref >> 24);
        }
        if (imm <= INT8_MAX && imm >= INT8_MIN) {
            code.push_back(imm);
        }
        else {
            code.push_back(imm);
            code.push_back(imm >> 8);
            code.push_back(imm >> 16);
            code.push_back(imm >> 24);
        }
    }

    void xidivRR(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);

        movRI(code, Register::RDX, 0, 8);
        if (dist != Register::RAX) movRR(code, Register::RAX, dist, size);

        if (size == 2) code += 0x66;
        uint8_t rex = 0;
        if (size == 8) rex |= 0b1000;
        if (!isX86Reg(src)) rex |= 0b0001;
        if (rex != 0) code += 0b0100'0000 | rex;

        code += size == 1 ? 0xF6 : 0xF7;
        code += 0b11'111'000 | modRM(src, Register::RAX);
    }
    void xidivRA(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);

        movRI(code, Register::RDX, 0, 8);
        if (dist != Register::RAX) movRR(code, Register::RAX, dist, size);

        if (size == 2) code += 0x66;
        uint8_t rex = 0;
        if (size == 8) rex |= 0b1000;
        if (!isX86Reg(src)) rex |= 0b0001;
        if (rex != 0) code += 0b0100'0000 | rex;

        code += size == 1 ? 0xF6 : 0xF7;
        code += 0b00'111'000 | modRM(src, Register::RAX);
    }
    void xidivRA(std::vector<uint8_t>& code, Register dist, uint32_t address, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);

        movRI(code, Register::RDX, 0, 8);
        if (dist != Register::RAX) movRR(code, Register::RAX, dist, size);

        if (size == 2) code += 0x66;
        uint8_t rex = 0;
        if (size == 8) rex |= 0b1000;
        if (rex != 0) code += 0b0100'0000 | rex;

        code += size == 1 ? 0xF6 : 0xF7;
        code += 0b00'111'100;
        code += address;
        code += address >> 8;
        code += address >> 16;
        code += address >> 24;
    }
    void xidivRA(std::vector<uint8_t>& code, Register dist, Register base, int32_t ref, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);

        movRI(code, Register::RDX, 0, 8);
        if (dist != Register::RAX) movRR(code, Register::RAX, dist, size);

        if (size == 2) code += 0x66;
        uint8_t rex = 0;
        if (size == 8) rex |= 0b1000;
        if (!isX86Reg(base)) rex |= 0b0001;
        if (rex != 0) code += 0b0100'0000 | rex;

        code += size == 1 ? 0xF6 : 0xF7;
        if (ref <= INT8_MAX && ref >= INT8_MIN) {
            code += 0b01'111'000 | modRM(base, Register::RAX);
            code += ref;
        }
        else {
            code += 0b10'111'000 | modRM(base, Register::RAX);
            code += ref;
            code += ref >> 8;
            code += ref >> 16;
            code += ref >> 24;
        }
    }
    void xidivRI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);

        movRI(code, Register::RAX, imm, size);
        idivRR(code, dist, Register::RAX, size);
    }
    void xidivAR(std::vector<uint8_t>& code, Register dist, Register src, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);

        movRI(code, Register::RDX, 0, 8);
        if (dist != Register::RAX) movRA(code, Register::RAX, dist, size);

        if (size == 2) code += 0x66;
        uint8_t rex = 0;
        if (size == 8) rex |= 0b1000;
        if (!isX86Reg(src)) rex |= 0b0001;
        if (rex != 0) code += 0b0100'0000 | rex;

        code += size == 1 ? 0xF6 : 0xF7;
        code += 0b00'111'000 | modRM(src, Register::RAX);
    }
    void xidivAR(std::vector<uint8_t>& code, uint32_t address, Register src, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);

        movRI(code, Register::RDX, 0, 8);
        movRA(code, Register::RAX, address, size);

        if (size == 2) code += 0x66;
        uint8_t rex = 0;
        if (size == 8) rex |= 0b1000;
        if (!isX86Reg(src)) rex |= 0b0001;
        if (rex != 0) code += 0b0100'0000 | rex;

        code += size == 1 ? 0xF6 : 0xF7;
        code += 0b00'111'000 | modRM(src, Register::RAX);
    }
    void xidivAR(std::vector<uint8_t>& code, Register base, int32_t ref, Register src, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);

        movRI(code, Register::RDX, 0, 8);
        movRA(code, Register::RAX, base, ref, size);

        if (size == 2) code += 0x66;
        uint8_t rex = 0;
        if (size == 8) rex |= 0b1000;
        if (!isX86Reg(src)) rex |= 0b0001;
        if (rex != 0) code += 0b0100'0000 | rex;

        code += size == 1 ? 0xF6 : 0xF7;
        code += 0b00'111'000 | modRM(src, Register::RAX);
    }
    void xidivAI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);

        movRA(code, Register::RAX, dist, size);
        idivRI(code, Register::RAX, imm, size);
    }
    void xidivAI(std::vector<uint8_t>& code, uint32_t address, int32_t imm, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);

        movRA(code, Register::RAX, address, size);
        idivRI(code, Register::RAX, imm, size);
    }
    void xidivAI(std::vector<uint8_t>& code, Register base, int32_t ref, int32_t imm, size_t size)
    {
        assert(size == 1 || size == 2 || size == 4 || size == 8);

        movRA(code, Register::RAX, base, ref, size);
        idivRI(code, Register::RAX, imm, size);
    }
    
    uint8_t immGrp1(ImmGrp1 immGrp)
    {
        switch (immGrp) {
        case ImmGrp1::ADD: return 0b000'000;
        case ImmGrp1::OR: return 0b001'000;
        case ImmGrp1::ADC: return 0b010'000;
        case ImmGrp1::SBB: return 0b011'000;
        case ImmGrp1::AND: return 0b100'000;
        case ImmGrp1::SUB: return 0b101'000;
        case ImmGrp1::XOR: return 0b110'000;
        case ImmGrp1::CMP: return 0b111'000;
        default: assert(false);
        }
    }
    bool isX86Reg(Register reg)
    {
        return
            reg == Register::RAX ||
            reg == Register::RCX ||
            reg == Register::RDX ||
            reg == Register::RBX ||
            reg == Register::RSP ||
            reg == Register::RBP ||
            reg == Register::RSI ||
            reg == Register::RDI;
    }
    bool is8bitReg(Register reg)
    {
        return
            reg == Register::RAX ||
            reg == Register::RCX ||
            reg == Register::RBX ||
            reg == Register::RDX;
    }
    ConstantSymbol::ConstantSymbol(const std::string& name, const std::vector<uint8_t>& imm) : name(name), type(ConstantSymbolType::Immediate), imm(imm), address(0)
    {
    }
#pragma warning(disable: 26495)
    ConstantSymbol::ConstantSymbol(const std::string& name, const Relocation& reloc) : name(name), type(ConstantSymbolType::Relocation), reloc(reloc), address(0)
    {
    }
#pragma warning(disable: 26495)
    ConstantSymbol::ConstantSymbol(const ConstantSymbol& con) : name(con.name), type(con.type), address(0)
    {
        switch (type) {
        case ConstantSymbolType::_NONE:
            break;
        case ConstantSymbolType::Immediate:
            new (&imm) std::vector(con.imm);
            break;
        case ConstantSymbolType::Relocation:
            new (&reloc) Relocation(con.reloc);
            break;
        default:
            assert(false);
        }
    }
    ConstantSymbol::~ConstantSymbol()
    {
        clear();
    }
    ConstantSymbol& ConstantSymbol::operator=(const ConstantSymbol& con)
    {
        clear();
        name = con.name;
        type = con.type;
        switch (type) {
        case ConstantSymbolType::_NONE:
            break;
        case ConstantSymbolType::Immediate:
            new (&imm) std::vector(con.imm);
            break;
        case ConstantSymbolType::Relocation:
            new (&reloc) Relocation(con.reloc);
            break;
        default:
            assert(false);
        }
        return *this;
    }
    void ConstantSymbol::clear()
    {
        switch (type) {
        case ConstantSymbolType::_NONE:
            break;
        case ConstantSymbolType::Immediate:
            imm.~vector();
            break;
        case ConstantSymbolType::Relocation:
            reloc.~Relocation();
            break;
        default:
            assert(false);
        }
        type = ConstantSymbolType::_NONE;
    }

    uint8_t modRM(Register dist, Register src)
    {
        uint8_t modRM = 0;
        switch (src) {
        case Register::RAX:
        case Register::R8:
            modRM |= 0b00'000'000;
            break;
        case Register::RCX:
        case Register::R9:
            modRM |= 0b00'001'000;
            break;
        case Register::RDX:
        case Register::R10:
            modRM |= 0b00'010'000;
            break;
        case Register::RBX:
        case Register::R11:
            modRM |= 0b00'011'000;
            break;
        case Register::RSP:
        case Register::R12:
            modRM |= 0b00'100'000;
            break;
        case Register::RBP:
        case Register::R13:
            modRM |= 0b00'101'000;
            break;
        case Register::RSI:
        case Register::R14:
            modRM |= 0b00'110'000;
            break;
        case Register::RDI:
        case Register::R15:
            modRM |= 0b00'111'000;
            break;
        default: assert(false);
        }
        switch (dist) {
        case Register::RAX:
        case Register::R8:
            modRM |= 0b00'000'000;
            break;
        case Register::RCX:
        case Register::R9:
            modRM |= 0b00'000'001;
            break;
        case Register::RDX:
        case Register::R10:
            modRM |= 0b00'000'010;
            break;
        case Register::RBX:
        case Register::R11:
            modRM |= 0b00'000'011;
            break;
        case Register::RSP:
        case Register::R12:
            modRM |= 0b00'000'100;
            break;
        case Register::RBP:
        case Register::R13:
            modRM |= 0b00'000'101;
            break;
        case Register::RSI:
        case Register::R14:
            modRM |= 0b00'000'110;
            break;
        case Register::RDI:
        case Register::R15:
            modRM |= 0b00'000'111;
            break;
        default: assert(false);
        }
        return modRM;
    }
    Operation Operation::r(Opecode opecode, Register reg)
    {
        Operation op{};
        op.opecode = opecode;
        op.op1.type = RMType::Register;
        op.op1.reg = reg;
        return op;
    }
    Operation Operation::a(Opecode opecode, Register reg)
    {
        Operation op{};
        op.op1.type = RMType::Memory;
        op.op1.mem.base = reg;
        return op;
    }
    Operation Operation::a(Opecode opecode, Register base, int32_t ref)
    {
        Operation op{};
        op.op1.type = RMType::Memory;
        op.op1.mem.base = base;
        op.op1.mem.disp = ref;
        return op;
    }
    Operation Operation::i(Opecode opecode, int32_t imm)
    {
        Operation op{};
        op.opecode = opecode;
        op.imm = imm;
        return op;
    }
    Operation Operation::rr(Opecode opecode, Register dist, Register src, size_t size)
    {
        if (size == 0) return Operation{ Opecode::_NONE };
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        Operation op{};
        op.opecode = opecode;
        op.size = size;
        op.op1.type = RMType::Register;
        op.op1.reg = dist;
        op.op2 = RM{};
        op.op2->type = RMType::Register;
        op.op2->reg = src;
        return op;
    }
    Operation Operation::ra(Opecode opecode, Register dist, Register src, size_t size)
    {
        if (size == 0) return Operation{ Opecode::_NONE };
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        Operation op{};
        op.opecode = opecode;
        op.size = size;
        op.op1.type = RMType::Register;
        op.op1.reg = dist;
        op.op2 = RM{};
        op.op2->type = RMType::Memory;
        op.op2->mem = Memory{};
        op.op2->mem.base = src;
        return op;
    }
    Operation Operation::ra(Opecode opecode, Register dist, RMType type, uint64_t address, size_t size)
    {
        if (size == 0) return Operation{ Opecode::_NONE };
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        assert(type == RMType::Constant || type == RMType::Runtime);
        Operation op{};
        op.opecode = opecode;
        op.size = size;
        op.op1.type = RMType::Register;
        op.op1.reg = dist;
        op.op2 = RM{};
        op.op2->type = type;
        op.op2->address = address;
        return op;
    }
    Operation Operation::ra(Opecode opecode, Register dist, Register base, int32_t ref, size_t size)
    {
        if (size == 0) return Operation{ Opecode::_NONE };
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        Operation op{};
        op.opecode = opecode;
        op.size = size;
        op.op1.type = RMType::Register;
        op.op1.reg = dist;
        op.op2 = RM{};
        op.op2->type = RMType::Memory;
        op.op2->mem = Memory{};
        op.op2->mem.base = base;
        op.op2->mem.disp = ref;
        return op;
    }
    Operation Operation::ra(Opecode opecode, Register dist, Register base, Register index, int scale, int32_t ref, size_t size)
    {
        if (size == 0) return Operation{ Opecode::_NONE };
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        Operation op{};
        op.opecode = opecode;
        op.size = size;
        op.op1.type = RMType::Register;
        op.op1.reg = dist;
        op.op2 = RM{};
        op.op2->type = RMType::Memory;
        op.op2->mem = Memory{};
        op.op2->mem.base = base;
        op.op2->mem.index = index;
        op.op2->mem.scale = scale;
        op.op2->mem.disp = ref;
        return op;
    }
    Operation Operation::ri(Opecode opecode, Register dist, int32_t imm, size_t size)
    {
        if (size == 0) return Operation{ Opecode::_NONE };
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        Operation op{};
        op.opecode = opecode;
        op.size = size;
        op.op1.type = RMType::Register;
        op.op1.reg = dist;
        op.imm = imm;
        return op;
    }
    Operation Operation::ar(Opecode opecode, Register dist, Register src, size_t size)
    {
        if (size == 0) return Operation{ Opecode::_NONE };
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        Operation op{};
        op.opecode = opecode;
        op.size = size;
        op.op1.type = RMType::Memory;
        op.op1.mem = Memory{};
        op.op1.mem.base = dist;
        op.op2 = RM{};
        op.op2->type = RMType::Register;
        op.op2->reg = src;
        return op;
    }
    Operation Operation::ar(Opecode opecode, RMType type, uint64_t address, Register src, size_t size)
    {
        Operation op{};
        op.opecode = opecode;
        op.op1.type = type;
        op.op1.address = address;
        op.op2 = RM{};
        op.op2.value().type = RMType::Register;
        op.op2.value().reg = src;
        op.size = size;
        return op;
    }
    Operation Operation::ar(Opecode opecode, Register base, int32_t ref, Register src, size_t size)
    {
        if (size == 0) return Operation{ Opecode::_NONE };
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        Operation op{};
        op.opecode = opecode;
        op.size = size;
        op.op1.type = RMType::Memory;
        op.op1.mem = Memory{};
        op.op1.mem.base = base;
        op.op1.mem.disp = ref;
        op.op2 = RM{};
        op.op2->type = RMType::Register;
        op.op2->reg = src;
        return op;
    }
    Operation Operation::ar(Opecode opecode, Register base, Register index, int scale, int32_t ref, Register src, size_t size)
    {
        if (size == 0) return Operation{ Opecode::_NONE };
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        Operation op{};
        op.opecode = opecode;
        op.size = size;
        op.op1.type = RMType::Memory;
        op.op1.mem = Memory{};
        op.op1.mem.base = base;
        op.op1.mem.index = index;
        op.op1.mem.scale = scale;
        op.op1.mem.disp = ref;
        op.op2 = RM{};
        op.op2->type = RMType::Register;
        op.op2->reg = src;
        return op;
    }
    Operation Operation::ai(Opecode opecode, Register dist, int32_t imm, size_t size)
    {
        if (size == 0) return Operation{ Opecode::_NONE };
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        Operation op{};
        op.opecode = opecode;
        op.size = size;
        op.op1.type = RMType::Memory;
        op.op1.mem = Memory{};
        op.op1.mem.base = dist;
        op.imm = imm;
        return op;
    }
    Operation Operation::ai(Opecode opecode, Register base, int32_t ref, int32_t imm, size_t size)
    {
        if (size == 0) return Operation{ Opecode::_NONE };
        assert(size == 1 || size == 2 || size == 4 || size == 8);
        Operation op{};
        op.opecode = opecode;
        op.size = size;
        op.op1.type = RMType::Memory;
        op.op1.mem = Memory{};
        op.op1.mem.base = base;
        op.op1.mem.disp = ref;
        op.imm = imm;
        return op;
    }
    Operation Operation::jcc(Opecode opecode, size_t indexOfBlock)
    {
        Operation op{};
        op.opecode = opecode;
        op.op1.type = RMType::Address;
        op.op1.address = indexOfBlock;
        return op;
    }
    Operation Operation::call(size_t indexOfFunction)
    {
        Operation op{};
        op.opecode = Opecode::CALL;
        op.op1.type = RMType::Address;
        op.op1.address = indexOfFunction;
        return op;
    }
    Operation Operation::addressof(Register dist, int32_t ref)
    {
        Operation op{};
        op.opecode = Opecode::ADDRESS_OF;
        op.op1.type = RMType::Register;
        op.op1.reg = dist;
        op.op2 = RM{};
        op.op2.value().type = RMType::Address;
        op.op2.value().address = ref;
        return op;
    }
    Operation Operation::ret()
    {
        Operation op{};
        op.opecode = Opecode::RET;
        return op;
    }
}