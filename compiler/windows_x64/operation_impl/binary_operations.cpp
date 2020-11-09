#include "routine.h"

namespace pickc::windows::x64
{
  BinaryVec BinaryOperation::binaryOpTemplate(Routine* routine, uint8_t ebgb, uint8_t evgv, uint8_t gbeb, uint8_t gvev, uint8_t alib, uint8_t axiz, uint8_t immop)
  {
    BinaryVec code;

    if(dist.type == OperandType::Memory && dist.memory.base && dist.memory.base.get() == Register::RBP && dist.memory.needAddressFix) {
      dist.memory.disp += routine->baseDiff;
    }
    if(src.type == OperandType::Memory && src.memory.base && src.memory.base.get() == Register::RBP && src.memory.needAddressFix) {
      src.memory.disp += routine->baseDiff;
    }

    if(size == OperationSize::Word) code.push_back(0x66);

    uint8_t rex = 0;
    if(size == OperationSize::QWord) rex |= REXW;

    BinaryVec opcode;
    BinaryVec operand;

    if(dist.type == OperandType::Register) {
      if(size == OperationSize::Byte && dist.reg >= Register::RSP && dist.reg >= Register::RDI) rex |= REX;
      if(src.type == OperandType::Register) {
        if(dist.reg >= Register::R8 && dist.reg <= Register::R15) rex |= REXB;
        if(src.reg >= Register::R8 && src.reg <= Register::R15) rex |= REXR;
        if(size == OperationSize::Byte) opcode.push_back(ebgb);
        else opcode.push_back(evgv);
        operand.push_back(0b11'000'000 | modRM(src.reg, dist.reg));
      }
      else if(src.type == OperandType::Memory) {
        if(src.reg >= Register::R8 && src.reg <= Register::R15) rex |= REXB;
        if(dist.reg >= Register::R8 && dist.reg <= Register::R15) rex |= REXR;
        if(size == OperationSize::Byte) opcode.push_back(gbeb);
        else opcode.push_back(gvev);
        if(src.memory.base) {
          uint8_t mod = 0;
          if(src.memory.disp) {
            if(in8bit(src.memory.disp)) mod |= 0x40;
            else mod |= 0x80;
          }
          else if(src.memory.base.get() == Register::RBP || src.memory.base.get() == Register::R13) {
            mod |= 0x40;
          }
          if(src.memory.scale) {
            if(src.memory.scale.get() >= Register::R8 && src.memory.scale.get() <= Register::R15) rex |= REXX;
            operand.push_back(mod | modRM(dist.reg, Register::RSP));
            operand.push_back((src.memory.index << 6) | modRM(src.memory.scale.get(), src.memory.base.get()));
          }
          else {
            operand.push_back(mod | modRM(dist.reg, src.memory.base.get()));
            if(src.memory.base.get() == Register::RSP || src.memory.base.get() == Register::R12) operand.push_back(0x24);
          }
          if(src.memory.disp || src.memory.base.get() == Register::RBP || src.memory.base.get() == Register::R13) {
            if(in8bit(src.memory.disp)) operand << static_cast<uint8_t>(src.memory.disp);
            else operand << src.memory.disp;
          }
        }
      }
      else if(src.type == OperandType::Immediate) {
        if(dist.reg >= Register::R8 && dist.reg <= Register::R15) rex |= REXB;
        bool isMin = false;
        if(dist.reg == Register::RAX && size == OperationSize::Byte) {
          opcode.push_back(alib);
        }
        else if(dist.reg == Register::RAX && size != OperationSize::Byte && in8bit(src.imm)) {
          opcode.push_back(axiz);
        }
        else {
          if(size == OperationSize::Byte) opcode.push_back(0x80);
          else if(in8bit(src.imm)) {
            opcode.push_back(0x83);
            isMin = true;
          }
          else opcode.push_back(0x81);
          opcode.push_back(0b11'000'000 | immop | modRM(Register::RAX, dist.reg));
        }
        if(size == OperationSize::Byte || isMin) operand << static_cast<int8_t>(src.imm);
        else if(size == OperationSize::Word) operand << static_cast<int16_t>(src.imm);
        else if(size == OperationSize::DWord) operand << static_cast<int32_t>(src.imm);
        else if(src.imm <= INT32_MAX && src.imm >= INT32_MIN) operand << static_cast<int32_t>(src.imm);
        else assert(false);
      }
      else {
        assert(false);
      }
    }
    else if(dist.type == OperandType::Memory) {
      if(src.type == OperandType::Register) {
        if(src.reg >= Register::RSP && src.reg <= Register::RDI) rex |= REX;
        if(dist.reg >= Register::R8 && dist.reg <= Register::R15) rex |= REXR;
        if(src.reg >= Register::R8 && src.reg <= Register::R15) rex |= REXB;
        if(size == OperationSize::Byte) opcode.push_back(ebgb);
        else opcode.push_back(evgv);
        uint8_t mod = 0;
        if(dist.memory.disp) {
          if(in8bit(dist.memory.disp)) mod |= 0x40;
          else mod |= 0x80;
        }
        else if(dist.memory.base.get() == Register::RBP || dist.memory.base.get() == Register::R13) {
          mod |= 0x40;
        }
        if(dist.memory.scale) {
          if(dist.memory.scale.get() >= Register::R8 && dist.memory.scale.get() <= Register::R15) rex |= REXX;
          operand.push_back(mod | modRM(src.reg, Register::RSP));
          operand.push_back((dist.memory.index << 6) | modRM(dist.memory.scale.get(), dist.memory.base.get()));
        }
        else {
          operand.push_back(mod | modRM(src.reg, dist.memory.base.get()));
          if(dist.memory.base.get() == Register::RSP || dist.memory.base.get() == Register::R12) operand.push_back(0x24);
        }
        if(dist.memory.disp || dist.memory.base.get() == Register::RBP || dist.memory.base.get() == Register::R13) {
          if(in8bit(dist.memory.disp)) operand << static_cast<uint8_t>(dist.memory.disp);
          else operand << dist.memory.disp;
        }
      }
      else {
        if(size == OperationSize::Byte) opcode.push_back(0x80);
        else opcode.push_back(0x81);
        uint8_t mod = 0;
        if(dist.memory.disp) {
          if(in8bit(dist.memory.disp)) mod |= 0x40;
          else mod |= 0x80;
        }
        else if(dist.memory.base.get() == Register::RBP || dist.memory.base.get() == Register::R13) {
          mod |= 0x40;
        }
        if(dist.memory.scale) {
          if(dist.memory.scale.get() >= Register::R8 && dist.memory.scale.get() <= Register::R15) rex |= REXX;
          operand.push_back(mod | immop | modRM(Register::RAX, Register::RSP));
          operand.push_back((dist.memory.index << 6) | modRM(dist.memory.scale.get(), dist.memory.base.get()));
        }
        else {
          operand.push_back(mod | immop | modRM(Register::RAX, dist.memory.base.get()));
          if(dist.memory.base.get() == Register::RSP || dist.memory.base.get() == Register::R12) operand.push_back(0x24);
        }
        if(dist.memory.disp || dist.memory.base.get() == Register::RBP || dist.memory.base.get() == Register::R13) {
          if(in8bit(dist.memory.disp)) operand << static_cast<uint8_t>(dist.memory.disp);
          else operand << dist.memory.disp;
        }
        if(size == OperationSize::Byte) operand << static_cast<int8_t>(src.imm);
        else if(size == OperationSize::Word) operand << static_cast<int16_t>(src.imm);
        else if(size == OperationSize::DWord) operand << static_cast<int32_t>(src.imm);
        else if(src.imm <= INT32_MAX && src.imm >= INT32_MIN) operand << static_cast<int32_t>(src.imm);
        else assert(false);
      }
    }
    else {
      assert(false);
    }

    if(rex) code << static_cast<uint8_t>(REX | rex);
    code << opcode << operand;
    return code;
  }

  BinaryOperation::BinaryOperation(OperationSize size, Operand dist, Operand src) : Operation(size), dist(dist), src(src) {
  }
  BinaryVec AddOperation::bin(WindowsX64& x64, Routine* routine)
  {
    return binaryOpTemplate(routine, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0b00'000'000);
  }
  BinaryVec SubOperation::bin(WindowsX64& x64, Routine* routine)
  {
    return binaryOpTemplate(routine, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0b00'101'000);
  }
  BinaryVec CmpOperation::bin(WindowsX64& x64, Routine* routine)
  {
    return binaryOpTemplate(routine, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0b00'111'000);
  }
}