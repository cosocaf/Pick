#include "module_analyzer.h"

#include "utils/instanceof.h"
#include "utils/vector_utils.h"
#include "utils/dyn_cast.h"

namespace pickc::pcir
{
  Result<Register*, std::vector<std::string>> ModuleAnalyzer::unaryAnalyze(const parser::UnaryNode* unary, FlowNode** flow)
  {
    using namespace parser;
    Register* reg = nullptr;
    std::vector<std::string> errors;
    const auto incdec = [&](uint8_t code, UnaryInstructions uinst, bool isFront) {
      if(auto base = exprAnalyze(unary->base, flow)) {
        if(base.get()->curVar == nullptr) {
          errors.push_back(createSemanticError(unary, "値は左辺値である必要があります。"));
        }
        else if(base.get()->curVar->mut == Mutability::Immutable) {
          errors.push_back(createSemanticError(unary, "値はミュータブルである必要があります。"));
        }
        else if(base.get()->curVar->status == VariableStatus::Uninited) {
          errors.push_back(createSemanticError(unary, "初期化されていない変数です。"));
        }
        else if(base.get()->curVar->status == VariableStatus::Moved) {
          errors.push_back(createSemanticError(unary, "既に移動されている変数です。"));
        }
        else if(!Type::computable(code, &base.get()->type)) {
          errors.push_back(createSemanticError(unary, "不正な型です。"));
        }
        else {
          Register* baseReg;
          Register* distReg;
          reg = baseReg = base.get();
          distReg = new Register();
          distReg->type = baseReg->type;
          base.get()->curVar->reg = distReg;
          (*flow)->addReg(distReg);
          auto inst = new UnaryInstruction();
          inst->inst = uinst;
          inst->reg = baseReg;
          inst->dist = distReg;
          (*flow)->insts.push_back(inst);
        }
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
        else {
          reg = new Register();
          reg->type = base.get()->type;
          (*flow)->addReg(reg);
          if(base.get()->curVar != nullptr) {
            if(base.get()->curVar->status == VariableStatus::Uninited) {
              errors.push_back(createSemanticError(unary, "初期化されていない変数です。"));
            }
            else if(base.get()->curVar->status == VariableStatus::Moved) {
              errors.push_back(createSemanticError(unary, "既に移動されている変数です。"));
            }
          }
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
      auto ac = dynCast<ArrayAccessNode>(unary);
      if(auto array = exprAnalyze(ac->base, flow)) {
        if(array.get()->curVar != nullptr) {
          if(array.get()->curVar->status == VariableStatus::Uninited) {
            errors.push_back(createSemanticError(ac->base, "初期化されていない変数です。"));
          }
          else if(array.get()->curVar->status == VariableStatus::Moved) {
            errors.push_back(createSemanticError(ac->base, "既に移動された変数です。"));
          }
        }
        if(!array.get()->type.isArray() && !array.get()->type.isPtr()) {
          errors.push_back(createSemanticError(ac, "[]でアクセスできるのは配列型かポインタ型のみです。"));
        }
        if(auto suffix = exprAnalyze(ac->suffix, flow)) {
          if(suffix.get()->curVar != nullptr) {
            if(suffix.get()->curVar->status == VariableStatus::Uninited) {
              errors.push_back(createSemanticError(ac->suffix, "初期化されていない変数です。"));
            }
            else if(suffix.get()->curVar->status == VariableStatus::Moved) {
              errors.push_back(createSemanticError(ac->suffix, "既に移動された変数です。"));
            }
          }
          if(!suffix.get()->type.isInt()) {
            errors.push_back(createSemanticError(ac, "インデックスに使用できる方は整数型のみです。"));
          }
          if(errors.empty()) {
            reg = new Register();
            reg->vType = ValueType::LValue;
            if(array.get()->type.isArray()) {
              reg->type = *array.get()->type.array.elem;
            }
            else {
              reg->type = *array.get()->type.ptr.elem;
            }
            (*flow)->addReg(reg);
            auto inst = new LoadElemInstruction();
            inst->dist = reg;
            inst->array = array.get();
            inst->index = suffix.get();
            (*flow)->insts.push_back(inst);
          }
        }
        else {
          errors += suffix.err();
        }
      }
      else {
        errors += array.err();
      }
    }
    else if(instanceof<CallNode>(unary)) {
      auto call = dynCast<CallNode>(unary);
      if(auto fn = exprAnalyze(call->base, flow)) {
        if(fn.get()->curVar != nullptr) {
          if(fn.get()->curVar->status == VariableStatus::Uninited) {
            errors.push_back(createSemanticError(call->base, "初期化されていない変数です。"));
          }
          else if(fn.get()->curVar->status == VariableStatus::Moved) {
            errors.push_back(createSemanticError(call->base, "既に移動された変数です。"));
          }
        }
        if(!fn.get()->type.isFn()) {
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
                errors.push_back(createSemanticError(call->args[i], "関数の呼び出しが不正です。引数の型は " + fn.get()->type.fn.args[i]->toString() + " ですが、" + arg.get()->type.toString() + " が指定されました。"));
              }
            }
            else {
              errors += arg.err();
            }
          }
          if(errors.empty()) {
            reg = new Register();
            reg->type = *fn.get()->type.fn.retType;
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