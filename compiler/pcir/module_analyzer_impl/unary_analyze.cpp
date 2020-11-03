#include "module_analyzer.h"

#include "utils/instanceof.h"
#include "utils/vector_utils.h"

namespace pickc::pcir
{
  Result<Register*, std::vector<std::string>> ModuleAnalyzer::unaryAnalyze(const parser::UnaryNode* unary, FlowNode** flow)
  {
    using namespace parser;
    Register* reg = nullptr;
    std::vector<std::string> errors;
    const auto incdec = [&](uint8_t code, UnaryInstructions uinst, bool isFront) {
      if(auto base = exprAnalyze(unary->base, flow)) {
        if(base.get()->vType == ValueType::RightValue || base.get()->mut == Mutability::Mutable) {
          errors.push_back(createSemanticError(unary, "値はミュータブルな左辺値である必要があります。"));
          return;
        }
        if(base.get()->status == RegisterStatus::Uninited) {
          errors.push_back(createSemanticError(unary, "初期化されていない変数です。"));
          return;
        }
        if(base.get()->status == RegisterStatus::Moved) {
          errors.push_back(createSemanticError(unary, "既に移動されている変数です。"));
          return;
        }
        if(!Type::computable(code, &base.get()->type)) {
          errors.push_back(createSemanticError(unary, "不正な型です。"));
          return;
        }
        Register* baseReg;
        Register* distReg;
        if(isFront) {
          reg = baseReg = distReg = base.get();
        }
        else {
          reg = baseReg = base.get();
          distReg = new Register();
          distReg->mut = Mutability::Mutable;
          distReg->status = RegisterStatus::InUse;
          distReg->vType = ValueType::RightValue;
          distReg->type = baseReg->type;
          distReg->scope = RegisterScope::LocalVariable;
          (*flow)->addReg(distReg);
        }
        auto inst = new UnaryInstruction();
        inst->inst = uinst;
        inst->reg = baseReg;
        inst->dist = distReg;
        (*flow)->insts.push_back(inst);
      }
      else {
        errors += base.err();
      }
    };
    const auto posneg = [&](uint8_t code, UnaryInstructions uinst) {
      if(auto base = exprAnalyze(unary->base, flow)) {
        if(!Type::computable(code, &base.get()->type)) {
          errors.push_back(createSemanticError(unary, "不正な型です。"));
        }
        else if(base.get()->status == RegisterStatus::Uninited) {
          errors.push_back(createSemanticError(unary, "初期化されていない変数です。"));
        }
        else if(base.get()->status == RegisterStatus::Moved) {
          errors.push_back(createSemanticError(unary, "既に移動されている変数です。"));
        }
        else {
          reg = new Register();
          reg->mut = Mutability::Mutable;
          reg->status = RegisterStatus::InUse;
          reg->vType = ValueType::RightValue;
          reg->type = base.get()->type;
          reg->scope = RegisterScope::LocalVariable;
          (*flow)->addReg(reg);
          auto inst = new UnaryInstruction();
          inst->inst = uinst;
          inst->dist = reg;
          inst->reg = base.get();
          (*flow)->insts.push_back(inst);
        }
      }
      else {
        errors += base.err();
      }
    };
    if(instanceof<BackIncrementNode>(unary)) {
      incdec(Inc, UnaryInstructions::Inc, false);
    }
    else if(instanceof<BackDecrementNode>(unary)) {
      incdec(Dec, UnaryInstructions::Dec, false);
    }
    else if(instanceof<FrontIncrementNode>(unary)) {
      incdec(Inc, UnaryInstructions::Inc, true);
    }
    else if(instanceof<FrontDecrementNode>(unary)) {
      incdec(Dec, UnaryInstructions::Dec, true);
    }
    else if(instanceof<PlusNode>(unary)) {
      posneg(Pos, UnaryInstructions::Pos);
    }
    else if(instanceof<MinusNode>(unary)) {
      posneg(Neg, UnaryInstructions::Neg);
    }
    else if(instanceof<MemberAccessNode>(unary)) {
      // TODO
      assert(false);
    }
    else if(instanceof<ArrayAccessNode>(unary)) {
      // TODO
      assert(false);
    }
    else if(instanceof<CallNode>(unary)) {
      auto call = dynamic_cast<const CallNode*>(unary);
      if(auto fn = exprAnalyze(call->base, flow)) {
        if(fn.get()->status == RegisterStatus::Uninited) {
          errors.push_back(createSemanticError(call->base, "初期化されていない変数です。"));
        }
        else if(fn.get()->status == RegisterStatus::Moved) {
          errors.push_back(createSemanticError(call->base, "既に移動された変数です。"));
        }
        else if(!fn.get()->type.isFn()) {
          errors.push_back(createSemanticError(call->base, "関数ではありません。"));
        }
        else if(fn.get()->type.fn.args.size() != call->args.size()) {
          errors.push_back(createSemanticError(call, "関数の呼び出しが不正です。関数の引数の数は" + std::to_string(fn.get()->type.fn.args.size()) + "個ですが、引数が" + std::to_string(call->args.size()) + "個でした。"));
        }
        else {
          std::vector<Register*> args;
          for(size_t i = 0, l = call->args.size(); i < l; ++i) {
            if(auto arg = exprAnalyze(call->args[i], flow)) {
              if(Type::castable(*fn.get()->type.fn.args[i], arg.get()->type)) {
                args.push_back(arg.get());
              }
              else {
                errors.push_back(createSemanticError(call->args[i], "関数の呼び出しが不正です。引数の型を変換できません。"));
              }
            }
          }
          if(errors.empty()) {
            reg = new Register();
            reg->mut = Mutability::Mutable;
            reg->status = RegisterStatus::InUse;
            reg->vType = ValueType::RightValue;
            reg->type = *fn.get()->type.fn.retType;
            reg->scope = RegisterScope::LocalVariable;
            (*flow)->addReg(reg);
            auto inst = new CallInstruction();
            inst->fn = fn.get();
            inst->args = std::move(args);
            inst->dist = reg;
            (*flow)->insts.push_back(inst);
          }
        }
      }
      else {
        errors += fn.err();
      }
    }
    if(errors.empty()) return ok(reg);
    return error(errors);
  }
}