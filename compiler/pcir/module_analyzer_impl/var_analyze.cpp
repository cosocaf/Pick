#include "module_analyzer.h"

#include "utils/instanceof.h"

namespace pickc::pcir
{
  Result<Register*, std::vector<std::string>> ModuleAnalyzer::varAnalyze(const parser::VariableNode* var, FlowNode** flow)
  {
    using namespace parser;
    Variable* flowVar = nullptr;
    Register* reg = nullptr;
    if(instanceof<ScopedVariableNode>(var) || !(flowVar = (*flow)->findVar(var->name))) {
      if(auto symbol = findGlobalVar(var)) {
        reg = new Register();
        reg->type = symbol->type;
        auto loadSymbol = new LoadSymbolInstruction();
        loadSymbol->reg = reg;
        loadSymbol->name = symbol->fullyQualifiedName;
        (*flow)->addReg(reg);
        (*flow)->insts.push_back(loadSymbol);
      }
    }
    else {
      reg = flowVar->reg;
    }
    if(!reg) return error(std::vector{ createSemanticError(var, "変数 " + var->name + " は定義されていません。") });
    return ok(reg);
  }
}