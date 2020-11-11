#ifndef PICKC_BUNDLER_BUNDLE_H_
#define PICKC_BUNDLER_BUNDLE_H_

#include <map>
#include <unordered_map>
#include <set>

#include "symbol.h"

namespace pickc::bundler
{
  struct Bundle
  {
    std::unordered_map<std::string, std::unordered_set<pcir::SymbolSection*>> modules;
    std::map<pcir::SymbolSection*, Symbol*> symbols;
    std::unordered_map<std::string, pcir::SymbolSection*> symbolNames;
    std::map<pcir::FunctionSection*, Function*> fns;
  };
}

#endif // PICKC_BUNDLER_BUNDLE_H_