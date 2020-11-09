#include "routine_compiler.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <unordered_map>

#include "utils/vector_utils.h"
#include "utils/instanceof.h"
#include "utils/dyn_cast.h"
#include "utils/map_utils.h"

#include "pcir/pcir_code.h"
#include "pcir/pcir_format.h"

namespace pickc::windows::x64
{
  RoutineCompiler::RoutineCompiler(bundler::Function* fn) : routine(new Routine{ fn }) {};
  Result<Routine*, std::vector<std::string>> RoutineCompiler::compile()
  {
    std::vector<std::string> errors;

    bundler::Register* curCmpReg;
    enum struct Cond
    {
      EQ,
      NEQ,
      GT,
      GE,
      LT,
      LE
    } curCmpCnd;

    stack = 0;
    minStack = 0;

    for(size_t i = 0, l = routine->fn->type->type.fn.args.size(); i < l; ++i) {
      if(i < 4) {
        regInfo[*(argRegs.begin() + i)] = RegisterInfo::Argument;
        args.push_back(Operand(*(argRegs.begin() + i)));
      }
      else {
        args.push_back(Operand(Memory(Register::RBP, (routine->fn->type->type.fn.args.size() - i) * 8, false)));
      }
    }

    prologue.push_back(new PushOperation(Operand(Register::RBP)));
    prologue.push_back(new MovOperation(OperationSize::QWord, Operand(Register::RBP), Operand(Register::RSP)));

    std::set<size_t> insertEpilogue;

    for(auto& group : routine->fn->phis) {
      auto operand = Operand(allocStack(8));
      for(auto& reg : group.second->regs) {
        if(keyExists(regs, reg)) {
          continue;
        }
        else {
          regs[reg] = operand;
        }
      }
    }

    const auto cmpOp = [&](Cond cond, bundler::BinaryInstruction* bin) {
      // assert(regs.find(bin->dist) == regs.end());
      assert(regs.find(bin->left) != regs.end());
      assert(regs.find(bin->right) != regs.end());
      if(curCmpReg == bin->left) {
        regs[bin->left] = createOperand();
        curCmpReg = nullptr;
        // TODO
        assert(false);
      }
      else if(curCmpReg == bin->right) {
        regs[bin->right] = createOperand();
        curCmpReg = nullptr;
        // TODO
        assert(false);
      }
      regs[bin->dist] = Operand();
      curCmpReg = bin->dist;
      curCmpCnd = cond;
      if(regs[bin->left].type == OperandType::Immediate && regs[bin->right].type == OperandType::Immediate) {
        body.push_back(new MovOperation(getSize(bin->left->type), Operand(Register::RAX), regs[bin->left]));
        body.push_back(new CmpOperation(getSize(bin->left->type), Operand(Register::RAX), regs[bin->right]));
      }
      else if(regs[bin->left].type == OperandType::Memory && regs[bin->right].type == OperandType::Memory) {
        body.push_back(new MovOperation(getSize(bin->left->type), Operand(Register::RAX), regs[bin->left]));
        body.push_back(new CmpOperation(getSize(bin->left->type), Operand(Register::RAX), regs[bin->right]));
      }
      else {
        body.push_back(new CmpOperation(getSize(bin->left->type), regs[bin->left], regs[bin->right]));
      }
    };
    for(size_t i = 0, l = routine->fn->insts.size(); i < l; ++i) {
      auto inst = routine->fn->insts[i];
      routine->bundleIndexes.push_back(body.size());
      if(instanceof<bundler::AddInstruction>(inst)) {
        auto add = dynCast<bundler::AddInstruction>(inst);
        assert(regs.find(add->left) != regs.end());
        assert(regs.find(add->right) != regs.end());
        if(curCmpReg == add->left) {
          regs[add->left] = createOperand();
          curCmpReg = nullptr;
          // TODO
          assert(false);
        }
        else if(curCmpReg == add->right) {
          regs[add->right] = createOperand();
          curCmpReg = nullptr;
          // TODO
          assert(false);
        }
        if(regs[add->left].type == OperandType::Immediate && regs[add->right].type == OperandType::Immediate) {
          if(keyExists(regs, add->dist)) {
            body.push_back(new MovOperation(getSize(add->dist->type), regs[add->dist], Operand(regs[add->left].imm + regs[add->right].imm)));
          }
          else {
            regs[add->dist] = Operand(regs[add->left].imm + regs[add->right].imm);
          }
        }
        else {
          if(!keyExists(regs, add->dist)) {
            regs[add->dist] = createOperand();
          }
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
      else if(instanceof<bundler::SubInstruction>(inst)) {
        auto sub = dynCast<bundler::SubInstruction>(inst);
        assert(regs.find(sub->left) != regs.end());
        assert(regs.find(sub->right) != regs.end());
        if(curCmpReg == sub->left) {
          regs[sub->left] = createOperand();
          curCmpReg = nullptr;
          // TODO
          assert(false);
        }
        else if(curCmpReg == sub->right) {
          regs[sub->right] = createOperand();
          curCmpReg = nullptr;
          // TODO
          assert(false);
        }
        if(regs[sub->left].type == OperandType::Immediate && regs[sub->right].type == OperandType::Immediate) {
          if(keyExists(regs, sub->dist)) {
            body.push_back(new MovOperation(getSize(sub->dist->type), regs[sub->dist], Operand(regs[sub->left].imm - regs[sub->right].imm)));
          }
          else {
            regs[sub->dist] = Operand(regs[sub->left].imm - regs[sub->right].imm);
          }
        }
        else {
          if(!keyExists(regs, sub->dist)) {
            regs[sub->dist] = createOperand();
          }
          if(regs[sub->dist].type == OperandType::Memory && (regs[sub->left].type == OperandType::Memory || regs[sub->right].type == OperandType::Memory)) {
            body.push_back(new MovOperation(getSize(sub->dist->type), Operand(Register::RAX), regs[sub->left]));
            body.push_back(new SubOperation(getSize(sub->dist->type), Operand(Register::RAX), regs[sub->right]));
            body.push_back(new MovOperation(getSize(sub->dist->type), regs[sub->dist], Operand(Register::RAX)));
          }
          else {
            body.push_back(new MovOperation(getSize(sub->dist->type), regs[sub->dist], regs[sub->left]));
            body.push_back(new SubOperation(getSize(sub->dist->type), regs[sub->dist], regs[sub->right]));
          }
        }
      }
      else if(instanceof<bundler::EqInstruction>(inst)) {
        cmpOp(Cond::EQ, dynCast<bundler::BinaryInstruction>(inst));
      }
      else if(instanceof<bundler::NeqInstruction>(inst)) {
        cmpOp(Cond::NEQ, dynCast<bundler::BinaryInstruction>(inst));
      }
      else if(instanceof<bundler::GtInstruction>(inst)) {
        cmpOp(Cond::GT, dynCast<bundler::BinaryInstruction>(inst));
      }
      else if(instanceof<bundler::GeInstruction>(inst)) {
        cmpOp(Cond::GE, dynCast<bundler::BinaryInstruction>(inst));
      }
      else if(instanceof<bundler::LtInstruction>(inst)) {
        cmpOp(Cond::LT, dynCast<bundler::BinaryInstruction>(inst));
      }
      else if(instanceof<bundler::LeInstruction>(inst)) {
        cmpOp(Cond::LE, dynCast<bundler::BinaryInstruction>(inst));
      }
      else if(instanceof<bundler::ImmInstruction>(inst)) {
        auto imm = dynCast<bundler::ImmInstruction>(inst);
        if(keyExists(regs, imm->dist)) {
          body.push_back(new MovOperation(getSize(imm->dist->type), regs[imm->dist], Operand(imm->imm)));
        }
        else if(imm->imm <= INT32_MAX && imm->imm >= INT32_MIN) {
          regs[imm->dist] = Operand(imm->imm);
        }
        else {
          regs[imm->dist] = createOperand();
          body.push_back(new MovOperation(OperationSize::QWord, regs[imm->dist], Operand(imm->imm)));
        }
      }
      else if(instanceof<bundler::RetInstruction>(inst)) {
        auto ret = dynCast<bundler::RetInstruction>(inst);
        assert(regs.find(ret->value) != regs.end());
        if(curCmpReg == ret->value) {
          // TODO
          assert(false);
        }
        else {
          body.push_back(new MovOperation(getSize(ret->value->type), Operand(Register::RAX), regs[ret->value]));
        }
        insertEpilogue.insert(body.size());
      }
      else if(instanceof<bundler::LoadFnInstruction>(inst)) {
        auto loadFn = dynCast<bundler::LoadFnInstruction>(inst);
        if(keyExists(regs, loadFn->dist)) {
          body.push_back(new MovOperation(OperationSize::QWord, regs[loadFn->dist], Operand(Relocation(loadFn->fn))));
        }
        else {
          regs[loadFn->dist] = Operand(Relocation(loadFn->fn));
        }
      }
      else if(instanceof<bundler::LoadArgInstruction>(inst)) {
        // 引数は関数の実行中に移動する可能性があるのでLoadFnとは異なり、値をコピーする。
        auto loadArg = dynCast<bundler::LoadArgInstruction>(inst);
        if(!keyExists(regs, loadArg->dist)) {
          regs[loadArg->dist] = createOperand();
        }
        if(regs[loadArg->dist].type == OperandType::Memory && args[loadArg->indexOfArg].type == OperandType::Memory) {
          body.push_back(new MovOperation(getSize(loadArg->dist->type), Operand(Register::RAX), args[loadArg->indexOfArg]));
          body.push_back(new MovOperation(getSize(loadArg->dist->type), regs[loadArg->dist], Operand(Register::RAX)));
        }
        else {
          body.push_back(new MovOperation(getSize(loadArg->dist->type), regs[loadArg->dist], args[loadArg->indexOfArg]));
        }
      }
      else if(instanceof<bundler::LoadSymbolInstruction>(inst)) {
        auto loadSymbol = dynCast<bundler::LoadSymbolInstruction>(inst);
        if(keyExists(regs, loadSymbol->dist)) {
          if(regs[loadSymbol->dist].type == OperandType::Register) {
            body.push_back(new MovOperation(getSize(loadSymbol->dist->type), regs[loadSymbol->dist], Operand(Relocation(loadSymbol->symbol))));
          }
          else {
            body.push_back(new MovOperation(OperationSize::QWord, Operand(Register::RAX), Operand(Relocation(loadSymbol->symbol))));
            body.push_back(new MovOperation(getSize(loadSymbol->dist->type), Operand(Register::RAX), Operand(Memory(Register::RAX, 8, false))));
            body.push_back(new MovOperation(getSize(loadSymbol->dist->type), regs[loadSymbol->dist], Operand(Register::RAX)));
          }
        }
        else {
          regs[loadSymbol->dist] = createOperand();
          body.push_back(new MovOperation(OperationSize::QWord, Operand(Register::RAX), Operand(Relocation(loadSymbol->symbol))));
          if(regs[loadSymbol->dist].type == OperandType::Register) {
            body.push_back(new MovOperation(getSize(loadSymbol->dist->type), regs[loadSymbol->dist], Operand(Operand(Memory(Register::RAX, 8, false)))));
          }
          else {
            body.push_back(new MovOperation(getSize(loadSymbol->dist->type), Operand(Register::RAX), Operand(Memory(Register::RAX, 8, false))));
            body.push_back(new MovOperation(getSize(loadSymbol->dist->type), regs[loadSymbol->dist], Operand(Register::RAX)));
          }
        }
      }
      else if(instanceof<bundler::CallInstruction>(inst)) {
        auto call = dynCast<bundler::CallInstruction>(inst);
        assert(regs.find(call->fn) != regs.end());
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
        body.push_back(new CallOperation(routine, regs[call->fn]));
        // 引数の巻き戻し
        if(!call->args.empty()) {
          body.push_back(new AddOperation(OperationSize::QWord, Operand(Register::RSP), Operand(call->args.size() * 8)));
        }
        // 戻り値の取得
        if(call->dist) {
          if(!keyExists(regs, call->dist)) {
            regs[call->dist] = createOperand();
          }
          body.push_back(new MovOperation(OperationSize::QWord, regs[call->dist], Operand(Register::RAX)));
        }
      }
      else if(instanceof<bundler::JmpInstruction>(inst)) {
        auto jmp = dynCast<bundler::JmpInstruction>(inst);
        if(jmp->cond == nullptr) {
          if(jmp->then != i + 1) {
            body.push_back(new JmpOperation(jmp->then));
          }
        }
        else {
          if(jmp->cond == curCmpReg) {
            switch(curCmpCnd) {
              case Cond::EQ: body.push_back(new JeOperation(jmp->then)); break;
              case Cond::NEQ: body.push_back(new JneOperation(jmp->then)); break;
              case Cond::GT: body.push_back(new JgOperation(jmp->then)); break;
              case Cond::GE: body.push_back(new JgeOperation(jmp->then)); break;
              case Cond::LT: body.push_back(new JlOperation(jmp->then)); break;
              case Cond::LE: body.push_back(new JleOperation(jmp->then)); break;
              default: assert(false);
            }
          }
          else {
            body.push_back(new CmpOperation(getSize(jmp->cond->type), regs[jmp->cond], Operand(0)));
            body.push_back(new JneOperation(jmp->then));
          }
          if(jmp->els != i + 1) {
            body.push_back(new JmpOperation(jmp->els));
          }
        }
      }
      else {
        assert(false);
      }
    }

    std::vector<Operation*> saveNonvolatileRegs;

    for(auto reg : nonvolatileRegs) {
      if(regInfo[reg] != RegisterInfo::Argument) {
        routine->baseDiff -= 8;
        saveNonvolatileRegs.push_back(new MovOperation(OperationSize::QWord, Operand(Memory(Register::RBP, routine->baseDiff, 8, false)), Operand(reg)));
        epilogue.push_back(new MovOperation(OperationSize::QWord, Operand(reg), Operand(Memory(Register::RBP, routine->baseDiff, 8, false))));
      }
    }

    if(routine->baseDiff != 0 || minStack != 0) {
      auto alloc = routine->baseDiff + minStack;
      alloc = (alloc - 15) / 16 * 16;
      prologue.push_back(new SubOperation(OperationSize::QWord, Operand(Register::RSP), Operand(-alloc)));
    }
    prologue += saveNonvolatileRegs;

    epilogue.push_back(new LeaveOperation());
    epilogue.push_back(new RetOperation());

    routine->code += prologue;
    size_t curBodyIndex = 0;
    for(auto insert : insertEpilogue) {
      routine->code.reserve(routine->code.size() + insert - curBodyIndex);
      routine->code.insert(routine->code.end(), body.begin() + curBodyIndex, body.begin() + insert);
      routine->code += epilogue;
      curBodyIndex = insert;
      for(auto& index : routine->bundleIndexes) {
        if(index >= insert) index += epilogue.size();
      }
    }
    routine->code.insert(routine->code.end(), body.begin() + curBodyIndex, body.end());
    
    for(auto& index : routine->bundleIndexes) {
      index += prologue.size();
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
          return Memory(Register::RBP, -i - numBytes, numBytes, true);
        }
      }
      else {
        cur = 0;
      }
    }
    stack -= numBytes;
    minStack = std::min(stack, minStack);
    stackStatus.reserve(stackStatus.size() + numBytes);
    for(size_t i = 0; i < numBytes; ++i) {
      stackStatus.push_back(true);
    }
    return Memory(Register::RBP, stack, numBytes, true);
  }
  void RoutineCompiler::saveRegs()
  {
    freeRegs();
    // 呼び出し元ルーチン自身の引数を保存
    for(size_t i = 0, l = std::min(args.size(), 4ull); i < l; ++i) {
      if(args[i].type == OperandType::Register && regInfo[args[i].reg] == RegisterInfo::Argument) {
        auto reg = args[i].reg;
        regInfo[reg] = RegisterInfo::Used;
        args[i] = Operand(Memory(Register::RBP, (l - i) * 8, false));
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
    // auto reg = regs.begin();
    // while(reg != regs.end()) {
    //   reg->second;
    //   if(reg->first->lifeEnd < curLifeTime) {
    //     switch(reg->second->type) {
    //       case OperandType::Register:
    //         regInfo[reg->second->reg] = RegisterInfo::Used;
    //         break;
    //       case OperandType::Memory:
    //         if(reg->second->memory.base && reg->second->memory.base.get() == Register::RBP && reg->second->memory.needAddressFix) {
    //           // TODO: call destructor
    //           for(int i = 0; i < reg->second->memory.numBytes; ++i) {
    //             assert(stackStatus[-reg->second->memory.disp + i]);
    //             stackStatus[-reg->second->memory.disp + i] = false;
    //           }
    //           size_t size = 0;
    //           for(int i = stackStatus.size() - 1; i >= 0; --i) {
    //             if(!stackStatus[i]) ++size;
    //             else break;
    //           }
    //           stackStatus.resize(stackStatus.size() - size);
    //         }
    //         break;
    //     }
    //     regs.erase(reg++);
    //   }
    //   else {
    //     ++reg;
    //   }
    // }
  }
  OperationSize RoutineCompiler::getSize(const pcir::TypeSection* type)
  {
    using namespace pcir;
    switch(type->types) {
      case Types::I8:
      case Types::U8:
      case Types::Bool:
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