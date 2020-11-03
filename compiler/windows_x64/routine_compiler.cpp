#include "routine_compiler.h"

#include <algorithm>
#include <cmath>

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
    routine->code.push_back(new PushOperation(Operand(Register::RBP)));
    routine->code.push_back(new MovOperation(OperationSize::QWord, Operand(Register::RBP), Operand(Register::RSP)));

    curLifeTime = 0;
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
        else if(regs[add->left].type == OperandType::Immediate) {
          regs[add->dist] = createOperand();
          routine->code.push_back(new MovOperation(getSize(add->dist->type), regs[add->dist], regs[add->right]));
          routine->code.push_back(new AddOperation(getSize(add->dist->type), regs[add->dist], regs[add->left]));
        }
        else {
          regs[add->dist] = createOperand();
          routine->code.push_back(new MovOperation(getSize(add->dist->type), regs[add->dist], regs[add->left]));
          routine->code.push_back(new AddOperation(getSize(add->dist->type), regs[add->dist], regs[add->right]));
        }
      }
      else if(instanceof<SSAImmInstruction>(inst)) {
        auto imm = dynamic_cast<SSAImmInstruction*>(inst);
        assert(regs.find(imm->dist) == regs.end());
        regs[imm->dist] = Operand(imm->imm);
      }
      else if(instanceof<SSARetInstruction>(inst)) {
        auto ret = dynamic_cast<SSARetInstruction*>(inst);
        assert(regs.find(ret->value) != regs.end());
        // TODO
        routine->code.push_back(new MovOperation(getSize(ret->value->type), Operand(Register::RAX), regs[ret->value]));
        routine->code.push_back(new LeaveOperation());
        routine->code.push_back(new RetOperation());
      }
      else if(instanceof<SSALoadFnInstruction>(inst)) {
        auto loadFn = dynamic_cast<SSALoadFnInstruction*>(inst);
        assert(regs.find(loadFn->dist) == regs.end());
        regs[loadFn->dist] = Operand(Relocation(loadFn->fn));
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
          routine->code.push_back(new SubOperation(OperationSize::QWord, Operand(Register::RSP), Operand(shadowStore)));
        }
        for(size_t arg = 0, argL = call->args.size(); arg < argL; ++arg) {
          switch(arg) {
            case 0:
              routine->code.push_back(new MovOperation(OperationSize::QWord, Operand(Register::RCX), regs[call->args[arg]]));
              break;
            case 1:
              routine->code.push_back(new MovOperation(OperationSize::QWord, Operand(Register::RDX), regs[call->args[arg]]));
              break;
            case 2:
              routine->code.push_back(new MovOperation(OperationSize::QWord, Operand(Register::R8), regs[call->args[arg]]));
              break;
            case 3:
              routine->code.push_back(new MovOperation(OperationSize::QWord, Operand(Register::R9), regs[call->args[arg]]));
              break;
            default:
              routine->code.push_back(new PushOperation(regs[call->args[arg]]));
          }
        }
        routine->code.push_back(new CallOperation(routine, regs[call->call]));
        if(call->dist) {
          regs[call->dist] = createOperand();
          routine->code.push_back(new MovOperation(OperationSize::QWord, regs[call->dist], Operand(Register::RAX)));
        }
      }
      else {
        assert(false);
      }
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
    // TODO: stack
    assert(false);
    return Operand(0ll);
  }
  void RoutineCompiler::saveRegs()
  {
    freeRegs();
    for(auto reg : volatileRegs) {
      if(regInfo[reg] == RegisterInfo::InUse) {
        routine->code.push_back(new PushOperation(Operand(reg)));
      }
    }
  }
  void RoutineCompiler::freeRegs()
  {
    for(auto reg : regs) {
      if(reg.first->lifeEnd < curLifeTime) {
        switch(reg.second.type) {
          case OperandType::Register:
            regInfo[reg.second.reg] = RegisterInfo::Used;
            break;
          case OperandType::Memory:
            // TODO: free stack memory
            break;
        }
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