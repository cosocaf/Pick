#include "routine.h"

namespace pickc::windows::x64
{
  BinaryVec MovOperation::bin(WindowsX64& x64, Routine* routine)
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
        if(size == OperationSize::Byte) opcode.push_back(0x88);
        else opcode.push_back(0x89);
        operand.push_back(0b11'000'000 | modRM(src.reg, dist.reg));
      }
      else if(src.type == OperandType::Memory) {
        if(src.reg >= Register::R8 && src.reg <= Register::R15) rex |= REXB;
        if(dist.reg >= Register::R8 && dist.reg <= Register::R15) rex |= REXR;
        if(size == OperationSize::Byte) opcode.push_back(0x8A);
        else opcode.push_back(0x8B);
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
        if(size == OperationSize::Byte) {
          opcode.push_back(0xB0 | modRM(Register::RAX, dist.reg));
          operand << static_cast<uint8_t>(src.imm);
        }
        else {
          opcode.push_back(0xB8 | modRM(Register::RAX, dist.reg));
          if(size == OperationSize::Word) operand << static_cast<int16_t>(src.imm);
          else if(size == OperationSize::DWord) operand << static_cast<int32_t>(src.imm);
          else if(src.imm <= INT32_MAX && src.imm >= INT32_MIN) {
            rex &= ~REXW;
            operand << static_cast<int32_t>(src.imm);
          }
          else operand << src.imm;
        }
      }
      else if(src.type == OperandType::Relocation) {
        if(dist.reg >= Register::R8 && dist.reg <= Register::R15) rex |= REXB;
        opcode.push_back(0xB8 | modRM(Register::RAX, dist.reg));
        x64.relocs.insert(new RelocationInfo(routine, src.reloc, this, 2, OperationSize::QWord, RelocationPosition::Absolute));
        opcode << 0ll;
      }
      else {
        assert(false);
      }
    }
    else if(dist.type == OperandType::Memory) {
      if(src.type == OperandType::Register) {
        if(src.reg >= Register::RSP && src.reg <= Register::RDI) rex |= REX;
        if(dist.memory.base.get() >= Register::R8 && dist.memory.base.get() <= Register::R15) rex |= REXR;
        if(src.reg >= Register::R8 && src.reg <= Register::R15) rex |= REXB;
        if(size == OperationSize::Byte) opcode.push_back(0x88);
        else opcode.push_back(0x89);
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
      else if(src.type == OperandType::Immediate) {
        if(size == OperationSize::Byte) opcode.push_back(0xC6);
        else opcode.push_back(0xC7);
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
          operand.push_back(mod | modRM(Register::RAX, Register::RSP));
          operand.push_back((dist.memory.index << 6) | modRM(dist.memory.scale.get(), dist.memory.base.get()));
        }
        else {
          operand.push_back(mod | modRM(Register::RAX, dist.memory.base.get()));
          if(dist.memory.base.get() == Register::RSP || dist.memory.base.get() == Register::R12) operand.push_back(0x24);
        }
        if(dist.memory.disp || dist.memory.base.get() == Register::RBP || dist.memory.base.get() == Register::R13) {
          if(in8bit(dist.memory.disp)) operand << static_cast<uint8_t>(dist.memory.disp);
          else operand << dist.memory.disp;
        }
        if(size == OperationSize::Byte) operand << static_cast<uint8_t>(src.imm);
        else if(size == OperationSize::Word) operand << static_cast<int16_t>(src.imm);
        else if(size == OperationSize::DWord) operand << static_cast<int32_t>(src.imm);
        // qword ptr [address], 64bit imm はできない。
        // 一度レジスタにmovして、それをmovする。
        else assert(false);
      }
      else {
        assert(false);
      }
    }
    else {
      assert(false);
    }

    if(rex) code << static_cast<uint8_t>(REX | rex);
    code << opcode << operand;
    return code;
  }
}