#include "module_analyzer.h"

#include "utils/instanceof.h"
#include "utils/vector_utils.h"

namespace pickc::pcir
{
  Result<Register*, std::vector<std::string>> ModuleAnalyzer::binaryAnalyze(const parser::BinaryNode* binary, FlowNode** flow)
  {
    using namespace parser;
    Register* reg = nullptr;
    std::vector<std::string> errors;
    const auto binaryInst = [&](uint8_t code, BinaryInstructions binst) {
      auto left = exprAnalyze(binary->left, flow);
      auto right = exprAnalyze(binary->right, flow);
      if(!left) errors += left.err();
      else if(!right) errors += right.err();
      else if(!Type::computable(code, &left.get()->type, &right.get()->type)) {
        errors.push_back(createSemanticError(binary, "型 " + left.get()->type.toString() + " と型 " + right.get()->type.toString() + " は演算できません。"));
      }
      else if(left.get()->status == RegisterStatus::Uninited) {
        errors.push_back(createSemanticError(binary->left, "初期化されていない変数です。"));
      }
      else if(right.get()->status == RegisterStatus::Uninited) {
        errors.push_back(createSemanticError(binary->right, "初期化されていない変数です。"));
      }
      else if(left.get()->status == RegisterStatus::Moved) {
        errors.push_back(createSemanticError(binary->left, "既に移動された変数です。"));
      }
      else if(right.get()->status == RegisterStatus::Moved) {
        errors.push_back(createSemanticError(binary->right, "既に移動された変数です。"));
      }
      else {
        reg = new Register();
        reg->mut = Mutability::Mutable;
        reg->status = RegisterStatus::InUse;
        reg->vType = ValueType::RightValue;
        reg->type = Type::merge(code, left.get()->type, right.get()->type);
        reg->scope = RegisterScope::LocalVariable;
        (*flow)->addReg(reg);
        auto inst = new BinaryInstruction();
        inst->dist = reg;
        inst->left = left.get();
        inst->right = right.get();
        inst->inst = binst;
        (*flow)->insts.push_back(inst);
      }
    };
    if(instanceof<MulNode>(binary)) {
      binaryInst(Mul, BinaryInstructions::Mul);
    }
    else if(instanceof<DivNode>(binary)) {
      binaryInst(Div, BinaryInstructions::Div);
    }
    else if(instanceof<ModNode>(binary)) {
      binaryInst(Mod, BinaryInstructions::Mod);
    }
    else if(instanceof<AddNode>(binary)) {
      binaryInst(Add, BinaryInstructions::Add);
    }
    else if(instanceof<SubNode>(binary)) {
      binaryInst(Sub, BinaryInstructions::Sub);
    }
    else if(instanceof<AsignNode>(binary)) {
      auto asign = dynamic_cast<const AsignNode*>(binary);
      auto left = exprAnalyze(asign->left, flow);
      auto right = exprAnalyze(asign->right, flow);
      if(!left) errors += left.err();
      else if(!right) errors += right.err();
      else if(!Type::castable(left.get()->type, right.get()->type)) {
        errors.push_back(createSemanticError(binary, "型 " + left.get()->type.toString() + " に型 " + right.get()->type.toString() + " は代入できません。"));
      }
      else if(right.get()->status == RegisterStatus::Uninited) {
        errors.push_back(createSemanticError(asign->right, "初期化されていない変数です。"));
      }
      else if(right.get()->status == RegisterStatus::Moved) {
        errors.push_back(createSemanticError(asign->right, "既に移動された変数です。"));
      }
      else if(left.get()->vType == ValueType::RightValue) {
        errors.push_back(createSemanticError(asign->right, "右辺値に値は代入できません。"));
      }
      else if(left.get()->mut == Mutability::Immutable && left.get()->status == RegisterStatus::InUse) {
        errors.push_back(createSemanticError(asign->right, "イミュータブルな値に代入しようとしました。"));
      }
      else {
        left.get()->status = RegisterStatus::InUse;
        right.get()->status = RegisterStatus::Moved;
        reg = left.get();
        auto inst = new MovInstruction();
        inst->dist = reg;
        inst->src = right.get();
        (*flow)->insts.push_back(inst);
      }
    }
    else {
      assert(false);
    }
    if(errors.empty()) return ok(reg);
    return error(errors);
  }
}