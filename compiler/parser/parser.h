#ifndef PICKC_PARSER_PARSER_H_
#define PICKC_PARSER_PARSER_H_

#include <vector>
#include <string>

#include "utils/result.h"
#include "pickc/compiler_option.h"
#include "pickc/module_tree.h"

namespace pickc::parser
{
  class Parser
  {
    void dump(const ModuleTree* tree);
  public:
    Parser();
    Result<ModuleTree*, std::vector<std::string>> parse(const CompilerOption& option);
  };
}

#endif