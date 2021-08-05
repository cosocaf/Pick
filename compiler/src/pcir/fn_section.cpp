/**
 * @file fn.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief fn_section.hの実装
 * @version 0.1
 * @date 2021-07-29
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "fn_section.h"

#include <cassert>

namespace pickc::pcir {
  NonTerminalBlock::NonTerminalBlock(const WeakCodeBlock& nextBlock) : nextBlock(nextBlock) {}
  TerminalBlock::TerminalBlock(const Variable& returnValue) : returnValue(returnValue) {}

  _CodeBlock::_CodeBlock(const WeakFn& fn, uint32_t index) : fn(fn), index(index) {}

  void _CodeBlock::toNonTerminalBlock(const NonTerminalBlock& nonTerminalBlock) {
    blockVariant = nonTerminalBlock;
  }
  void _CodeBlock::toTerminalBlock(const TerminalBlock& terminalBlock) {
    blockVariant = terminalBlock;
  }
  void _CodeBlock::toConditionalBranchBlock(const ConditionalBranchBlock& conditional) {
    blockVariant = conditional;
  }
  uint32_t _CodeBlock::getIndex() const {
    return index;
  }
  BinaryVec _CodeBlock::toBinaryVec() const {
    assert(!std::holds_alternative<void*>(blockVariant));

    BinaryVec bin;
    if(std::holds_alternative<NonTerminalBlock>(blockVariant)) {
      auto nonTerminalBlock = std::get<NonTerminalBlock>(blockVariant);
      bin << BlockKind::NonTerminal
          << static_cast<uint32_t>(0)
          << nonTerminalBlock.nextBlock.lock()->getIndex();
    }
    else if(std::holds_alternative<TerminalBlock>(blockVariant)) {
      auto terminalBlock = std::get<TerminalBlock>(blockVariant);
      bin << BlockKind::Terminal
          << static_cast<uint32_t>(0)
          << terminalBlock.returnValue.getIndex();
    }
    else if(std::holds_alternative<ConditionalBranchBlock>(blockVariant)) {
      auto conditional = std::get<ConditionalBranchBlock>(blockVariant);
      bin << BlockKind::ConditionalBranch
          << static_cast<uint32_t>(0)
          << conditional.conditional.getIndex()
          << conditional.thenBlock.lock()->getIndex()
          << conditional.elseBlock.lock()->getIndex();
    }
    else {
      assert(false);
    }

    BinaryVec byteCode;
    for(const auto& op : operators) {
      op->write(byteCode);
    }
    write(bin, 2, static_cast<uint32_t>(byteCode.size()));
    bin << byteCode;
    return bin;
  }

  _Fn::_Fn(const WeakFnSection& fnSection, uint32_t index, const Type& type) :
    fnSection(fnSection),
    index(index),
    type(type),
    variableTable(_VariableTable::create()) {}
  CodeBlock _Fn::getEntryBlock() {
    if(codeBlocks.empty()) {
      entryBlock = std::make_shared<_CodeBlock>(weak_from_this(), 0);
      codeBlocks.push_back(entryBlock);
    }
    return entryBlock;
  }
  CodeBlock _Fn::addBlock() {
    if(codeBlocks.empty()) return getEntryBlock();
    return codeBlocks.emplace_back(std::make_shared<_CodeBlock>(
      weak_from_this(),
      static_cast<uint32_t>(codeBlocks.size())
    ));
  }
  Variable _Fn::addVariable(const std::string& name, const Type& varType) {
    return variableTable->createVariable(varType, name);
  }
  Variable _Fn::addVariable(const Type& varType) {
    return variableTable->createAnonymousVariable(varType);
  }

  uint32_t _Fn::getIndex() const {
    return index;
  }
  Type _Fn::getType() const {
    return type;
  }
  VariableTable _Fn::getVariableTable() const {
    return variableTable;
  }
  uint32_t _Fn::getCodeBlocksSize() const {
    return static_cast<uint32_t>(codeBlocks.size());
  }
  std::vector<CodeBlock>::iterator _Fn::begin() {
    return codeBlocks.begin();
  }
  std::vector<CodeBlock>::iterator _Fn::end() {
    return codeBlocks.end();
  }
  std::vector<CodeBlock>::const_iterator _Fn::begin() const {
    return codeBlocks.begin();
  }
  std::vector<CodeBlock>::const_iterator _Fn::end() const {
    return codeBlocks.end();
  }

  FnSection _FnSection::create() {
    return FnSection(new _FnSection());
  }
  Fn _FnSection::createFn(const Type& type) {
    auto fn = std::make_shared<_Fn>(
      weak_from_this(),
      static_cast<uint32_t>(fns.size()),
      type
    );
    fns.push_back(fn);
    return fn;
  }

  BinaryVec _FnSection::toBinaryVec() const {
    BinaryVec bin;
    bin << static_cast<uint32_t>(0)
        << static_cast<uint32_t>(fns.size())
        << static_cast<uint32_t>(0);
    for(const auto& fn : fns) {
      auto varTable = fn->getVariableTable();
      bin << fn->getType()->getIndex()
          << varTable->size()
          << fn->getCodeBlocksSize();
      for(const auto& [name, var] : *varTable) {
        bin << var.getType()->getIndex();
      }
      for(const auto& block : *fn) {
        bin << block->toBinaryVec();
      }
    }
    write(bin, 0, static_cast<uint32_t>(bin.size()));
    return bin;
  }
}