/**
 * @file operator.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief PCIR中間コードの命令
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

#include "opecode.h"

namespace pickc::pcir {
  class _Fn;
  using WeakFn = std::weak_ptr<_Fn>;

  /**
   * @brief 全てのPCIR命令クラスのルートクラス。
   * 
   */
  class Operator {
  public:
    /**
     * @brief デストラクタ。
     * 
     */
    virtual ~Operator() = default;
    /**
     * @brief outに命令を出力する。
     * 
     * @param out 命令を出力するバッファ
     */
    virtual void write(BinaryVec& out) const = 0;
  };
  /**
   * @brief 加算命令。
   * res = op1 + op2;
   */
  class AddOperator : public Operator {
    static constexpr uint8_t opecode = static_cast<uint8_t>(Opecode::ADD);
    Variable res;
    Variable op1;
    Variable op2;
  public:
    /**
     * @brief AddOperatorを構築する。
     * 
     * @param res 計算結果
     * @param op1 左オペランド
     * @param op2 右オペランド
     */
    AddOperator(const Variable& res, const Variable& op1, const Variable& op2);
    virtual void write(BinaryVec& out) const override;
  };
  /**
   * @brief 32bit即値代入命令。
   * res = imm32;
   */
  class Imm32Operator : public Operator {
    static constexpr uint8_t opecode = static_cast<uint8_t>(Opecode::Imm32);
    Variable res;
    int32_t imm32;
  public: 
    /**
     * @brief Imm32Operatorを構築する。
     * 
     * @param res 代入先
     * @param imm32 代入元
     */
    Imm32Operator(const Variable& res, int32_t imm32);
    virtual void write(BinaryVec& out) const override;
  };
  /**
   * @brief 関数読込命令。
   * res = fn;
   */
  class LoadFnOperator : public Operator {
    static constexpr uint8_t opecode = static_cast<uint8_t>(Opecode::LoadFn);
    Variable res;
    WeakFn fn;
  public:
    /**
     * @brief LoadFnOperatorを構築する。
     * 
     * @param res 代入先
     * @param fn 代入元
     */
    LoadFnOperator(const Variable& res, const WeakFn& fn);
    virtual void write(BinaryVec& out) const override;
  };
}

#endif // PICKC_PCIR_OPERATOR_H_