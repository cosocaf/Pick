/**
 * @file fn_generator.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 関数を生成する。
 * @version 0.1
 * @date 2021-07-28
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_PCIR_FN_GENERATOR_H_
#define PICKC_PCIR_FN_GENERATOR_H_

#include <optional>

#include "fn_section.h"
#include "type_section.h"
#include "variable.h"

#include "parser/ast/node.h"

namespace pickc::pcir {
  class PCIRGenerator;

  /**
   * @brief 関数ジェネレータ。
   * parser::FunctionNodeから、PCIR用の関数に変換する。
   * 
   */
  class FnGenerator {
    PCIRGenerator* pcirGenerator;
    parser::FunctionNode fnNode;
  public:
    /**
     * @brief FnGeneratorを構築する。
     * 
     * @param pcirGenerator 生成した関数が記録されるPCIR用のPCIRジェネレータ
     * @param fnNode 生成する関数のノード
     */
    FnGenerator(PCIRGenerator* pcirGenerator, const parser::FunctionNode& fnNode);
    /**
     * @brief 関数の宣言部を解析する。
     * これは、PCIRGeneratorで、関数自身をシンボルとして記録するために使用される。
     * 
     * @return Type この関数の型
     */
    Type declare();
    /**
     * @brief 関数本文を解析する。
     * 
     * @param fn 解析結果を格納するオブジェクト
     */
    void generate(Fn& fn);
  private:
    /**
     * @brief 式を解析する。
     * 
     * @param fn 生成する関数
     * @param curBlock 現在のブロック
     * @param formula 解析する式
     * @return Variable 解析結果を格納している変数
     */
    Variable analyzeFormula(Fn& fn, CodeBlock& curBlock, const parser::FormulaNode& formula);
    /**
     * @brief 文を解析する。
     * 
     * @param fn 生成する関数
     * @param curBlock 現在のコードブロック
     * @param statement 解析する文
     * @return std::optional<Variable> 文が式であった場合はその式の結果。そうでない場合はnullopt 
     */
    std::optional<Variable> analyzeStatement(Fn& fn, CodeBlock& curBlock, const parser::StatementNode& statement);
    /**
     * @brief void型変数を作成する。
     * 
     * @param fn 生成する関数
     * @return Variable 作成したvoid型変数
     */
    Variable createVoid(Fn& fn);
  };
}

#endif // PICKC_PCIR_FN_GENERATOR_H_