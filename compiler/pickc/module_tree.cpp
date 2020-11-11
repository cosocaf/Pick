#include "module_tree.h"

namespace pickc
{
  size_t ModuleTree::countModules() const
  {
    size_t count = 1;
    for(auto sub : submodules) {
      count += sub.second->countModules();
    }
    return count;
  }
}