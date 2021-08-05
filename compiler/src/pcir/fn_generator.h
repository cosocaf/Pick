/**
 * @file fn_generator.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-07-28
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_PCIR_FN_GENERATOR_H_
#define PICKC_PCIR_FN_GENERATOR_H_

#include <optional>
#include <map>

#include "fn_section.h"
#include "type_section.h"
#include "variable.h"

#include "parser/ast/node.h"

namespace pickc::pcir {
  class PCIRGenerator;
  class FnGenerator {
    PCIRGenerator* pcirGenerator;
    parser::FunctionNode fnNode;
  public:
    FnGenerator(PCIRGenerator* pcirGenerator, const parser::FunctionNode& fnNode);
    Type declare();
    void generate(Fn& fn);
  private:
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
    Variable createVoid(Fn& fn);
  };
}

#endif // PICKC_PCIR_FN_GENERATOR_H_