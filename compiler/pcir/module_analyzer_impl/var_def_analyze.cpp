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
        if(!Type::castable(expectedType, res.get()->type)) {
          errors.push_back(createSemanticError(varDef, "変数の型に式の型を変換できません。"));
        }
        else if(res.get()->curVar != nullptr) {
          if(res.get()->curVar->status == VariableStatus::Uninited) {
            errors.push_back(createSemanticError(varDef, "未初期化の値を代入しようとしています。"));
          }
          else if(res.get()->curVar->status == VariableStatus::Moved) {
            errors.push_back(createSemanticError(varDef, "既に移動された値を代入しようとしています。"));
          }
          res.get()->curVar->status = VariableStatus::Moved;
        }
        reg = res.get();
      }
      else {
        errors += res.err();
      }
    }
    else {
      reg = new Register();
      reg->type = expectedType;
    }
    (*flow)->vars.insert(new Variable{
      varDef->name->name,
      varDef->isMut ? Mutability::Mutable : Mutability::Immutable,
      VariableStatus::InUse,
      expectedType,
      reg
    });
    if(errors.empty()) return ok(reg);
    return error(errors);
  }
}