#include "module_analyzer.h"

#include "utils/instanceof.h"

namespace pickc::pcir
{
  Result<Register*, std::vector<std::string>> ModuleAnalyzer::varAnalyze(const parser::VariableNode* var, FlowNode** flow)
  {
    using namespace parser;
    if(instanceof<ScopedVariableNode>(var)) {
      // TODO
      assert(false);
    }
    else {
      auto reg = (*flow)->findVar(var->name);
      if(!reg) return error(std::vector{ createSemanticError(var, "変数 " + var->name + " は定義されていません。") });
      return ok(reg);
    }
  }
}