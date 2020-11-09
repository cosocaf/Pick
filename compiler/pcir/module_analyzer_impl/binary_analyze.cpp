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
      else {
        if(left.get()->curVar != nullptr) {
          if(left.get()->curVar->status == VariableStatus::Uninited) {
            errors.push_back(createSemanticError(binary->left, "初期化されていない変数です。"));
          }
          else if(left.get()->curVar->status == VariableStatus::Moved) {
            errors.push_back(createSemanticError(binary->left, "既に移動された変数です。"));
          }
        }
        if(right.get()->curVar != nullptr) {
          if(right.get()->curVar->status == VariableStatus::Uninited) {
            errors.push_back(createSemanticError(binary->right, "初期化されていない変数です。"));
          }
          else if(right.get()->curVar->status == VariableStatus::Moved) {
            errors.push_back(createSemanticError(binary->right, "既に移動された変数です。"));
          }
        }

        reg = new Register();
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
    else if(instanceof<EqualNode>(binary)) {
      binaryInst(EQ, BinaryInstructions::EQ);
    }
    else if(instanceof<NotEqualNode>(binary)) {
      binaryInst(NEQ, BinaryInstructions::NEQ);
    }
    else if(instanceof<GreaterThanNode>(binary)) {
      binaryInst(GT, BinaryInstructions::GT);
    }
    else if(instanceof<GreaterEqualNode>(binary)) {
      binaryInst(GE, BinaryInstructions::GE);
    }
    else if(instanceof<LessThanNode>(binary)) {
      binaryInst(LT, BinaryInstructions::LT);
    }
    else if(instanceof<LessEqualNode>(binary)) {
      binaryInst(LE, BinaryInstructions::LE);
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
      else if(left.get()->curVar == nullptr) {
        errors.push_back(createSemanticError(asign, "右辺値に値は代入できません。"));
      }
      else if(right.get()->curVar != nullptr) {
        if(right.get()->curVar->status == VariableStatus::Uninited) {
          errors.push_back(createSemanticError(asign->right, "初期化されていない変数です。"));
        }
        else if(right.get()->curVar->status == VariableStatus::Moved) {
          errors.push_back(createSemanticError(asign->right, "既に移動された変数です。"));
        }
        else {
          right.get()->curVar->status = VariableStatus::Moved;
        }
      }

      if(errors.empty()) {
        left.get()->curVar->status = VariableStatus::InUse;
        right.get()->curVar = left.get()->curVar;
        reg = left.get();
      }
    }
    else {
      assert(false);
    }
    if(errors.empty()) return ok(reg);
    return error(errors);
  }
}