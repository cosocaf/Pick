/**
 * @file assembly.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-10-01
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "assembly.h"

#include <cassert>

namespace pickc::linker::windows_x64 {
  Immediate::Immediate(int64_t value) : value(value) {}
  Memory::Memory(Register base) : base(base), index(std::nullopt), displacement(0) {}
  Memory::Memory(Register base, int32_t displacement) : base(base), index(std::nullopt), displacement(displacement) {}
  Memory::Memory(Register base, Register index, Scale scale) : base(base), index(index), scale(scale), displacement(0) {}
  Memory::Memory(Register base, Register index, Scale scale, int32_t displacement) : base(base), index(index), scale(scale), displacement(displacement) {}
  Operand::Operand(Register reg) : rm(reg) {}
  Operand::Operand(Immediate imm) : rm(imm) {}
  Operand::Operand(const Memory& mem) : rm(mem) {}
  Assembly::Assembly(OperandSize operandSize) : operandSize(operandSize) {}
  Assembly::~Assembly() {}
  BinaryVec Assembly::modRM(const Operand& op1, const Operand& op2) {
    assert(!std::holds_alternative<Immediate>(op1.rm) && !std::holds_alternative<Immediate>(op2.rm));
    BinaryVec res;
    if(std::holds_alternative<Register>(op1.rm)) {
      uint8_t modRM = 0;

      const auto& reg = std::get<Register>(op1.rm);
      if(reg >= Register::R8) {
        modRM |= (static_cast<uint8_t>(reg) - 8) << 3;
      }
      else {
        modRM |= static_cast<uint8_t>(reg) << 3;
      }
      if(std::holds_alternative<Register>(op2.rm)) {
        const auto& rm = std::get<Register>(op2.rm);
        if(rm >= Register::R8) {
          modRM |= (static_cast<uint8_t>(rm) - 8);
        }
        else {
          modRM |= static_cast<uint8_t>(rm);
        }

        modRM |= 0b11'000'000;
        res << modRM;
      }
      else if(std::holds_alternative<Memory>(op2.rm)) {
        uint8_t SIB = 0;

        const auto& rm = std::get<Memory>(op2.rm);

        if(rm.displacement == 0) {
          modRM |= 0b00'000'000;
        }
        else if(rm.displacement >= INT8_MIN && rm.displacement <= INT8_MAX) {
          modRM |= 0b01'000'000;
        }
        else {
          modRM |= 0b10'000'000;
        }

        if(rm.index.has_value()) {
          // SIBのindexにRSPは指定できない。
          assert(rm.index.value() != Register::RSP);
          modRM |= static_cast<uint8_t>(Register::RSP);
          SIB |= static_cast<uint8_t>(rm.base);
          SIB |= static_cast<uint8_t>(rm.index.value()) << 3;
          SIB |= static_cast<uint8_t>(rm.scale);
        }
        else if(rm.base == Register::RSP) {
          modRM |= static_cast<uint8_t>(Register::RSP);
          SIB |= static_cast<uint8_t>(Register::RSP);
          SIB |= static_cast<uint8_t>(Register::RSP) << 3;
          SIB |= static_cast<uint8_t>(Scale::x1);
        }
        else {
          modRM |= static_cast<uint8_t>(rm.base);
        }

        res << modRM;
        if(SIB != 0) res << SIB;
        if(rm.displacement == 0) {
          // do nothing.
        }
        else if(rm.displacement >= INT8_MIN && rm.displacement <= INT8_MAX) {
          res << static_cast<int8_t>(rm.displacement);
        }
        else {
          res << rm.displacement;
        }
      }
      else {
        // TODO
        assert(false);
      }
    }
    else if(std::holds_alternative<Memory>(op1.rm)) {
      assert(std::holds_alternative<Register>(op2.rm));
      return modRM(op2, op1);
    }
    else {
      // TODO
      assert(false);
    }
    return res;
  }

  Add::Add(OperandSize operandSize, const Operand& dst, const Operand& src) : Assembly(operandSize), dst(dst), src(src) {
    assert(!std::holds_alternative<Immediate>(dst.rm));
    assert(!std::holds_alternative<Memory>(dst.rm) || !std::holds_alternative<Memory>(src.rm));
  }
  BinaryVec Add::toBinaryVec() {
    BinaryVec res;
    uint8_t rex = 0;
    if(operandSize == OperandSize::QWord) {
      rex |= 0b1000;
    }
    if(std::holds_alternative<Immediate>(src.rm)) {
      auto imm = std::get<Immediate>(src.rm);
      if(std::holds_alternative<Register>(dst.rm) && std::get<Register>(dst.rm) == Register::RAX) {
        switch(operandSize) {
          case OperandSize::Byte:
            res.push_back(0x04);
            res << static_cast<int8_t>(imm.value);
            break;
          case OperandSize::Word:
            res.push_back(0x05);
            res << static_cast<int16_t>(imm.value);
            break;
          case OperandSize::DWord:
            res.push_back(0x05);
            res << static_cast<int32_t>(imm.value);
            break;
          case OperandSize::QWord:
            res.push_back(0x05);
            res << static_cast<int32_t>(imm.value);
            break;
        }
      }
      else {
        if(operandSize == OperandSize::Byte) {
          res.push_back(0x83);
          res << modRM(Register::RAX, dst)
              << static_cast<int8_t>(imm.value);
        }
        else {
          res.push_back(0x81);
          res << modRM(Register::RAX, dst)
              << static_cast<int32_t>(imm.value);
        }
      }
    }
    else if(std::holds_alternative<Register>(dst.rm)) {
      if(operandSize == OperandSize::Byte) {
        res.push_back(0x02);
      }
      else {
        res.push_back(0x03);
      }
      res << modRM(dst, src);
    }
    else {
      if(operandSize == OperandSize::Byte) {
        res.push_back(0x00);
      }
      else {
        res.push_back(0x01);
      }
      res << modRM(dst, src);
    }

    if(rex) {
      res.insert(res.begin(), 0x40 | rex);
    }
    if(operandSize == OperandSize::Word) {
      res.insert(res.begin(), 0x66);
    }
    return res;
  }

  Sub::Sub(OperandSize operandSize, const Operand& dst, const Operand& src) : Assembly(operandSize), dst(dst), src(src) {
    assert(!std::holds_alternative<Immediate>(dst.rm));
    assert(!std::holds_alternative<Memory>(dst.rm) || !std::holds_alternative<Memory>(src.rm));
  }
  BinaryVec Sub::toBinaryVec() {
    BinaryVec res;
    uint8_t rex = 0;
    if(operandSize == OperandSize::QWord) {
      rex |= 0b1000;
    }
    if(std::holds_alternative<Immediate>(src.rm)) {
      auto imm = std::get<Immediate>(src.rm);
      if(std::holds_alternative<Register>(dst.rm) && std::get<Register>(dst.rm) == Register::RAX) {
        switch(operandSize) {
          case OperandSize::Byte:
            res.push_back(0x2C);
            res << static_cast<int8_t>(imm.value);
            break;
          case OperandSize::Word:
            res.push_back(0x2D);
            res << static_cast<int16_t>(imm.value);
            break;
          case OperandSize::DWord:
            res.push_back(0x2D);
            res << static_cast<int32_t>(imm.value);
            break;
          case OperandSize::QWord:
            res.push_back(0x2D);
            res << static_cast<int32_t>(imm.value);
            break;
        }
      }
      else {
        if(operandSize == OperandSize::Byte) {
          res.push_back(0x83);
          res << modRM(Register::RBP, dst)
              << static_cast<int8_t>(imm.value);
        }
        else {
          res.push_back(0x81);
          res << modRM(Register::RBP, dst)
              << static_cast<int32_t>(imm.value);
        }
      }
    }
    else if(std::holds_alternative<Register>(dst.rm)) {
      if(operandSize == OperandSize::Byte) {
        res.push_back(0x2A);
      }
      else {
        res.push_back(0x2B);
      }
      res << modRM(dst, src);
    }
    else {
      if(operandSize == OperandSize::Byte) {
        res.push_back(0x28);
      }
      else {
        res.push_back(0x29);
      }
      res << modRM(dst, src);
    }

    if(rex) {
      res.insert(res.begin(), 0x40 | rex);
    }
    if(operandSize == OperandSize::Word) {
      res.insert(res.begin(), 0x66);
    }
    return res;
  }

  Mov::Mov(OperandSize operandSize, const Operand& dst, const Operand& src) : Assembly(operandSize), dst(dst), src(src) {
    assert(!std::holds_alternative<Immediate>(dst.rm));
    assert(!std::holds_alternative<Memory>(dst.rm) || !std::holds_alternative<Memory>(src.rm));
  }
  BinaryVec Mov::toBinaryVec() {
    BinaryVec res;
    uint8_t rex = 0;
    if(operandSize == OperandSize::QWord) {
      rex |= 0b1000;
    }
    if(std::holds_alternative<Immediate>(src.rm)) {
      const auto& imm = std::get<Immediate>(src.rm);
      if(std::holds_alternative<Register>(dst.rm)) {
        const auto& reg = std::get<Register>(dst.rm);
        switch(operandSize) {
          case OperandSize::Byte:
            res.push_back(0xB0 | static_cast<uint8_t>(reg));
            res << static_cast<int8_t>(imm.value);
            break;
          case OperandSize::Word:
            res.push_back(0xB8 | static_cast<uint8_t>(reg));
            res << static_cast<int16_t>(imm.value);
            break;
          case OperandSize::DWord:
            res.push_back(0xB8 | static_cast<uint8_t>(reg));
            res << static_cast<int32_t>(imm.value);
            break;
          case OperandSize::QWord:
            res.push_back(0xB8 | static_cast<uint8_t>(reg));
            res << static_cast<int64_t>(imm.value);
            break;
          default:
            assert(false);
        }
      }
      else if(std::holds_alternative<Memory>(dst.rm)) {
        assert(operandSize != OperandSize::QWord);
        if(operandSize == OperandSize::Byte) {
          res.push_back(0xC6);
          res << modRM(Register::RAX, dst)
              << static_cast<int8_t>(imm.value);
        }
        else {
          res.push_back(0xC7);
          res << modRM(Register::RAX, dst)
              << static_cast<int32_t>(imm.value);
        }
      }
      else {
        // TODO
        assert(false);
      }
    }
    else if(std::holds_alternative<Register>(dst.rm)) {
      if(operandSize == OperandSize::Byte) {
        res.push_back(0x8A);
      }
      else {
        res.push_back(0x8B);
      }
      res << modRM(dst, src);
    }
    else {
      if(operandSize == OperandSize::Byte) {
        res.push_back(0x88);
      }
      else {
        res.push_back(0x89);
      }
      res << modRM(dst, src);
    }

    if(rex) {
      res.insert(res.begin(), 0x40 | rex);
    }
    if(operandSize == OperandSize::Word) {
      res.insert(res.begin(), 0x66);
    }
    return res;
  }

  Push::Push(OperandSize operandSize, const Operand& op) : Assembly(operandSize), op(op) {
    assert(operandSize == OperandSize::Word || operandSize == OperandSize::QWord);
  };
  BinaryVec Push::toBinaryVec() {
    BinaryVec res;
    if(std::holds_alternative<Register>(op.rm)) {
      const auto& reg = std::get<Register>(op.rm);
      if(reg >= Register::R8) {
        res.push_back(0x41);
        res.push_back(0x50 | (static_cast<uint8_t>(reg) - 8));
      }
      else {
        res.push_back(0x50 | static_cast<uint8_t>(reg));
      }
    }
    else {
      // TODO
      assert(false);
    }
    return res;
  }
  Pop::Pop(OperandSize operandSize, const Operand& op) : Assembly(operandSize), op(op) {
    assert(operandSize == OperandSize::Word || operandSize == OperandSize::QWord);
  }
  BinaryVec Pop::toBinaryVec() {
    BinaryVec res;
    if(std::holds_alternative<Register>(op.rm)) {
      const auto& reg = std::get<Register>(op.rm);
      if(reg >= Register::R8) {
        res.push_back(0x41);
        res.push_back(0x58 | (static_cast<uint8_t>(reg) - 8));
      }
      else {
        res.push_back(0x58 | static_cast<uint8_t>(reg));
      }
    }
    else {
      // TODO
      assert(false);
    }
    return res;
  }
  Ret::Ret() : Assembly(OperandSize::QWord) {}
  BinaryVec Ret::toBinaryVec() {
    return BinaryVec{ 0xC3 };
  }
}