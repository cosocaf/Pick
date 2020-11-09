#ifndef PICKC_BUNDLER_BUNDLER_H_
#define PICKC_BUNDLER_BUNDLER_H_

#include "pickc/compiler_option.h"
#include "pcir/pcir_struct.h"
#include "utils/result.h"

#include "bundle.h"

namespace pickc::bundler
{
  class Bundler
  {
    Bundle b;
    std::vector<pcir::PCIRFile> pcirs;
  public:
    Bundler();
    Result<Bundle, std::vector<std::string>> bundle(const CompilerOption& option);
  };
}

#endif // PICKC_BUNDLER_BUNDLER_H_