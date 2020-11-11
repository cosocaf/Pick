#include "routine.h"

namespace pickc::windows::x64
{
  NegOperation::NegOperation(OperationSize size, Operand operand) : Operation(size), operand(operand) {}
  BinaryVec NegOperation::bin(WindowsX64& x64, Routine* routine)
  {
    if(operand.type == OperandType::Memory && operand.memory.base && operand.memory.base.get() == Register::RBP && operand.memory.needAddressFix) {
      operand.memory.disp += routine->baseDiff;
    }

    BinaryVec code;
    if(size == OperationSize::Word) code.push_back(0x66);

    uint8_t rex = 0;
    if(size == OperationSize::QWord) rex |= REXW;

    BinaryVec opcode;
    if(size == OperationSize::Byte) opcode.push_back(0xF6);
    else opcode.push_back(0xF7);

    BinaryVec op;
    if(operand.type == OperandType::Register) {
      if(operand.reg >= Register::R8 && operand.reg <= Register::R15) rex |= REXB;
      op.push_back(0b11'011'000 | modRM(Register::RAX, operand.reg));
    }
    else if(operand.type == OperandType::Memory) {
      if(operand.memory.base) {
        if(operand.memory.base.get() >= Register::R8 && operand.memory.base.get() <= Register::R15) rex |= REXB;
        uint8_t mod = 0b00'011'000;
        if(operand.memory.disp) {
          if(in8bit(operand.memory.disp)) mod |= 0x40;
          else mod |= 0x80;
        }
        else if(operand.memory.base.get() == Register::RBP || operand.memory.base.get() == Register::R13) {
          mod |= 0x40;
        }
        if(operand.memory.scale) {
          if(operand.memory.scale.get() >= Register::R8 && operand.memory.scale.get() <= Register::R15) rex |= REXX;
          op.push_back(mod | modRM(Register::RAX, Register::RSP));
          op.push_back((operand.memory.index << 6) | modRM(operand.memory.scale.get(), operand.memory.base.get()));
        }
        else {
          op.push_back(mod | modRM(Register::RAX, operand.memory.base.get()));
          if(operand.memory.base.get() == Register::RSP || operand.memory.base.get() == Register::R12) op.push_back(0x24);
        }
        if(operand.memory.disp || operand.memory.base.get() == Register::RBP || operand.memory.base.get() == Register::R13) {
          if(in8bit(operand.memory.disp)) op << static_cast<uint8_t>(operand.memory.disp);
          else op << operand.memory.disp;
        }
      }
    }
    else {
      assert(false);
    }

    if(rex != 0) rex |= REX;
    code << rex << opcode << op;
    return code;
  }
}