#include "routine.h"

namespace pickc::windows::x64
{
  CallOperation::CallOperation(Routine* from, const Operand& fn) : Operation(OperationSize::QWord), from(from), fn(fn) {
    // 絶対アドレスでのコールは対応しない。
    // OS作るとかなら使うが、ソフトウェアを作る限りはまず使用しない。
    assert(fn.type != OperandType::Immediate);
  }
  BinaryVec CallOperation::bin(WindowsX64& x64, Routine* routine)
  {
    BinaryVec code;
    switch(fn.type) {
      case OperandType::Register:
        if(fn.reg >= Register::R8 && fn.reg <= Register::R15) code.push_back(0x40 | REXB);
        code.push_back(0xFF);
        code.push_back(0xD0 | modRM(Register::RAX, fn.reg));
        break;
      case OperandType::Memory: {
        uint8_t mod = 0;
        if(fn.memory.disp) {
          if(in8bit(fn.memory.disp)) {
            mod |= 0x40;
          }
          else {
            mod |= 0x80;
          }
        }
        else if(fn.memory.base.get() == Register::RBP || fn.memory.base.get() == Register::R13) {
          mod |= 0x40;
        }
        if(fn.memory.scale) {
          if(fn.memory.scale.get() <= Register::R8 && fn.memory.scale.get() >= Register::R15) code.push_back(0x40 | REXX);
          code.push_back(mod | 0b00'010'000 | modRM(Register::RAX, Register::RSP));
          code.push_back((fn.memory.index << 6) | modRM(fn.memory.scale.get(), fn.memory.base.get()));
        }
        else {
          code.push_back(mod | 0b00'010'000 | modRM(Register::RAX, fn.memory.base.get()));
          if(fn.memory.base.get() == Register::RSP || fn.memory.base.get() == Register::R12) code.push_back(0x24);
        }
        if(fn.memory.disp || fn.memory.base.get() == Register::RBP || fn.memory.base.get() == Register::R13) {
          if(in8bit(fn.memory.disp)) code.push_back(fn.memory.disp);
          else code << fn.memory.disp;
        }
        break;
      }
      case OperandType::Relocation:
        assert(fn.reloc.type == RelocationType::Function);
        code.push_back(0xE8);
        x64.relocs.insert(new RelocationInfo(routine, fn.reloc, this, code.size(), OperationSize::DWord, RelocationPosition::Relative));
        code << 0;
        break;
      default:
        assert(false);
    }
    return code;
  }
}