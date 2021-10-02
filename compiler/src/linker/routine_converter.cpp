/**
 * @file routine_converter.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-09-27
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "routine_converter.h"

#include <cassert>

#include "opecode.h"

namespace pickc::linker {
  RoutineConverter::RoutineConverter(const bundler::WeakBundle& bundle, const bundler::InternalFunction& function) : bundle(bundle), function(function) {}
  Routine RoutineConverter::convert() {
    for(const auto& fnVar : function.variables) {
      auto var = std::make_shared<_Variable>();
      var->type = fnVar;
      routine.variables.push_back(var);
    }

    routine.blocks.resize(function.blocks.size());
    for(auto& block : routine.blocks) block = std::make_shared<_Block>();

    for(size_t i = 0; i < routine.blocks.size(); ++i) {
      const auto& fnBlock = function.blocks[i];
      auto& block = routine.blocks[i];

      block->operators = analyzeBytecode(fnBlock.byteCode);
      switch(fnBlock.kind) {
        // NonTerminalBlock
        case 0x0001: {
          NonTerminalBlock b;
          b.nextBlock = routine.blocks[std::get<bundler::PCIRNonTerminalBlock>(fnBlock.variant).nextBlock];
          block->variant = b;
          break;
        }
        // TerminalBlock
        case 0x0002: {
          TerminalBlock b;
          b.result = routine.variables[std::get<bundler::PCIRTerminalBlock>(fnBlock.variant).returnValue];
          block->variant = b;
          break;
        }
        // ConditionalBranchBlock
        case 0x0003: {
          const auto& pcirBlock = std::get<bundler::PCIRConditionalBranchBlock>(fnBlock.variant);
          ConditionalBranchBlock b;
          b.cond = routine.variables[pcirBlock.conditionalValue];
          b.thenBlock = routine.blocks[pcirBlock.thenBlock];
          b.elseBlock = routine.blocks[pcirBlock.elseBlock];
          block->variant = b;
          break;
        }
      }
    }

    return routine;
  }

  std::vector<Operator> RoutineConverter::analyzeBytecode(const BinaryVec& bytecode) {
    std::vector<Operator> operators;
    size_t i = 0;
    size_t l = bytecode.size();
    while(i < l) {
      auto opecode = static_cast<Opecode>(get8(bytecode, i));
      switch(opecode) {
        case Opecode::ADD: {
          uint32_t res = get32(bytecode, i);
          uint32_t op1 = get32(bytecode, i);
          uint32_t op2 = get32(bytecode, i);
          operators.emplace_back(AddOperator(routine.variables[res], routine.variables[op1], routine.variables[op2]));
          break;
        }
        case Opecode::SUB: {
          uint32_t res = get32(bytecode, i);
          uint32_t op1 = get32(bytecode, i);
          uint32_t op2 = get32(bytecode, i);
          operators.emplace_back(SubOperator(routine.variables[res], routine.variables[op1], routine.variables[op2]));
          break;
        }
        case Opecode::Imm32: {
          uint32_t res = get32(bytecode, i);
          int32_t imm32 = get32(bytecode, i);
          operators.emplace_back(Imm32Operator(routine.variables[res], imm32));
          break;
        }
        case Opecode::LoadFn: {
          uint32_t res = get32(bytecode, i);
          uint32_t imm32 = get32(bytecode, i);
          operators.emplace_back(LoadFnOperator(routine.variables[res], imm32));
          break;
        }
        default:
          //TODO
          assert(false);
      }
    }

    return operators;
  }
}