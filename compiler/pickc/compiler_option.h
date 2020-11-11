#ifndef PICKC_PICKC_COMPILER_OPTION_H_
#define PICKC_PICKC_COMPILER_OPTION_H_

#include <string>
#include <vector>

#include "utils/result.h"

namespace pickc
{
  enum struct TargetPlatforms
  {
    WindowsX64
  };
  struct CompilerOption
  {
    TargetPlatforms target;
    bool compilerDebug;
    std::string projectName;
    std::string mainModule;
    std::string out;
    std::string outDir;
    std::vector<std::string> libraries;
    std::string srcDir;

    CompilerOption();
    static Result<CompilerOption, std::string> create(int argc, char* argv[]);
    void dump() const;
  };
}

#endif // PICKC_PICKC_COMPILER_OPTION_H_