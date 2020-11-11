#include "routine.h"

namespace pickc::windows::x64
{
  MovXOperation::MovXOperation(OperationSize distSize, OperationSize srcSize, Register dist, Operand src) : Operation(distSize), distSize(distSize), srcSize(srcSize), dist(dist), src(src) {
    assert(distSize > srcSize);
  }
  BinaryVec MovXOperation::movxTemplate(Routine* routine, uint8_t opb, uint8_t opv)
  {
    assert(srcSize <= OperationSize::Word);

    if(src.type == OperandType::Memory && src.memory.base && src.memory.base.get() == Register::RBP && src.memory.needAddressFix) {
      src.memory.disp += routine->baseDiff;
    }
    
    BinaryVec code;
    if(distSize == OperationSize::Word) code.push_back(0x66);

    uint8_t rex = 0;
    if(distSize == OperationSize::QWord) rex |= REXW;
    if(Register::R8 <= dist && Register::R15 >= dist) rex |= REXR;

    BinaryVec opcode;
    BinaryVec operand;
    
    opcode.push_back(0x0F);
    if(srcSize == OperationSize::Byte) opcode.push_back(opb);
    else opcode.push_back(opv);

    if(src.type == OperandType::Register) {
      if(src.reg >= Register::R8 && src.reg <= Register::R15) rex |= REXB;
      operand.push_back(0b11'000'000 | modRM(dist, src.reg));
    }
    else if(src.type == OperandType::Memory) {
      if(src.memory.base.get() >= Register::R8 && src.memory.base.get() <= Register::R15) rex |= REXB;
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
        operand.push_back(mod | modRM(dist, Register::RSP));
        operand.push_back((src.memory.index << 6) | modRM(src.memory.scale.get(), src.memory.base.get()));
      }
      else {
        operand.push_back(mod | modRM(dist, src.memory.base.get()));
        if(src.memory.base.get() == Register::RSP || src.memory.base.get() == Register::R12) operand.push_back(0x24);
      }
      if(src.memory.disp || src.memory.base.get() == Register::RBP || src.memory.base.get() == Register::R13) {
        if(in8bit(src.memory.disp)) operand << static_cast<uint8_t>(src.memory.disp);
        else operand << src.memory.disp;
      }
    }
    else {
      assert(false);
    }

    if(rex) code << static_cast<uint8_t>(REX | rex);
    code << opcode << operand;
    return code;
  }

  BinaryVec MovSXOperation::bin(WindowsX64& x64, Routine* routine)
  {
    return movxTemplate(routine, 0xBE, 0xBF);
  }
  BinaryVec MovZXOperation::bin(WindowsX64& x64, Routine* routine)
  {
    return movxTemplate(routine, 0xBE, 0xBF);
  }

  MovSXDOperation::MovSXDOperation(Register dist, Operand src) : Operation(OperationSize::QWord), dist(dist), src(src) {}

  BinaryVec MovSXDOperation::bin(WindowsX64& x64, Routine* routine)
  {
    if(src.type == OperandType::Memory && src.memory.base && src.memory.base.get() == Register::RBP && src.memory.needAddressFix) {
      src.memory.disp += routine->baseDiff;
    }
    
    BinaryVec code;

    uint8_t rex = REXW;
    if(Register::R8 <= dist && Register::R15 >= dist) rex |= REXR;

    BinaryVec opcode;
    BinaryVec operand;
    
    opcode.push_back(0x63);

    if(src.type == OperandType::Register) {
      if(src.reg >= Register::R8 && src.reg <= Register::R15) rex |= REXB;
      operand.push_back(0b11'000'000 | modRM(dist, src.reg));
    }
    else if(src.type == OperandType::Memory) {
      if(src.memory.base.get() >= Register::R8 && src.memory.base.get() <= Register::R15) rex |= REXB;
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
        operand.push_back(mod | modRM(dist, Register::RSP));
        operand.push_back((src.memory.index << 6) | modRM(src.memory.scale.get(), src.memory.base.get()));
      }
      else {
        operand.push_back(mod | modRM(dist, src.memory.base.get()));
        if(src.memory.base.get() == Register::RSP || src.memory.base.get() == Register::R12) operand.push_back(0x24);
      }
      if(src.memory.disp || src.memory.base.get() == Register::RBP || src.memory.base.get() == Register::R13) {
        if(in8bit(src.memory.disp)) operand << static_cast<uint8_t>(src.memory.disp);
        else operand << src.memory.disp;
      }
    }
    else if(src.type == OperandType::Relocation)
    {
      operand.push_back(0x04);
      operand.push_back(0x25);
      operand << 0;
    }
    else {
      assert(false);
    }

    code << static_cast<uint8_t>(REX | rex) << opcode << operand;
    if(src.type == OperandType::Relocation) {
      x64.relocs.insert(new RelocationInfo(routine, src.reloc, this, code.size() - 4, OperationSize::DWord, RelocationPosition::Absolute));
    }
    return code;
  }
}