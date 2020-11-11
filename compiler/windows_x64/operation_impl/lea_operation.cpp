#include "routine.h"

namespace pickc::windows::x64
{
  LeaOperation::LeaOperation(OperationSize size, Register dist, Memory src) : Operation(size), dist(dist), src(src)
  {
    assert(size != OperationSize::Byte);
  }
  BinaryVec LeaOperation::bin(WindowsX64& x64, Routine* routine)
  {
    if(src.base && src.base.get() == Register::RBP && src.needAddressFix) {
      src.disp += routine->baseDiff;
    }

    BinaryVec code;

    if(size == OperationSize::Word) code.push_back(0x66);

    uint8_t rex = 0;
    if(size == OperationSize::QWord) rex |= REXW;

    BinaryVec opcode;
    BinaryVec operand;

    if(src.base.get() >= Register::R8 && src.base.get() <= Register::R15) rex |= REXB;
    if(dist >= Register::R8 && dist <= Register::R15) rex |= REXR;
    opcode.push_back(0x8D);
    uint8_t mod = 0;
    if(src.disp) {
      if(in8bit(src.disp)) mod |= 0x40;
      else mod |= 0x80;
    }
    else if(src.base.get() == Register::RBP || src.base.get() == Register::R13) {
      mod |= 0x40;
    }
    if(src.scale) {
      if(src.scale.get() >= Register::R8 && src.scale.get() <= Register::R15) rex |= REXX;
      operand.push_back(mod | modRM(dist, Register::RSP));
      operand.push_back((src.index << 6) | modRM(src.scale.get(), src.base.get()));
    }
    else {
      operand.push_back(mod | modRM(dist, src.base.get()));
      if(src.base.get() == Register::RSP || src.base.get() == Register::R12) operand.push_back(0x24);
    }
    if(src.disp || src.base.get() == Register::RBP || src.base.get() == Register::R13) {
      if(in8bit(src.disp)) operand << static_cast<uint8_t>(src.disp);
      else operand << src.disp;
    }

    if(rex) code << static_cast<uint8_t>(REX | rex);
    code << opcode << operand;
    return code;
  }
}