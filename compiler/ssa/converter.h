#ifndef PICKC_SSA_CONVERTER_H_
#define PICKC_SSA_CONVERTER_H_

#include "pickc/compiler_option.h"
#include "pcir/pcir_struct.h"
#include "utils/result.h"

#include "ssa_struct.h"

namespace pickc::ssa
{
  class SSAConverter
  {
    std::vector<pcir::PCIRFile> pcirs;
    SSABundle bundle;
    Result<SSAFunction, std::vector<std::string>> fnConvert(const pcir::PCIRFile& pcir, const pcir::FunctionSection* pcirFn);
  public:
    SSAConverter();
    Result<SSABundle, std::vector<std::string>> convert(const CompilerOption& option);
  };
}

#endif