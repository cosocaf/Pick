#ifndef PICKC_WINDOWS_X64_COMPILER_H_
#define PICKC_WINDOWS_X64_COMPILER_H_

#include <vector>
#include <map>
#include <string>

#include "pickc/compiler_option.h"
#include "utils/result.h"
#include "bundler/bundle.h"

#include "routine.h"

namespace pickc::windows::x64
{
  class Compiler
  {
    WindowsX64 x64;
    bundler::Bundle bundle;
  public:
    Compiler(const bundler::Bundle& bundle);
    Result<WindowsX64, std::vector<std::string>> compile(const CompilerOption& option);
  };
}

#endif // PICKC_WINDOWS_X64_COMPILER_H_