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
  Variable FnGenerator::analyzeFormula(Fn& fn, CodeBlock& curBlock, const parser::FormulaNode& formula, std::optional<std::string> varName) {
    if(instanceof<parser::BlockNode>(formula)) {
      auto block = dynCast<parser::BlockNode>(formula);
      auto nextBlock = fn->addBlock();
      curBlock->toNonTerminalBlock(std::weak_ptr(nextBlock));
      std::optional<Variable> var;
      for(const auto& statement : block->statements) {
        var = analyzeStatement(fn, nextBlock, statement, varName);
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
      std::optional<Variable> var;
      if(varName) {
        var = fn->addVariable(varName.value(), pcirGenerator->createType(LangDefinedTypes::I32));
      }
      else {
        var = fn->addVariable(pcirGenerator->createType(LangDefinedTypes::I32));
      }
      curBlock->emplace<Imm32Operator>(var.value(), integer->value);
      return var.value();
    }
    else if(instanceof<parser::VariableNode>(formula)) {
      auto var = dynCast<parser::VariableNode>(formula);
      auto find = fn->findVariable(var->name);
      if(!find) {
        // TODO: return error.
        assert(false);
      }
      else {
        return find.value();
      }
    }
    else {
      // TODO:
      assert(false);
    }
  }
  std::optional<Variable> FnGenerator::analyzeStatement(Fn& fn, CodeBlock& curBlock, const parser::StatementNode& statement, std::optional<std::string> varName) {
    if(instanceof<parser::FormulaNode>(statement)) {
      return analyzeFormula(fn, curBlock, dynCast<parser::FormulaNode>(statement), varName);
    }
    else if(instanceof<parser::ControlNode>(statement)) {
      auto control = dynCast<parser::ControlNode>(statement);
      switch(control->kind) {
        case parser::ControlNodeKind::Return: {
          std::optional<Variable> ret;
          if(control->formula) {
            ret = analyzeFormula(fn, curBlock, control->formula, varName);
          }
          else {
            ret = createVoid(fn);
          }
          curBlock->toTerminalBlock(ret.value());
          break;
        }
        case parser::ControlNodeKind::Break:
          // TODO
          assert(false);
          break;
        case parser::ControlNodeKind::Continue:
          // TODO
          assert(false);
          break;
      }
    }
    else if(instanceof<parser::VariableDeclarationNode>(statement)) {
      auto varDecl = dynCast<parser::VariableDeclarationNode>(statement);
      analyzeFormula(fn, curBlock, varDecl->value, varDecl->name);
    }
    else {
      // TODO
      assert(false);
    }
    return std::nullopt;
  }
  Variable FnGenerator::createVoid(Fn& fn) {
    return fn->addVariable(pcirGenerator->createType(LangDefinedTypes::Void));
  }
}