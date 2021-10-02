/**
 * @file block.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-09-29
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_LINKER_BLOCK_H_
#define PICKC_LINKER_BLOCK_H_

#include <memory>
#include <vector>
#include <variant>

#include "variable.h"
#include "operator.h"

namespace pickc::linker {
  struct _Block;
  using Block = std::shared_ptr<_Block>;
  using WeakBlock = std::weak_ptr<_Block>;

  struct NonTerminalBlock {
    WeakBlock nextBlock;
  };
  struct TerminalBlock {
    WeakVariable result;
  };
  struct ConditionalBranchBlock {
    WeakVariable cond;
    WeakBlock thenBlock;
    WeakBlock elseBlock;
  };
  struct _Block {
    std::vector<Operator> operators;
    std::variant<NonTerminalBlock, TerminalBlock, ConditionalBranchBlock> variant;
  };
}

#endif // PICKC_LINKER_BLOCK_H_