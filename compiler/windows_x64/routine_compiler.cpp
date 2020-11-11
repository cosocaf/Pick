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
  RoutineCompiler::RoutineCompiler(bundler::Function* fn, WindowsX64* x64) : routine(new Routine{ fn }), x64(x64) {};
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
    lifetime = 0;

    for(size_t i = 0, l = routine->fn->type->type.fn.args.size(); i < l; ++i) {
      args.push_back(Operand(Memory(Register::RBP, (routine->fn->type->type.fn.args.size() - i + 1) * 8, 8, false)));
    }

    prologue.push_back(new PushOperation(Operand(Register::RBP)));
    prologue.push_back(new MovOperation(OperationSize::QWord, Operand(Register::RBP), Operand(Register::RSP)));

    for(size_t i = 0, l = args.size(); i < l; ++i) {
      args[i] = Operand(Memory(Register::RBP, (i + 1) * 8 + 8, 8, false));
      if(i < 4) {
        prologue.push_back(new MovOperation(OperationSize::QWord, args[i], Operand(*(argRegs.begin() + i))));
      }
    }

    std::set<size_t> insertEpilogue;

    for(auto& group : routine->fn->phis) {
      auto operand = Operand(allocStack(8));
      size_t lifeEnd = 0;
      for(auto& reg : group.second->regs) {
        lifeEnd = std::max(reg->lifeEnd, lifeEnd);
        if(keyExists(regs, reg)) {
          continue;
        }
        else {
          regs[reg] = operand;
        }
      }
      for(auto& reg : group.second->regs) reg->lifeEnd = lifeEnd;
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
        Operand left;
        if(regs[bin->left].type == OperandType::Immediate) {
          left = createOperand();
          body.push_back(new MovOperation(getSize(bin->left->type), left, regs[bin->left]));
        }
        else {
          left = regs[bin->left];
        }
        body.push_back(new CmpOperation(getSize(bin->left->type), left, regs[bin->right]));
      }
    };
    
    for(size_t l = routine->fn->insts.size(); lifetime < l; ++lifetime) {
      auto inst = routine->fn->insts[lifetime];
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
      else if(instanceof<bundler::MulInstruction>(inst)) {
        auto mul = dynCast<bundler::MulInstruction>(inst);
        assert(regs.find(mul->left) != regs.end());
        assert(regs.find(mul->right) != regs.end());
        if(regs[mul->left].type == OperandType::Immediate && regs[mul->right].type == OperandType::Immediate) {
          int64_t imm;
          if(mul->dist->type->type.isSignedInt()) {
            imm = static_cast<int64_t>(regs[mul->left].imm) * static_cast<int64_t>(regs[mul->right].imm);
          }
          else {
            imm = static_cast<int64_t>(static_cast<uint64_t>(regs[mul->left].imm) * static_cast<uint64_t>(regs[mul->right].imm));
          }
          if(keyExists(regs, mul->dist)) {
            body.push_back(new MovOperation(getSize(mul->dist->type), regs[mul->dist], Operand(imm)));
          }
          else {
            regs[mul->dist] = Operand(imm);
          }
        }
        else {
          if(!keyExists(regs, mul->dist)) {
            regs[mul->dist] = createOperand();
          }
          body.push_back(new MovOperation(getSize(mul->dist->type), Operand(Register::RAX), regs[mul->left]));
          Operand right;
          if(regs[mul->right].type == OperandType::Immediate) {
            body.push_back(new MovOperation(getSize(mul->dist->type), Operand(Register::RDX), regs[mul->right]));
            right = Operand(Register::RDX);
          }
          else {
            right = regs[mul->right];
          }
          if(mul->dist->type->type.isSignedInt()) {
            body.push_back(new IMulOperation(getSize(mul->dist->type), right));
          }
          else {
            body.push_back(new MulOperation(getSize(mul->dist->type), right));
          }
          body.push_back(new MovOperation(getSize(mul->dist->type), regs[mul->dist], Operand(Register::RAX)));
        }
      }
      else if(instanceof<bundler::DivInstruction>(inst)) {
        auto div = dynCast<bundler::DivInstruction>(inst);
        assert(regs.find(div->left) != regs.end());
        assert(regs.find(div->right) != regs.end());
        if(regs[div->left].type == OperandType::Immediate && regs[div->right].type == OperandType::Immediate) {
          int64_t imm;
          if(div->dist->type->type.isSignedInt()) {
            imm = static_cast<int64_t>(regs[div->left].imm) / static_cast<int64_t>(regs[div->right].imm);
          }
          else {
            imm = static_cast<int64_t>(static_cast<uint64_t>(regs[div->left].imm) / static_cast<uint64_t>(regs[div->right].imm));
          }
          if(keyExists(regs, div->dist)) {
            body.push_back(new MovOperation(getSize(div->dist->type), regs[div->dist], Operand(imm)));
          }
          else {
            regs[div->dist] = Operand(imm);
          }
        }
        else {
          if(!keyExists(regs, div->dist)) {
            regs[div->dist] = createOperand();
          }
          body.push_back(new MovOperation(getSize(div->dist->type), Operand(Register::RAX), regs[div->left]));
          Operand right;
          if(regs[div->right].type == OperandType::Immediate) {
            right = createOperand();
            body.push_back(new MovOperation(getSize(div->dist->type), right, regs[div->right]));
          }
          else {
            right = regs[div->right];
          }
          if(div->dist->type->type.isSignedInt()) {
            switch(getSize(div->dist->type)) {
              case OperationSize::Byte: break;
              case OperationSize::Word: body.push_back(new CWDOperation()); break;
              case OperationSize::DWord: body.push_back(new CDQOperation()); break;
              case OperationSize::QWord: body.push_back(new CQOOperation()); break;
              default: assert(false);
            }
            body.push_back(new IDivOperation(getSize(div->dist->type), right));
          }
          else {
            body.push_back(new XorOperation(OperationSize::QWord, Operand(Register::RDX), Operand(Register::RDX)));
            body.push_back(new DivOperation(getSize(div->dist->type), right));
          }
          body.push_back(new MovOperation(getSize(div->dist->type), regs[div->dist], Operand(Register::RAX)));
        }
      }
      else if(instanceof<bundler::ModInstruction>(inst)) {
        auto mod = dynCast<bundler::ModInstruction>(inst);
        assert(regs.find(mod->left) != regs.end());
        assert(regs.find(mod->right) != regs.end());
        if(regs[mod->left].type == OperandType::Immediate && regs[mod->right].type == OperandType::Immediate) {
          int64_t imm;
          if(mod->dist->type->type.isSignedInt()) {
            imm = static_cast<int64_t>(regs[mod->left].imm) % static_cast<int64_t>(regs[mod->right].imm);
          }
          else {
            imm = static_cast<int64_t>(static_cast<uint64_t>(regs[mod->left].imm) % static_cast<uint64_t>(regs[mod->right].imm));
          }
          if(keyExists(regs, mod->dist)) {
            body.push_back(new MovOperation(getSize(mod->dist->type), regs[mod->dist], Operand(imm)));
          }
          else {
            regs[mod->dist] = Operand(imm);
          }
        }
        else {
          if(!keyExists(regs, mod->dist)) {
            regs[mod->dist] = createOperand();
          }
          body.push_back(new MovOperation(getSize(mod->dist->type), Operand(Register::RAX), regs[mod->left]));
          Operand right;
          if(regs[mod->right].type == OperandType::Immediate) {
            right = createOperand();
            body.push_back(new MovOperation(getSize(mod->dist->type), right, regs[mod->right]));
          }
          else {
            right = regs[mod->right];
          }
          if(mod->dist->type->type.isSignedInt()) {
            switch(getSize(mod->dist->type)) {
              case OperationSize::Byte: break;
              case OperationSize::Word: body.push_back(new CWDOperation()); break;
              case OperationSize::DWord: body.push_back(new CDQOperation()); break;
              case OperationSize::QWord: body.push_back(new CQOOperation()); break;
              default: assert(false);
            }
            body.push_back(new IDivOperation(getSize(mod->dist->type), right));
          }
          else {
            body.push_back(new XorOperation(OperationSize::QWord, Operand(Register::RDX), Operand(Register::RDX)));
            body.push_back(new DivOperation(getSize(mod->dist->type), right));
          }
          body.push_back(new MovOperation(getSize(mod->dist->type), regs[mod->dist], Operand(Register::RDX)));
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
      else if(instanceof<bundler::IncInstruction>(inst)) {
        auto inc = dynCast<bundler::IncInstruction>(inst);
        assert(regs.find(inc->src) != regs.end());
        if(regs[inc->src].type == OperandType::Immediate) {
          if(keyExists(regs, inc->dist)) {
            body.push_back(new MovOperation(getSize(inc->dist->type), regs[inc->dist], Operand(regs[inc->src].imm + 1)));
          }
          else {
            regs[inc->dist] = Operand(regs[inc->src].imm + 1);
          }
        }
        else {
          if(!keyExists(regs, inc->dist)) {
            regs[inc->dist] = createOperand();
          }
          if(regs[inc->dist].type == OperandType::Memory && regs[inc->src].type == OperandType::Memory) {
            body.push_back(new MovOperation(getSize(inc->dist->type), Operand(Register::RAX), regs[inc->src]));
            body.push_back(new AddOperation(getSize(inc->dist->type), Operand(Register::RAX), Operand(1)));
            body.push_back(new MovOperation(getSize(inc->dist->type), regs[inc->dist], Operand(Register::RAX)));
          }
          else {
            body.push_back(new MovOperation(getSize(inc->dist->type), regs[inc->dist], regs[inc->src]));
            body.push_back(new AddOperation(getSize(inc->dist->type), regs[inc->dist], Operand(1)));
          }
        }
      }
      else if(instanceof<bundler::DecInstruction>(inst)) {
        auto dec = dynCast<bundler::DecInstruction>(inst);
        assert(regs.find(dec->src) != regs.end());
        if(regs[dec->src].type == OperandType::Immediate) {
          if(keyExists(regs, dec->dist)) {
            body.push_back(new MovOperation(getSize(dec->dist->type), regs[dec->dist], Operand(regs[dec->src].imm - 1)));
          }
          else {
            regs[dec->dist] = Operand(regs[dec->src].imm - 1);
          }
        }
        else {
          if(!keyExists(regs, dec->dist)) {
            regs[dec->dist] = createOperand();
          }
          if(regs[dec->dist].type == OperandType::Memory && regs[dec->src].type == OperandType::Memory) {
            body.push_back(new MovOperation(getSize(dec->dist->type), Operand(Register::RAX), regs[dec->src]));
            body.push_back(new SubOperation(getSize(dec->dist->type), Operand(Register::RAX), Operand(1)));
            body.push_back(new MovOperation(getSize(dec->dist->type), regs[dec->dist], Operand(Register::RAX)));
          }
          else {
            body.push_back(new MovOperation(getSize(dec->dist->type), regs[dec->dist], regs[dec->src]));
            body.push_back(new SubOperation(getSize(dec->dist->type), regs[dec->dist], Operand(1)));
          }
        }
      }
      else if(instanceof<bundler::PosInstruction>(inst)) {
        auto pos = dynCast<bundler::PosInstruction>(inst);
        assert(regs.find(pos->src) != regs.end());
        regs[pos->dist] = regs[pos->src];
      }
      else if(instanceof<bundler::NegInstruction>(inst)) {
        auto neg = dynCast<bundler::NegInstruction>(inst);
        assert(regs.find(neg->src) != regs.end());
        if(regs[neg->src].type == OperandType::Immediate) {
          if(keyExists(regs, neg->dist)) {
            body.push_back(new MovOperation(getSize(neg->dist->type), regs[neg->dist], Operand(-regs[neg->src].imm)));
          }
          else {
            regs[neg->dist] = Operand(-regs[neg->src].imm);
          }
        }
        else {
          if(!keyExists(regs, neg->dist)) {
            regs[neg->dist] = createOperand();
          }
          if(regs[neg->dist].type == OperandType::Memory && regs[neg->src].type == OperandType::Memory) {
            body.push_back(new MovOperation(getSize(neg->dist->type), Operand(Register::RAX), regs[neg->src]));
            body.push_back(new NegOperation(getSize(neg->dist->type), Operand(Register::RAX)));
            body.push_back(new MovOperation(getSize(neg->dist->type), regs[neg->dist], Operand(Register::RAX)));
          }
          else {
            body.push_back(new MovOperation(getSize(neg->dist->type), regs[neg->dist], regs[neg->src]));
            body.push_back(new NegOperation(getSize(neg->dist->type), regs[neg->dist]));
          }
        }
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
        assert(ret->value == nullptr || keyExists(regs, ret->value));
        if(curCmpReg == ret->value) {
          // TODO
          assert(false);
        }
        else {
          if(ret->value == nullptr || ret->value->type->type.isVoid()) {
            body.push_back(new XorOperation(OperationSize::QWord, Operand(Register::RAX), Operand(Register::RAX)));
          }
          else {
            if(regs[ret->value].type == OperandType::Immediate) {
              body.push_back(new MovOperation(OperationSize::QWord, Operand(Register::RAX), regs[ret->value]));
            }
            else {
              switch(getSize(ret->value->type)) {
                case OperationSize::Byte:
                case OperationSize::Word:
                  if(ret->value->type->type.isSignedInt()) {
                    body.push_back(new MovSXOperation(OperationSize::QWord, getSize(ret->value->type), Register::RAX, regs[ret->value]));
                  }
                  else {
                    body.push_back(new MovZXOperation(OperationSize::QWord, getSize(ret->value->type), Register::RAX, regs[ret->value]));
                  }
                  break;
                case OperationSize::DWord:
                  if(ret->value->type->type.isSignedInt()) {
                    body.push_back(new MovSXDOperation(Register::RAX, regs[ret->value]));
                  }
                  else {
                    body.push_back(new MovOperation(getSize(ret->value->type), Operand(Register::RAX), regs[ret->value]));
                  }
                  break;
                case OperationSize::QWord:
                  body.push_back(new MovOperation(getSize(ret->value->type), Operand(Register::RAX), regs[ret->value]));
                  break;
                default:
                  assert(false);
              }
            }
          }
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
        auto loadArg = dynCast<bundler::LoadArgInstruction>(inst);
        if(!keyExists(regs, loadArg->dist)) {
          regs[loadArg->dist] = args[loadArg->indexOfArg];
        }
        else if(regs[loadArg->dist].type == OperandType::Register) {
          body.push_back(new MovOperation(getSize(loadArg->dist->type), regs[loadArg->dist], args[loadArg->indexOfArg]));
        }
        else {
          body.push_back(new MovOperation(getSize(loadArg->dist->type), Operand(Register::RAX), args[loadArg->indexOfArg]));
          body.push_back(new MovOperation(getSize(loadArg->dist->type), regs[loadArg->dist], Operand(Register::RAX)));
        }
      }
      else if(instanceof<bundler::LoadSymbolInstruction>(inst)) {
        auto loadSymbol = dynCast<bundler::LoadSymbolInstruction>(inst);
        regs[loadSymbol->dist] = Operand(Relocation(loadSymbol->symbol));
        // if(keyExists(regs, loadSymbol->dist)) {
        //   if(regs[loadSymbol->dist].type == OperandType::Register) {
        //     body.push_back(new MovOperation(getSize(loadSymbol->dist->type), regs[loadSymbol->dist], Operand(Relocation(loadSymbol->symbol))));
        //   }
        //   else {
        //     body.push_back(new MovOperation(OperationSize::QWord, Operand(Register::RAX), Operand(Relocation(loadSymbol->symbol))));
        //     body.push_back(new MovOperation(getSize(loadSymbol->dist->type), Operand(Register::RAX), Operand(Memory(Register::RAX, 8, false))));
        //     body.push_back(new MovOperation(getSize(loadSymbol->dist->type), regs[loadSymbol->dist], Operand(Register::RAX)));
        //   }
        // }
        // else {
        //   regs[loadSymbol->dist] = createOperand();
        //   body.push_back(new MovOperation(OperationSize::QWord, Operand(Register::RAX), Operand(Relocation(loadSymbol->symbol))));
        //   if(regs[loadSymbol->dist].type == OperandType::Register) {
        //     body.push_back(new MovOperation(getSize(loadSymbol->dist->type), regs[loadSymbol->dist], Operand(Operand(Memory(Register::RAX, 8, false)))));
        //   }
        //   else {
        //     body.push_back(new MovOperation(getSize(loadSymbol->dist->type), Operand(Register::RAX), Operand(Memory(Register::RAX, 8, false))));
        //     body.push_back(new MovOperation(getSize(loadSymbol->dist->type), regs[loadSymbol->dist], Operand(Register::RAX)));
        //   }
        // }
      }
      else if(instanceof<bundler::LoadStringInstruction>(inst)) {
        auto loadStr = dynCast<bundler::LoadStringInstruction>(inst);
        x64->texts[loadStr->text->text] = 0;
        if(keyExists(regs, loadStr->dist)) {
          body.push_back(new MovOperation(getSize(loadStr->dist->type), regs[loadStr->dist], Operand(Relocation(loadStr->text))));
        }
        else {
          regs[loadStr->dist] = Operand(Relocation(loadStr->text));
        }
      }
      else if(instanceof<bundler::LoadElemInstruction>(inst)) {
        auto loadElem = dynCast<bundler::LoadElemInstruction>(inst);
        assert(keyExists(regs, loadElem->array));
        assert(keyExists(regs, loadElem->index));
        if(!keyExists(regs, loadElem->dist)) {
          regs[loadElem->dist] = createOperand();
        }
        Register base;
        if(regs[loadElem->array].type == OperandType::Register) {
          base = regs[loadElem->array].reg;
        }
        else {
          base = Register::RAX;
          body.push_back(new MovOperation(getSize(loadElem->array->type), Operand(base), regs[loadElem->array]));
        }
        
        if(regs[loadElem->dist].type == OperandType::Register) {
          if(regs[loadElem->index].type == OperandType::Register) {
            body.push_back(new LeaOperation(OperationSize::QWord, regs[loadElem->dist].reg, Memory(base, regs[loadElem->index].reg, 0, 0, 8, false)));
          }
          else if(regs[loadElem->index].type == OperandType::Memory || regs[loadElem->index].type == OperandType::Relocation) {
            body.push_back(new MovOperation(getSize(loadElem->index->type), Operand(Register::RDX), regs[loadElem->index]));
            body.push_back(new LeaOperation(OperationSize::QWord, regs[loadElem->dist].reg, Memory(base, Register::RDX, 0, 0, 8, false)));
          }
          else if(regs[loadElem->index].type == OperandType::Immediate) {
            body.push_back(new LeaOperation(OperationSize::QWord, regs[loadElem->dist].reg, Memory(base, regs[loadElem->index].imm, 8, false)));
          }
          else {
            assert(false);
          }
        }
        else {
          // dist == Memoryの場合は一度RAXに値を保持しておく。
          if(regs[loadElem->index].type == OperandType::Register) {
            body.push_back(new LeaOperation(OperationSize::QWord, Register::RAX, Memory(base, regs[loadElem->index].reg, 0, 0, 8, false)));
          }
          else if(regs[loadElem->index].type == OperandType::Memory || regs[loadElem->index].type == OperandType::Relocation) {
            body.push_back(new MovOperation(getSize(loadElem->index->type), Operand(Register::RDX), regs[loadElem->index]));
            body.push_back(new LeaOperation(OperationSize::QWord, Register::RAX, Memory(base, Register::RDX, 0, 0, 8, false)));
          }
          else if(regs[loadElem->index].type == OperandType::Immediate) {
            body.push_back(new LeaOperation(OperationSize::QWord, Register::RAX, Memory(base, regs[loadElem->index].imm, 8, false)));
          }
          else {
            assert(false);
          }
          body.push_back(new MovOperation(OperationSize::QWord, regs[loadElem->dist], Operand(Register::RAX)));
        }
      }
      else if(instanceof<bundler::AllocInstruction>(inst)) {
        auto alloc = dynCast<bundler::AllocInstruction>(inst);
        assert(keyExists(regs, alloc->dist));
        assert(keyExists(regs, alloc->src));
        Operand dist;
        if(regs[alloc->dist].type == OperandType::Register) {
          dist = Operand(Memory(regs[alloc->dist].reg, 8, false));
        }
        else {
          body.push_back(new MovOperation(OperationSize::QWord, Operand(Register::RAX), regs[alloc->dist]));
          dist = Operand(Memory(Register::RAX, 8, false));
        }
        if(regs[alloc->src].type == OperandType::Register) {
          body.push_back(new MovOperation(getSize(alloc->dist->type), dist, regs[alloc->src]));
        }
        else {
          body.push_back(new MovOperation(getSize(alloc->src->type), Operand(Register::RDX), regs[alloc->src]));
          body.push_back(new MovOperation(getSize(alloc->dist->type), dist, Operand(Register::RDX)));
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
        
        if(call->args.size() % 2 != 0) body.push_back(new PushOperation(Operand(0)));
        for(int arg = call->args.size() - 1; arg >= 0; --arg) {
          if(arg < 4) {
            body.push_back(new MovOperation(OperationSize::QWord, Operand(*(argRegs.begin() + arg)), regs[call->args[arg]]));
          }
          else {
            body.push_back(new PushOperation(regs[call->args[arg]]));
          }
        }
        auto shadowStore = std::min(call->args.size(), 4ull) * 8;
        if(shadowStore) {
          body.push_back(new SubOperation(OperationSize::QWord, Operand(Register::RSP), Operand(shadowStore)));
        }
        body.push_back(new CallOperation(routine, regs[call->fn]));
        // 引数の巻き戻し
        if(!call->args.empty()) {
          auto rewind = call->args.size() * 8;
          if(call->args.size() % 2 != 0) rewind += 8;
          body.push_back(new AddOperation(OperationSize::QWord, Operand(Register::RSP), Operand(rewind)));
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
          if(jmp->then != lifetime + 1) {
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
          if(jmp->els != lifetime + 1) {
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
      if(regInfo[reg] != RegisterInfo::Unused) {
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
          for(size_t j = 0; j < numBytes; ++j) {
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
      if(reg->first->lifeEnd < lifetime) {
        switch(reg->second.type) {
          case OperandType::Register:
            regInfo[reg->second.reg] = RegisterInfo::Used;
            break;
          case OperandType::Memory:
            if(reg->second.memory.base && reg->second.memory.base.get() == Register::RBP && reg->second.memory.needAddressFix) {
              // TODO: call destructor
              for(int i = 0; i < reg->second.memory.numBytes; ++i) {
                assert(stackStatus[-reg->second.memory.disp - i - 1]);
                stackStatus[-reg->second.memory.disp - i - 1] = false;
              }
            }
            break;
        }
        if(keyExists(routine->fn->phis, reg->first)) {
          for(auto phi : routine->fn->phis[reg->first]->regs) {
            regs.erase(phi);
          }
          reg = regs.begin();
        }
        else {
          regs.erase(reg++);
        }
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
      case Types::Bool:
      case Types::Char:
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
      case Types::Null:
        return OperationSize::QWord;
      case Types::Function:
        return OperationSize::QWord;
      default:
        assert(false);
        return OperationSize::DWord;
    }
  }
}