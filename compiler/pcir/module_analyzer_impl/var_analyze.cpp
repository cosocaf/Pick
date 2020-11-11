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
        reg->type = symbol.get()->type;
        auto loadSymbol = new LoadSymbolInstruction();
        loadSymbol->reg = reg;
        loadSymbol->name = symbol.get()->fullyQualifiedName;
        (*flow)->addReg(reg);
        (*flow)->insts.push_back(loadSymbol);
      }
      else {
        return error(symbol.err());
      }
    }
    else {
      reg = flowVar->reg;
    }
    return ok(reg);
  }
}