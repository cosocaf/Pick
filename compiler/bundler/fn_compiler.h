#ifndef PICKC_BUNDLER_FN_COMPILER_H_
#define PICKC_BUNDLER_FN_COMPILER_H_

#include "pcir/pcir_struct.h"
#include "utils/result.h"

#include "bundle.h"

namespace pickc::bundler
{
  class FnCompiler
  {
    Bundle* bundle;
    pcir::PCIRFile* pcir;
    pcir::FunctionSection* pcirFn;
    Function* fn;
    std::unordered_map<pcir::RegisterStruct*, Register*> regs;
    std::unordered_map<pcir::FlowStruct*, std::vector<Instruction*>> flows;
    std::unordered_set<pcir::FlowStruct*> joinedFlows;
    Result<_, std::vector<std::string>> flowCompile(pcir::FlowStruct* flow);
    void joinFlows(pcir::FlowStruct* flow);
    // ジャンプ命令の一覧。JmpInstruction::toIndexを求めるために使用する。
    std::unordered_map<JmpInstruction*, std::pair<pcir::FlowStruct*, pcir::FlowStruct*>> jmpTo;
    // フローの配置インデックス一覧。
    std::unordered_map<pcir::FlowStruct*, size_t> flowIndexes;
  public:
    FnCompiler(Bundle* bundle, pcir::PCIRFile* pcir, pcir::FunctionSection* pcirFn);
    Result<Function*, std::vector<std::string>> compile();
  };
}

#endif // PICKC_BUNDLER_FN_COMPILER_H_