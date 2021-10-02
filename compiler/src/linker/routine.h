/**
 * @file routine.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-09-27
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_LINKER_ROUTINE_H_
#define PICKC_LINKER_ROUTINE_H_

#include <vector>

#include "variable.h"
#include "block.h"

#include "utils/binary_vec.h"

namespace pickc::linker {
  struct Routine {
    std::vector<Variable> variables;
    std::vector<Block> blocks;
    // compilerOption::target向けの機械語
    BinaryVec bin;
    // 出力される機械語上のこのルーチンが配置されるアドレス
    uint64_t address;
  };
}

#endif // PICKC_LINKER_ROUTINE_H_