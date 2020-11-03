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
    else {
      assert(false);
    }
    if(errors.empty()) return ok(reg);
    return error(errors);
  }
}