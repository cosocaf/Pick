#include "routine.h"

namespace pickc::windows::x64
{
  MulDivOperation::MulDivOperation(OperationSize size, Operand right) : Operation(size), right(right) {}
  BinaryVec MulDivOperation::mulDivTemplate(Routine* routine, uint8_t op)
  {
    if(right.type == OperandType::Memory && right.memory.base && right.memory.base.get() == Register::RBP && right.memory.needAddressFix) {
      right.memory.disp += routine->baseDiff;
    }

    BinaryVec code;

    if(size == OperationSize::Word) code.push_back(0x66);

    uint8_t rex = 0;
    if(size == OperationSize::QWord) rex |= REXW;

    BinaryVec opcode;
    BinaryVec operand;

    if(right.type == OperandType::Register) {
      if(right.reg >= Register::R8 && right.reg <= Register::R15) rex |= REXB;
      if(size == OperationSize::Byte) opcode.push_back(0xF6);
      else opcode.push_back(0xF7);
      operand.push_back(0b11'000'000 | op | modRM(Register::RAX, right.reg));
    }
    else if(right.type == OperandType::Memory) {
      if(right.memory.base.get() >= Register::R8 && right.memory.base.get() <= Register::R15) rex |= REXB;
      if(size == OperationSize::Byte) opcode.push_back(0xF6);
      else opcode.push_back(0xF7);
      uint8_t mod = 0;
      if(right.memory.disp) {
        if(in8bit(right.memory.disp)) mod |= 0x40;
        else mod |= 0x80;
      }
      else if(right.memory.base.get() == Register::RBP || right.memory.base.get() == Register::R13) {
        mod |= 0x40;
      }
      if(right.memory.scale) {
        if(right.memory.scale.get() >= Register::R8 && right.memory.scale.get() <= Register::R15) rex |= REXX;
        operand.push_back(mod | op | modRM(Register::RAX, Register::RSP));
        operand.push_back((right.memory.index << 6) | modRM(right.memory.scale.get(), right.memory.base.get()));
      }
      else {
        operand.push_back(mod | op | modRM(Register::RAX, right.memory.base.get()));
        if(right.memory.base.get() == Register::RSP || right.memory.base.get() == Register::R12) operand.push_back(0x24);
      }
      if(right.memory.disp || right.memory.base.get() == Register::RBP || right.memory.base.get() == Register::R13) {
        if(in8bit(right.memory.disp)) operand << static_cast<uint8_t>(right.memory.disp);
        else operand << right.memory.disp;
      }
    }
    else {
      assert(false);
    }

    if(rex) code << static_cast<uint8_t>(REX | rex);

    code << opcode << operand;
    return code;
  }
  BinaryVec MulOperation::bin(WindowsX64& x64, Routine* routine)
  {
    return mulDivTemplate(routine, 0b00'100'000);
  }
  BinaryVec IMulOperation::bin(WindowsX64& x64, Routine* routine)
  {
    return mulDivTemplate(routine, 0b00'101'000);
  }
  BinaryVec DivOperation::bin(WindowsX64& x64, Routine* routine)
  {
    return mulDivTemplate(routine, 0b00'110'000);
  }
  BinaryVec IDivOperation::bin(WindowsX64& x64, Routine* routine)
  {
    return mulDivTemplate(routine, 0b00'111'000);
  }
}