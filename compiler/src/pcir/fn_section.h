/**
 * @file fn.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-07-28
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_PCIR_FN_SECTION_H_
#define PICKC_PCIR_FN_SECTION_H_

#include <vector>
#include <memory>
#include <variant>

#include "type_section.h"
#include "variable.h"
#include "operator.h"

#include "utils/binary_vec.h"

namespace pickc::pcir {
  class _FnSection;
  class _Fn;
  class _CodeBlock;
  using FnSection = std::shared_ptr<_FnSection>;
  using WeakFnSection = std::weak_ptr<_FnSection>;
  using Fn = std::shared_ptr<_Fn>;
  using WeakFn = std::weak_ptr<_Fn>;
  using CodeBlock = std::shared_ptr<_CodeBlock>;
  using WeakCodeBlock = std::weak_ptr<_CodeBlock>;

  enum struct BlockKind : uint16_t {
    NonTerminal = 0x01,
    Terminal = 0x02,
    ConditionalBranch = 0x03,
  };
  struct NonTerminalBlock {
    WeakCodeBlock nextBlock;
    NonTerminalBlock(const WeakCodeBlock& nextBlock);
  };
  struct TerminalBlock {
    Variable returnValue;
    TerminalBlock(const Variable& returnValue);
  };
  struct ConditionalBranchBlock {
    Variable conditional;
    WeakCodeBlock thenBlock;
    WeakCodeBlock elseBlock;
  };
  class _CodeBlock : public std::enable_shared_from_this<_CodeBlock> {
    WeakFn fn;
    uint32_t index;
    std::vector<std::unique_ptr<Operator>> operators;
    std::variant<void*, NonTerminalBlock, TerminalBlock, ConditionalBranchBlock> blockVariant;
  public:
    _CodeBlock(const WeakFn& fn, uint32_t index);
    template<typename Operator, typename... Vals>
    void emplace(Vals&&... vals) {
      operators.emplace_back(
        std::make_unique<Operator>(std::forward<Vals>(vals)...)
      );
    }
    void toNonTerminalBlock(const NonTerminalBlock& nonTerminalBlock);
    void toTerminalBlock(const TerminalBlock& terminalBlock);
    void toConditionalBranchBlock(const ConditionalBranchBlock& conditional);
    uint32_t getIndex() const;
    BinaryVec toBinaryVec() const;
  };

  class _Fn : public std::enable_shared_from_this<_Fn>{
    WeakFnSection fnSection;
    uint32_t index;
    Type type;
    std::vector<CodeBlock> codeBlocks;
    CodeBlock entryBlock;
    VariableTable variableTable;
  public:
    _Fn(const WeakFnSection& fnSection, uint32_t index, const Type& type);
    CodeBlock getEntryBlock();
    CodeBlock addBlock();
    Variable addVariable(const std::string& name, const Type& type);
    Variable addVariable(const Type& type);
    
    uint32_t getIndex() const;
    Type getType() const;
    VariableTable getVariableTable() const;
    uint32_t getCodeBlocksSize() const;
    std::vector<CodeBlock>::iterator begin();
    std::vector<CodeBlock>::iterator end();
    std::vector<CodeBlock>::const_iterator begin() const;
    std::vector<CodeBlock>::const_iterator end() const;
  };

  class _FnSection : public std::enable_shared_from_this<_FnSection> {
    std::vector<Fn> fns;
  private:
    _FnSection() = default;
  public:
    static FnSection create();
    Fn createFn(const Type& type);
    BinaryVec toBinaryVec() const;
  };
}

#endif // PICKC_PCIR_FN_SECTION_H_