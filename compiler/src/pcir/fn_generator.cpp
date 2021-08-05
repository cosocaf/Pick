/**
 * @file fn_generator.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief fn_generator.hの実装
 * @version 0.1
 * @date 2021-07-28
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "fn_generator.h"

#include "generator.h"

#include "utils/instanceof.h"
#include "utils/dyn_cast.h"
#include "output_buffer.h"

namespace pickc::pcir {
  FnGenerator::FnGenerator(PCIRGenerator* pcirGenerator, const parser::FunctionNode& fnNode) :
    pcirGenerator(pcirGenerator),
    fnNode(fnNode) {}
  Type FnGenerator::declare() {
    // TODO
    return pcirGenerator->createType(FnType(
      pcirGenerator->createType(LangDefinedTypes::I32),
      {}
    ));
  }
  void FnGenerator::generate(Fn& fn) {
    auto entryBlock = fn->getEntryBlock();
    analyzeFormula(fn, entryBlock, fnNode->body);
  }
  Variable FnGenerator::analyzeFormula(Fn& fn, CodeBlock& curBlock, const parser::FormulaNode& formula) {
    if(instanceof<parser::BlockNode>(formula)) {
      auto block = dynCast<parser::BlockNode>(formula);
      auto nextBlock = fn->addBlock();
      curBlock->toNonTerminalBlock(std::weak_ptr(nextBlock));
      std::optional<Variable> var;
      for(const auto& statement : block->statements) {
        var = analyzeStatement(fn, nextBlock, statement);
      }
      if(var) {
        return var.value();
      }
      else {
        return createVoid(fn);
      }
    }
    else if(instanceof<parser::IntegerNode>(formula)) {
      auto integer = dynCast<parser::IntegerNode>(formula);
      auto var = fn->addVariable(pcirGenerator->createType(LangDefinedTypes::I32));
      curBlock->emplace<Imm32Operator>(var, integer->value);
      return var;
    }
    else {
      // TODO:
      assert(false);
    }
  }
  std::optional<Variable> FnGenerator::analyzeStatement(Fn& fn, CodeBlock& curBlock, const parser::StatementNode& statement) {
    if(instanceof<parser::FormulaNode>(statement)) {
      return analyzeFormula(fn, curBlock, dynCast<parser::FormulaNode>(statement));
    }
    
    if(instanceof<parser::ControlNode>(statement)) {
      auto control = dynCast<parser::ControlNode>(statement);
      switch(control->kind) {
        case parser::ControlNodeKind::Return: {
          std::optional<Variable> ret;
          if(control->formula) {
            ret = analyzeFormula(fn, curBlock, control->formula);
          }
          else {
            ret = createVoid(fn);
          }
          curBlock->toTerminalBlock(ret.value());
          break;
        }
        case parser::ControlNodeKind::Break:
          // TODO
          break;
        case parser::ControlNodeKind::Continue:
          // TODO
          break;
      }
    }
    return std::nullopt;
  }
  Variable FnGenerator::createVoid(Fn& fn) {
    return fn->addVariable(pcirGenerator->createType(LangDefinedTypes::Void));
  }
}