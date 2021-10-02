/**
 * @file operator.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-09-29
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "operator.h"

namespace pickc::linker {
  AddOperator::AddOperator(const WeakVariable& res, const WeakVariable& op1, const WeakVariable& op2) : res(res), op1(op1), op2(op2) {}
  Imm32Operator::Imm32Operator(const WeakVariable& res, int32_t imm32) : res(res), imm32(imm32) {}
  LoadFnOperator::LoadFnOperator(const WeakVariable& res, int32_t fn) : res(res), fn(fn) {}
}