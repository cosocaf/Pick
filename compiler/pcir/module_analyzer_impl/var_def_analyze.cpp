#include "module_analyzer.h"

#include "utils/vector_utils.h"

namespace pickc::pcir
{
  Result<Register*, std::vector<std::string>> ModuleAnalyzer::varDefAnalyze(const parser::VariableDefineNode* varDef, FlowNode** flow)
  {
    using namespace parser;
    Register* reg = nullptr;
    std::vector<std::string> errors;
    Type expectedType(varDef->type);
    if((*flow)->findVar(varDef->name->name)) errors.push_back(createSemanticError(varDef->name, "既に変数 " + varDef->name->name + " は定義されています。"));
    else if(varDef->init) {
      if(auto res = exprAnalyze(varDef->init, flow)) {
        if(res.get()->status == RegisterStatus::Uninited) {
          errors.push_back(createSemanticError(varDef, "未初期化の値を代入しようとしています。"));
        }
        else if(res.get()->status == RegisterStatus::Moved) {
          errors.push_back(createSemanticError(varDef, "既に移動された値を代入しようとしています。"));
        }
        else if(!Type::castable(expectedType, res.get()->type)) {
          errors.push_back(createSemanticError(varDef, "変数の型に式の型を変換できません。"));
        }
        else {
          if(res.get()->vType == ValueType::RightValue) {
            reg = res.get();
          }
          else {
            res.get()->status = RegisterStatus::Moved;
            reg = new Register();
            (*flow)->addReg(reg);
            auto inst = new MovInstruction();
            inst->dist = reg;
            inst->src = res.get();
            (*flow)->insts.push_back(inst);
          }
          reg->mut = varDef->isMut ? Mutability::Mutable : Mutability::Immutable;
          reg->status = RegisterStatus::InUse;
          reg->vType = ValueType::LeftValue;
          reg->type = Type::merge(Mov, expectedType, res.get()->type);
          reg->scope = RegisterScope::LocalVariable;
          (*flow)->vars[varDef->name->name] = reg;
        }
      }
      else {
        errors += res.err();
      }
    }
    else {
      reg = new Register();
      reg->mut = varDef->isMut ? Mutability::Mutable : Mutability::Immutable;
      reg->status = RegisterStatus::Uninited;
      reg->vType = ValueType::LeftValue;
      reg->type = expectedType;
      reg->scope = RegisterScope::LocalVariable;
      (*flow)->vars[varDef->name->name] = reg;
    }
    if(errors.empty()) return ok(reg);
    return error(errors);
  }
}