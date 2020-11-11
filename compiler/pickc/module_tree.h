#ifndef PICKC_PICKC_MODULE_TREE_H_
#define PICKC_PICKC_MODULE_TREE_H_

#include <string>
#include <unordered_map>

#include "parser/token.h"
#include "parser/ast_node.h"
// #include "ir1/ir1.h"
#include "pcir/pcir.h"

namespace pickc
{
  struct ModuleTree
  {
    // モジュールの完全修飾名
    std::string name;
    ModuleTree* parent;
    // key = モジュール名。完全修飾名ではない。
    std::unordered_map<std::string, ModuleTree*> submodules;
    parser::TokenSequence sequence;
    parser::RootNode ast;
    // ir1::IR1Module ir1Module;
    pcir::Module module;
    size_t countModules() const;
  };
}

#endif // PICKC_PICKC_MODULE_TREE_H_