#include "routine_compiler.h"

#include <algorithm>
#include <cmath>
#include <set>

#include "utils/vector_utils.h"
#include "utils/instanceof.h"

#include "pcir/pcir_code.h"
#include "pcir/pcir_format.h"

namespace pickc::windows::x64
{
  RoutineCompiler::RoutineCompiler(const ssa::SSAFunction& ssaFn) : routine(new Routine{ ssaFn }) {};
  Result<Routine*, std::vector<std::string>> RoutineCompiler::compile()
  {
    std::vector<std::string> errors;

    for(size_t i = 0, l = routine->ssaFn.type->type.fn.args.size(); i < l; ++i) {
      if(i < 4) {
        regInfo[*(argRegs.begin() + i)] = RegisterInfo::InUse;
        args.push_back(Operand(*(argRegs.begin() + i)));
      }
      else {
        args.push_back(Operand(Memory(Register::RBP, (routine->ssaFn.type->type.fn.args.size() - i) * 8, false)));
      }
    }

    prologue.push_back(new PushOperation(Operand(Register::RBP)));
    prologue.push_back(new MovOperation(OperationSize::QWord, Operand(Register::RBP), Operand(Register::RSP)));

    std::set<size_t> insertEpilogue;

    curLifeTime = 0;
    stack = 0;
    minStack = 0;
    for(size_t l = routine->ssaFn.insts.size(); curLifeTime < l; ++curLifeTime) {
      using namespace ssa;
      auto inst = routine->ssaFn.insts[curLifeTime];
      if(instanceof<SSAAddInstruction>(inst)) {
        auto add = dynamic_cast<SSAAddInstruction*>(inst);
        assert(regs.find(add->dist) == regs.end());
        assert(regs.find(add->left) != regs.end());
        assert(regs.find(add->right) != regs.end());
        if(regs[add->left].type == OperandType::Immediate && regs[add->right].type == OperandType::Immediate) {
          regs[add->dist] = Operand(regs[add->left].imm + regs[add->right].imm);
        }
        else {
          regs[add->dist] = createOperand();
          if(regs[add->dist].type == OperandType::Memory && (regs[add->left].type == OperandType::Memory || regs[add->right].type == OperandType::Memory)) {
            body.push_back(new MovOperation(getSize(add->dist->type), Operand(Register::RAX), regs[add->left]));
            body.push_back(new AddOperation(getSize(add->dist->type), Operand(Register::RAX), regs[add->right]));
            body.push_back(new MovOperation(getSize(add->dist->type), regs[add->dist], Operand(Register::RAX)));
          }
          else {
            body.push_back(new MovOperation(getSize(add->dist->type), regs[add->dist], regs[add->left]));
            body.push_back(new AddOperation(getSize(add->dist->type), regs[add->dist], regs[add->right]));
          }
        }
      }
      else if(instanceof<SSAImmInstruction>(inst)) {
        auto imm = dynamic_cast<SSAImmInstruction*>(inst);
        assert(regs.find(imm->dist) == regs.end());
        if(imm->imm <= INT32_MAX && imm->imm >= INT32_MIN) {
          regs[imm->dist] = Operand(imm->imm);
        }
        else {
          regs[imm->dist] = createOperand();
          body.push_back(new MovOperation(OperationSize::QWord, regs[imm->dist], Operand(imm->imm)));
        }
      }
      else if(instanceof<SSARetInstruction>(inst)) {
        auto ret = dynamic_cast<SSARetInstruction*>(inst);
        assert(regs.find(ret->value) != regs.end());
        // TODO
        body.push_back(new MovOperation(getSize(ret->value->type), Operand(Register::RAX), regs[ret->value]));
        insertEpilogue.insert(body.size());
      }
      else if(instanceof<SSALoadFnInstruction>(inst)) {
        auto loadFn = dynamic_cast<SSALoadFnInstruction*>(inst);
        assert(regs.find(loadFn->dist) == regs.end());
        regs[loadFn->dist] = Operand(Relocation(loadFn->fn));
      }
      else if(instanceof<SSALoadArgInstruction>(inst)) {
        // 引数は関数の実行中に移動する可能性があるのでLoadFnとは異なり、値をコピーする。
        auto loadArg = dynamic_cast<SSALoadArgInstruction*>(inst);
        assert(regs.find(loadArg->dist) == regs.end());
        regs[loadArg->dist] = createOperand();
        if(regs[loadArg->dist].type == OperandType::Memory && args[loadArg->indexOfArg].type == OperandType::Memory) {
          body.push_back(new MovOperation(getSize(loadArg->dist->type), Operand(Register::RAX), args[loadArg->indexOfArg]));
          body.push_back(new MovOperation(getSize(loadArg->dist->type), regs[loadArg->dist], Operand(Register::RAX)));
        }
        else {
          body.push_back(new MovOperation(getSize(loadArg->dist->type), regs[loadArg->dist], args[loadArg->indexOfArg]));
        }
      }
      else if(instanceof<SSACallInstruction>(inst)) {
        auto call = dynamic_cast<SSACallInstruction*>(inst);
        assert(regs.find(call->dist) == regs.end());
        assert(regs.find(call->call) != regs.end());
        #ifndef NDEBUG
        for(const auto& arg : call->args) {
          assert(regs.find(arg) != regs.end());
        }
        #endif // NDEBUG
        saveRegs();
        auto shadowStore = std::min(call->args.size(), 4ull) * 8;
        if(shadowStore) {
          body.push_back(new SubOperation(OperationSize::QWord, Operand(Register::RSP), Operand(shadowStore)));
        }
        for(size_t arg = 0, argL = call->args.size(); arg < argL; ++arg) {
          if(arg < 4) {
            body.push_back(new MovOperation(OperationSize::QWord, Operand(*(argRegs.begin() + arg)), regs[call->args[arg]]));
          }
          else {
            body.push_back(new PushOperation(regs[call->args[arg]]));
          }
        }
        body.push_back(new CallOperation(routine, regs[call->call]));
        // 引数の巻き戻し
        if(!call->args.empty()) {
          body.push_back(new AddOperation(OperationSize::QWord, Operand(Register::RSP), Operand(call->args.size() * 8)));
        }
        // 戻り値の取得
        if(call->dist) {
          regs[call->dist] = createOperand();
          body.push_back(new MovOperation(OperationSize::QWord, regs[call->dist], Operand(Register::RAX)));
        }
      }
      else if(instanceof<SSAMovInstruction>(inst)) {
        auto mov = dynamic_cast<SSAMovInstruction*>(inst);
        if(regs[mov->dist].type == OperandType::Immediate) {
          regs[mov->dist] = Operand(regs[mov->dist].imm);
        }
        else {
          regs[mov->dist] = createOperand();
          if(regs[mov->dist].type == OperandType::Memory && regs[mov->src].type == OperandType::Memory) {
            body.push_back(new MovOperation(getSize(mov->dist->type), Operand(Register::RAX), regs[mov->src]));
            body.push_back(new MovOperation(getSize(mov->dist->type), regs[mov->dist], Operand(Register::RAX)));
          }
          else {
            body.push_back(new MovOperation(getSize(mov->dist->type), regs[mov->dist], regs[mov->src]));
          }
        }
      }
      else {
        assert(false);
      }
    }

    std::vector<Operation*> saveNonvolatileRegs;

    for(auto reg : nonvolatileRegs) {
      if(regInfo[reg] != RegisterInfo::Unused) {
        routine->baseDiff -= 8;
        saveNonvolatileRegs.push_back(new MovOperation(OperationSize::QWord, Operand(Memory(Register::RBP, routine->baseDiff, false)), Operand(reg)));
        epilogue.push_back(new MovOperation(OperationSize::QWord, Operand(reg), Operand(Memory(Register::RBP, routine->baseDiff, false))));
      }
    }

    if(routine->baseDiff != 0 || minStack != 0) {
      auto alloc = routine->baseDiff + minStack;
      alloc = (alloc - 15) / 16 * 16;
      prologue.push_back(new SubOperation(OperationSize::QWord, Operand(Register::RSP), Operand(-alloc)));
    }

    epilogue.push_back(new LeaveOperation());
    epilogue.push_back(new RetOperation());

    routine->code += prologue;
    size_t curBodyIndex = 0;
    for(auto insert : insertEpilogue) {
      routine->code.reserve(routine->code.size() + insert - curBodyIndex);
      routine->code.insert(routine->code.end(), body.begin() + curBodyIndex, body.begin() + insert);
      routine->code += epilogue;
      curBodyIndex = insert;
    }
    if(curBodyIndex != body.size()) {
      routine->code += epilogue;
    }
    
    if(errors.empty()) return ok(routine);
    return error(errors);
  }
  
  Operand RoutineCompiler::createOperand()
  {
    freeRegs();
    for(auto reg : useRegs) {
      if(regInfo[reg] != RegisterInfo::InUse) {
        regInfo[reg] = RegisterInfo::InUse;
        return Operand(reg);
      }
    }
    return Operand(allocStack(8));
  }
  Memory RoutineCompiler::allocStack(size_t numBytes)
  {
    assert(numBytes != 0);
    int cur = 0;
    for(size_t i = 0, l = stackStatus.size(); i < l; ++i) {
      if(!stackStatus[i]) {
        ++cur;
        if(cur >= numBytes) {
          for(size_t j = 0; j < cur; ++j) {
            stackStatus[i - j] = true;
          }
          return Memory(Register::RBP, -cur, true);
        }
      }
      else {
        cur = 0;
      }
    }
    stack -= numBytes;
    minStack = std::min(stack, minStack);
    return Memory(Register::RBP, stack, true);
  }
  void RoutineCompiler::saveRegs()
  {
    freeRegs();
    for(size_t i = 0, l = std::min(args.size(), 4ull); i < l; ++i) {
      if(args[i].type == OperandType::Register) {
        auto reg = args[i].reg;
        regInfo[reg] = RegisterInfo::Used;
        args[i] = Operand(Memory(Register::RBP, (l - i) * 8, true));
        body.push_back(new MovOperation(OperationSize::QWord, args[i], Operand(reg)));
      }
    }
    for(auto reg : volatileRegs) {
      if(regInfo[reg] == RegisterInfo::InUse) {
        auto mem = Operand(allocStack(8));
        body.push_back(new MovOperation(OperationSize::QWord, mem, Operand(reg)));
        for(auto& operand : regs) {
          if(operand.second.type == OperandType::Register && operand.second.reg == reg) {
            operand.second = mem;
            break;
          }
        }
      }
    }
  }
  void RoutineCompiler::freeRegs()
  {
    auto reg = regs.begin();
    while(reg != regs.end()) {
      if(reg->first->lifeEnd < curLifeTime) {
        switch(reg->second.type) {
          case OperandType::Register:
            regInfo[reg->second.reg] = RegisterInfo::Used;
            break;
          case OperandType::Memory:
            if(reg->second.memory.base && reg->second.memory.base.get() == Register::RBP) {
              // TODO: call destructor
              for(int i = 0; i < reg->second.memory.numBytes; ++i) {
                assert(stackStatus[-reg->second.memory.disp + i]);
                stackStatus[-reg->second.memory.disp + i] = false;
              }
              size_t size = 0;
              for(int i = stackStatus.size() - 1; i >= 0; --i) {
                if(!stackStatus[i]) ++size;
                else break;
              }
              stackStatus.resize(stackStatus.size() - size);
            }
            break;
        }
        regs.erase(reg++);
      }
      else {
        ++reg;
      }
    }
  }
  OperationSize RoutineCompiler::getSize(const pcir::TypeSection* type)
  {
    using namespace pcir;
    switch(type->types) {
      case Types::I8:
      case Types::U8:
        return OperationSize::Byte;
      case Types::I16:
      case Types::U16:
        return OperationSize::Word;
      case Types::I32:
      case Types::U32:
        return OperationSize::DWord;
      case Types::I64:
      case Types::U64:
        return OperationSize::QWord;
      case Types::Ptr:
        return OperationSize::QWord;
      case Types::Function:
        return OperationSize::QWord;
      default:
        assert(false);
        return OperationSize::DWord;
    }
  }
}