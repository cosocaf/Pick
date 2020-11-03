#ifndef PICKC_PCIR_SEMANTIC_ANALYZER_H_
#define PICKC_PCIR_SEMANTIC_ANALYZER_H_

#include <string>
#include <vector>
#include <map>
#include <set>

#include "pickc/module_tree.h"
#include "pickc/compiler_option.h"
#include "utils/option.h"
#include "utils/binary_vec.h"
#include "pcir.h"

namespace pickc::pcir
{
  class SemanticAnalyzer
  {
    friend class ModuleAnalyzer;
    ModuleTree* rootTree;
    std::set<std::string> texts;
    std::set<Type> types;
    std::map<std::string, Module*> modules;
    std::vector<Symbol*> symbols;
    std::map<Function*, BinaryVec> functions;
    Option<std::vector<std::string>> declare(ModuleTree* tree);
    Option<std::vector<std::string>> analyze(ModuleTree* tree);
    void findModules(ModuleTree* mod);
    void insertType(const Type& type);
    BinaryVec compileFunction(const Function* fn);
    void dumpPCIR(const std::string& path);
  public:
    SemanticAnalyzer(ModuleTree* rootTree);
    Option<std::vector<std::string>> write(const CompilerOption& option);
  };
}

#endif // PICKC_PCIR_SEMANTIC_ANALYZER_H_