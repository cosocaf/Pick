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
    analyzeExpression(fn, entryBlock, fnNode->body);
  }
  Variable FnGenerator::analyzeExpression(Fn& fn, CodeBlock& curBlock, const parser::ExpressionNode& expr, std::optional<std::string> varName) {
    if(instanceof<parser::BlockNode>(expr)) {
      auto block = dynCast<parser::BlockNode>(expr);
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
    else if(instanceof<parser::IntegerNode>(expr)) {
      auto integer = dynCast<parser::IntegerNode>(expr);
      auto var = createVariable(fn, varName, pcirGenerator->createType(LangDefinedTypes::I32));
      curBlock->emplace<Imm32Operator>(var, integer->value);
      return var;
    }
    else if(instanceof<parser::VariableNode>(expr)) {
      auto var = dynCast<parser::VariableNode>(expr);
      auto find = fn->findVariable(var->name);
      if(!find) {
        // TODO: return error.
        assert(false);
      }
      else {
        return find.value();
      }
    }
    else if(instanceof<parser::AddNode>(expr)) {
      auto add = dynCast<parser::AddNode>(expr);
      auto left = analyzeExpression(fn, curBlock, add->left);
      auto right = analyzeExpression(fn, curBlock, add->right);
      auto var = createVariable(fn, varName, left.getType());
      curBlock->emplace<AddOperator>(var, left, right);
      return var;
    }
    else if(instanceof<parser::SubNode>(expr)) {
      auto sub = dynCast<parser::SubNode>(expr);
      auto left = analyzeExpression(fn, curBlock, sub->left);
      auto right = analyzeExpression(fn, curBlock, sub->right);
      auto var = createVariable(fn, varName, left.getType());
      curBlock->emplace<SubOperator>(var, left, right);
      return var;
    }
    else {
      // TODO:
      assert(false);
    }
  }
  std::optional<Variable> FnGenerator::analyzeStatement(Fn& fn, CodeBlock& curBlock, const parser::StatementNode& statement, std::optional<std::string> varName) {
    if(instanceof<parser::ExpressionNode>(statement)) {
      return analyzeExpression(fn, curBlock, dynCast<parser::ExpressionNode>(statement), varName);
    }
    else if(instanceof<parser::ControlNode>(statement)) {
      auto control = dynCast<parser::ControlNode>(statement);
      switch(control->kind) {
        case parser::ControlNodeKind::Return: {
          std::optional<Variable> ret;
          if(control->expr) {
            ret = analyzeExpression(fn, curBlock, control->expr, varName);
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
      analyzeExpression(fn, curBlock, varDecl->value, varDecl->name);
    }
    else {
      // TODO
      assert(false);
    }
    return std::nullopt;
  }
  Variable FnGenerator::createVariable(Fn& fn, const std::optional<std::string> &name, const pickc::pcir::Type &type) {
    if(name) {
      return fn->addVariable(name.value(), pcirGenerator->createType(LangDefinedTypes::I32));
    }
    else {
      return fn->addVariable(pcirGenerator->createType(LangDefinedTypes::I32));
    }
  }
  Variable FnGenerator::createVoid(Fn& fn) {
    return fn->addVariable(pcirGenerator->createType(LangDefinedTypes::Void));
  }
}