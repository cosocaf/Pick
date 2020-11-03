#include "module_analyzer.h"

#include "utils/instanceof.h"

namespace pickc::pcir
{
  Result<Register*, std::vector<std::string>> ModuleAnalyzer::varAnalyze(const parser::VariableNode* var, FlowNode** flow)
  {
    using namespace parser;
    Register* reg = nullptr;
    if(instanceof<ScopedVariableNode>(var) || !(reg = (*flow)->findVar(var->name))) {
      if(auto symbol = findGlobalVar(var)) {
        reg = new Register();
        reg->mut = symbol->mut;
        reg->scope = RegisterScope::GlobalVariable;
        reg->status = RegisterStatus::InUse;
        reg->type = symbol->type;
        reg->vType = ValueType::LeftValue;
        auto loadSymbol = new LoadSymbolInstruction();
        loadSymbol->reg = reg;
        loadSymbol->name = symbol->fullyQualifiedName;
        (*flow)->addReg(reg);
        (*flow)->insts.push_back(loadSymbol);
      }
    }
    if(!reg) return error(std::vector{ createSemanticError(var, "変数 " + var->name + " は定義されていません。") });
    return ok(reg);
  }
}