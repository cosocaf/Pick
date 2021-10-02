/**
 * @file operator.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-08-11
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_LINKER_OPERATOR_H_
#define PICKC_LINKER_OPERATOR_H_

#include <variant>

#include "variable.h"

namespace pickc::linker {
  struct AddOperator {
    WeakVariable res;
    WeakVariable op1;
    WeakVariable op2;
    AddOperator(const WeakVariable& res, const WeakVariable& op1, const WeakVariable& op2);
  };
  struct SubOperator {
    WeakVariable res;
    WeakVariable op1;
    WeakVariable op2;
    SubOperator(const WeakVariable& res, const WeakVariable& op1, const WeakVariable& op2);
  };
  struct Imm32Operator {
    WeakVariable res;
    int32_t imm32;
    Imm32Operator(const WeakVariable& res, int32_t imm32);
  };
  struct LoadFnOperator {
    WeakVariable res;
    int32_t fn;
    LoadFnOperator(const WeakVariable& res, int32_t fn);
  };

  using Operator = std::variant<
    AddOperator,
    SubOperator,
    Imm32Operator,
    LoadFnOperator
  >;
}

#endif // PICKC_LINKER_OPERATOR_H_