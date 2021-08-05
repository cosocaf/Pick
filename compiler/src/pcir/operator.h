/**
 * @file operator.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-07-28
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_PCIR_OPERATOR_H_
#define PICKC_PCIR_OPERATOR_H_

#include <memory>

#include "variable.h"

#include "utils/binary_vec.h"

namespace pickc::pcir {
  class _Fn;
  using WeakFn = std::weak_ptr<_Fn>;

  class Operator {
  public:
    virtual ~Operator() = default;
    virtual void write(BinaryVec& out) const = 0;
  };
  class AddOperator : public Operator {
    static constexpr uint8_t opecode = 0x00;
    Variable res;
    Variable op1;
    Variable op2;
  public:
    AddOperator(const Variable& res, const Variable& op1, const Variable& op2);
    virtual void write(BinaryVec& out) const override;
  };
  class Imm32Operator : public Operator {
    static constexpr uint8_t opecode = 0x82;
    Variable res;
    int32_t imm32;
  public: 
    Imm32Operator(const Variable& res, int32_t imm32);
    virtual void write(BinaryVec& out) const override;
  };
  class LoadFnOperator : public Operator {
    static constexpr uint8_t opecode = 0x90;
    Variable res;
    WeakFn fn;
  public:
    LoadFnOperator(const Variable& res, const WeakFn& fn);
    virtual void write(BinaryVec& out) const override;
  };
}

#endif // PICKC_PCIR_OPERATOR_H_