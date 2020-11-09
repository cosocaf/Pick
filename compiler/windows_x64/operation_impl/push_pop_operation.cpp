#include "routine.h"

namespace pickc::windows::x64
{
  PushOperation::PushOperation(Operand value) : Operation(OperationSize::QWord), value(value) {}
  BinaryVec PushOperation::bin(WindowsX64& x64, Routine* routine)
  {
    if(value.type == OperandType::Memory && value.memory.base && value.memory.base.get() == Register::RBP && value.memory.needAddressFix) {
      value.memory.disp += routine->baseDiff;
    }

    uint8_t rex = 0;
    BinaryVec opcode;
    BinaryVec operand;

    switch(value.type) {
      case OperandType::Register:
        if(value.reg >= Register::R8 && value.reg <= Register::R15) {
          rex |= REXB;
        }
        opcode.push_back(0x50 | modRM(Register::RAX, value.reg));
        break;
      case OperandType::Memory: {
        if(value.memory.base.get() >= Register::R8 && value.memory.base.get() <= Register::R15) {
          rex |= REXB;
        }
        opcode.push_back(0xFF);
        uint8_t mod = 0;
        if(value.memory.disp) {
          if(in8bit(value.memory.disp)) {
            mod |= 0x40;
          }
          else {
            mod |= 0x80;
          }
        }
        else if(value.memory.base.get() == Register::RBP || value.memory.base.get() == Register::R13) {
          mod |= 0x40;
        }
        if(value.memory.scale) {
          if(value.memory.scale.get() <= Register::R8 && value.memory.scale.get() >= Register::R15) {
            rex |= REXX;
          }
          opcode.push_back(mod | 0b00'110'000 | modRM(Register::RAX, Register::RSP));
          operand.push_back((value.memory.index << 6) | modRM(value.memory.scale.get(), value.memory.base.get()));
        }
        else {
          opcode.push_back(mod | 0b00'110'000 | modRM(Register::RAX, value.memory.base.get()));
          if(value.memory.base.get() == Register::RSP || value.memory.base.get() == Register::R12) operand.push_back(0x24);
        }
        if(value.memory.disp || value.memory.base.get() == Register::RBP || value.memory.base.get() == Register::R13) {
          if(in8bit(value.memory.disp)) operand << static_cast<uint8_t>(value.memory.disp);
          else operand << value.memory.disp;
        }
        break;
      }
      case OperandType::Immediate:
        if(in8bit(value.imm)) {
          opcode.push_back(0x6A);
          opcode << static_cast<uint8_t>(value.imm);
        }
        else {
          opcode.push_back(0x68);
          opcode << value.imm;
        }
        break;
      default:
        assert(false);
    }
    BinaryVec code;
    if(rex) code << static_cast<uint8_t>(REX | rex);
    code << opcode;
    return code;
  }

  PopOperation::PopOperation(Operand dist) : Operation(OperationSize::QWord), dist(dist) {}
  BinaryVec PopOperation::bin(WindowsX64& x64, Routine* routine)
  {
    // TODO
    assert(false);
    return {};
  }
}