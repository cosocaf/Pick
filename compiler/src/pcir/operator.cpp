/**
 * @file byte_code.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-07-28
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "operator.h"

#include "fn_section.h"

namespace pickc::pcir {
  AddOperator::AddOperator(const Variable& res, const Variable& op1, const Variable& op2) : res(res), op1(op1), op2(op2) {}
  void AddOperator::write(BinaryVec& out) const {
    out << opecode << res.getIndex() << op1.getIndex() << op2.getIndex();
  }

  Imm32Operator::Imm32Operator(const Variable& res, int32_t imm32) : res(res), imm32(imm32) {}
  void Imm32Operator::write(BinaryVec& out) const {
    out << opecode << res.getIndex() << imm32;
  }

  LoadFnOperator::LoadFnOperator(const Variable& res, const WeakFn& fn) : res(res), fn(fn) {}
  void LoadFnOperator::write(BinaryVec& out) const {
    out << opecode << res.getIndex() << fn.lock()->getIndex();
  }
}