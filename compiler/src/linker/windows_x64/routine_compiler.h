/**
 * @file routine_compiler.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-09-29
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_LINKER_WINDOWS_X64_ROUTINE_COMPILER_H_
#define PICKC_LINKER_WINDOWS_X64_ROUTINE_COMPILER_H_

#include <vector>
#include <memory>

#include "assembly.h"

#include "../routine.h"

#include "bundler/bundle.h"

#include "utils/binary_vec.h"

namespace pickc::linker::windows_x64 {
  class RoutineCompiler {
    Routine routine;
  public:
    RoutineCompiler(const Routine& routine);
    BinaryVec compile();
  private:
    int32_t requiredAllocatedMemoryArea(const bundler::TypeInfo& type);
    int32_t memoryAlignment(int32_t offset, int32_t size);
    OperandSize getOperandSize(int32_t size);
  };
}

#endif // PICKC_LINKER_WINDOWS_X64_ROUTINE_COMPILER_H_