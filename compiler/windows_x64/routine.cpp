#include "routine.h"

namespace pickc::windows::x64
{
  Operation::Operation(OperationSize size) : size(size) {}
  Operation::~Operation() {}
  uint8_t Operation::modRM(Register reg, Register rm)
  {
    uint8_t modRM = 0;
    switch(reg) {
      case Register::RAX:
      case Register::R8:
        modRM |= 0b00'000'000;
        break;
      case Register::RCX:
      case Register::R9:
        modRM |= 0b00'001'000;
        break;
      case Register::RDX:
      case Register::R10:
        modRM |= 0b00'010'000;
        break;
      case Register::RBX:
      case Register::R11:
        modRM |= 0b00'011'000;
        break;
      case Register::RSP:
      case Register::R12:
        modRM |= 0b00'100'000;
        break;
      case Register::RBP:
      case Register::R13:
        modRM |= 0b00'101'000;
        break;
      case Register::RSI:
      case Register::R14:
        modRM |= 0b00'110'000;
        break;
      case Register::RDI:
      case Register::R15:
        modRM |= 0b00'111'000;
        break;
      default:
        assert(false);
    }
    switch(rm) {
      case Register::RAX:
      case Register::R8:
        modRM |= 0b00'000'000;
        break;
      case Register::RCX:
      case Register::R9:
        modRM |= 0b00'000'001;
        break;
      case Register::RDX:
      case Register::R10:
        modRM |= 0b00'000'010;
        break;
      case Register::RBX:
      case Register::R11:
        modRM |= 0b00'000'011;
        break;
      case Register::RSP:
      case Register::R12:
        modRM |= 0b00'000'100;
        break;
      case Register::RBP:
      case Register::R13:
        modRM |= 0b00'000'101;
        break;
      case Register::RSI:
      case Register::R14:
        modRM |= 0b00'000'110;
        break;
      case Register::RDI:
      case Register::R15:
        modRM |= 0b00'000'111;
        break;
      default:
        assert(false);
    }
    return modRM;
  }
  bool Operation::in8bit(int64_t value)
  {
    return value <= INT8_MAX && value >= INT8_MIN;
  }
  Memory::Memory(Register base, size_t numBytes, bool needAddressFix) : base(some(base)), scale(none), index(0), disp(0), numBytes(numBytes), needAddressFix(needAddressFix) {}
  Memory::Memory(Register base, int32_t disp, size_t numBytes, bool needAddressFix) : base(some(base)), scale(none), index(0), disp(disp), numBytes(numBytes), needAddressFix(needAddressFix) {}
  Operand::Operand() : type(OperandType::_NONE) {}
  Operand::Operand(Register reg) : type(OperandType::Register), reg(reg) {}
  Operand::Operand(int64_t imm) : type(OperandType::Immediate), imm(imm) {}
  Operand::Operand(const Memory& memory) : type(OperandType::Memory), memory(memory)
  {
    if(memory.scale) assert(memory.scale.get() != Register::RSP);
  }
  Operand::Operand(const Relocation& reloc) : type(OperandType::Relocation), reloc(reloc) {}
  Operand::Operand(const Operand& operand) : type(operand.type)
  {
    switch(type) {
      case OperandType::_NONE:
        break;
      case OperandType::Register:
        reg = operand.reg;
        break;
      case OperandType::Immediate:
        imm = operand.imm;
        break;
      case OperandType::Memory:
        new (&memory) Memory(operand.memory);
        break;
      case OperandType::Relocation:
        new (&reloc) Relocation(operand.reloc);
        break;
      default:
        assert(false);
    }
  }
  Operand::~Operand()
  {
    clear();
  }
  Operand& Operand::operator=(const Operand& operand)
  {
    clear();
    type = operand.type;
    switch(type) {
      case OperandType::_NONE:
        break;
      case OperandType::Register:
        reg = operand.reg;
        break;
      case OperandType::Immediate:
        imm = operand.imm;
        break;
      case OperandType::Memory:
        new (&memory) Memory(operand.memory);
        break;
      case OperandType::Relocation:
        new (&reloc) Relocation(operand.reloc);
        break;
      default:
        assert(false);
    }
    return *this;
  }
  void Operand::clear()
  {
    if(type == OperandType::Memory) {
      memory.~Memory();
    }
    type = OperandType::Register;
  }
  Relocation::Relocation(pcir::SymbolSection* symbol) : type(RelocationType::Symbol), symbol(symbol) {}
  Relocation::Relocation(pcir::FunctionSection* fn) : type(RelocationType::Function), fn(fn) {}
  Relocation::Relocation(const Relocation& reloc) : type(reloc.type)
  {
    switch(type) {
      case RelocationType::_NONE:
        break;
      case RelocationType::Symbol:
        symbol = reloc.symbol;
        break;
      case RelocationType::Function:
        fn = reloc.fn;
        break;
      default:
        assert(false);
    }
  }
  Relocation& Relocation::operator=(const Relocation& reloc)
  {
    type = reloc.type;
    switch(type) {
      case RelocationType::_NONE:
        break;
      case RelocationType::Symbol:
        symbol = reloc.symbol;
        break;
      case RelocationType::Function:
        fn = reloc.fn;
        break;
      default:
        assert(false);
    }
    return *this;
  }
  Relocation::~Relocation() {}
  RelocationInfo::RelocationInfo(Routine* routine, const Relocation& reloc, Operation* op, size_t index, OperationSize size, RelocationPosition pos) : routine(routine), reloc(reloc), op(op), index(index), size(size), pos(pos) {}
}