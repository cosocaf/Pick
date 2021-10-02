/**
 * @file routine_compiler.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-09-29
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "routine_compiler.h"

#include <cassert>

namespace pickc::linker::windows_x64 {
  RoutineCompiler::RoutineCompiler(const Routine& routine) : routine(routine) {}
  BinaryVec RoutineCompiler::compile() {
    std::vector<std::shared_ptr<Assembly>> assembly;

    int32_t memoryOffset = 0;
    for(auto& var : routine.variables) {
      var->memoryArea = requiredAllocatedMemoryArea(var->type);
      var->memoryOffset = memoryAlignment(memoryOffset + var->memoryArea, var->memoryArea);
      memoryOffset += var->memoryOffset;
    }
    memoryOffset = memoryAlignment(memoryOffset, 16);

    assembly.push_back(std::make_shared<Push>(OperandSize::QWord, Register::RBP));
    assembly.push_back(std::make_shared<Mov>(OperandSize::QWord, Register::RBP, Register::RSP));
    assembly.push_back(std::make_shared<Sub>(OperandSize::QWord, Register::RSP, Immediate(memoryOffset)));

    for(const auto& block : routine.blocks) {
      for(const auto& op : block->operators) {
        if(std::holds_alternative<AddOperator>(op)) {
          auto add = std::get<AddOperator>(op);
          const auto& res = add.res.lock();
          const auto& op1 = add.op1.lock();
          const auto& op2 = add.op2.lock();
          auto opSize = getOperandSize(res->memoryArea);
          assembly.push_back(std::make_shared<Mov>(opSize, Register::RAX, Memory(Register::RBP, -op1->memoryOffset)));
          assembly.push_back(std::make_shared<Add>(opSize, Register::RAX, Memory(Register::RBP, -op2->memoryOffset)));
          assembly.push_back(std::make_shared<Mov>(opSize, Memory(Register::RBP, -res->memoryOffset), Register::RAX));
        }
        else if(std::holds_alternative<Imm32Operator>(op)) {
          auto imm32 = std::get<Imm32Operator>(op);
          const auto& res = imm32.res.lock();
          assembly.push_back(std::make_shared<Mov>(getOperandSize(res->memoryArea), Memory(Register::RBP, -res->memoryOffset), Immediate(imm32.imm32)));
        }
        else if(std::holds_alternative<LoadFnOperator>(op)) {
          // TODO
          //assert(false);
        }
      }
      if(std::holds_alternative<TerminalBlock>(block->variant)) {
        const auto& terminalBlock = std::get<TerminalBlock>(block->variant);
        auto result = terminalBlock.result.lock();
        assembly.push_back(std::make_shared<Mov>(getOperandSize(result->memoryArea), Register::RAX, Memory(Register::RBP, -result->memoryOffset)));
        assembly.push_back(std::make_shared<Add>(OperandSize::QWord, Register::RSP, Immediate(memoryOffset)));
        assembly.push_back(std::make_shared<Pop>(OperandSize::QWord, Register::RBP));
        assembly.push_back(std::make_shared<Ret>());
      }
    }

    BinaryVec res;
    for(const auto& op : assembly) {
      res << op->toBinaryVec();
    }
    return res;
  }
  int32_t RoutineCompiler::requiredAllocatedMemoryArea(const bundler::TypeInfo& type) {
    if(std::holds_alternative<bundler::TypeLangDef>(type.variant)) {
      auto langDef = std::get<bundler::TypeLangDef>(type.variant);
      switch(langDef) {
        case bundler::TypeLangDef::I8: return 1;
        case bundler::TypeLangDef::I16: return 2;
        case bundler::TypeLangDef::I32: return 4;
        case bundler::TypeLangDef::I64: return 8;
        case bundler::TypeLangDef::ISize: return 8;
        case bundler::TypeLangDef::U8: return 1;
        case bundler::TypeLangDef::U16: return 2;
        case bundler::TypeLangDef::U32: return 4;
        case bundler::TypeLangDef::U64: return 8;
        case bundler::TypeLangDef::USize: return 8;
        case bundler::TypeLangDef::F32: return 4;
        case bundler::TypeLangDef::F64: return 8;
        case bundler::TypeLangDef::Void: return 0;
        case bundler::TypeLangDef::Bool: return 1;
        default:
          assert(false);
          return 0;
      }
    }
    else if(std::holds_alternative<bundler::TypeFunction>(type.variant)) {
      return 8;
    }
    else {
      assert(false);
      return 0;
    }
  }
  int32_t RoutineCompiler::memoryAlignment(int32_t offset, int32_t size) {
    if(size <= 1) return offset;
    else if(size <= 2) return (offset + 1) / 2 * 2;
    else if(size <= 4) return (offset + 3) / 4 * 4;
    else if(size <= 8) return (offset + 7) / 8 * 8;
    else return (offset + 15) / 16 * 16;
  }
  OperandSize RoutineCompiler::getOperandSize(int32_t size) {
    switch(size) {
      case 1: return OperandSize::Byte;
      case 2: return OperandSize::Word;
      case 4: return OperandSize::DWord;
      case 8: return OperandSize::QWord;
      default: 
        assert(false);
        return OperandSize::DWord;
    }
  }
}