/**
 * @file assembly.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-10-01
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_LINKER_WINDOWS_X64_ASSEMBLY_H_
#define PICKC_LINKER_WINDOWS_X64_ASSEMBLY_H_

#include <variant>
#include <optional>

#include "utils/binary_vec.h"

namespace pickc::linker::windows_x64 {
  /**
   * @brief 
   * 
   * レジスタ名がRから始まっているが、EAX, AXなどもRAXとする。
   */
  enum struct Register {
    RAX,
    RCX,
    RDX,
    RBX,
    RSP,
    RBP,
    RSI,
    RDI,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
  };
  struct Immediate {
    int64_t value;
    Immediate(int64_t value);
  };
  enum struct Scale : uint8_t {
    x1 = 0b00'000'000,
    x2 = 0b01'000'000,
    x4 = 0b10'000'000,
    x8 = 0b11'000'000,
  };
  struct Memory {
    Register base;
    std::optional<Register> index;
    Scale scale;
    int32_t displacement;
    Memory(Register base);
    Memory(Register base, int32_t displacement);
    Memory(Register base, Register index, Scale scale);
    Memory(Register base, Register index, Scale scale, int32_t displacement);
  };

  enum struct OperandSize {
    Byte,
    Word,
    DWord,
    QWord,
  };
  struct Operand {
    std::variant<Register, Immediate, Memory> rm;
    Operand(Register reg);
    Operand(Immediate imm);
    Operand::Operand(const Memory& mem);
  };
  class Assembly {
  protected:
    OperandSize operandSize;
    static BinaryVec modRM(const Operand& op1, const Operand& op2);
  public:
    Assembly(OperandSize operandSize);
    virtual ~Assembly();
    virtual BinaryVec toBinaryVec() = 0;
  };
  class Add : public Assembly {
    Operand dst;
    Operand src;
  public:
    Add(OperandSize operandSize, const Operand& dst, const Operand& src);
    virtual BinaryVec toBinaryVec();
  };
  class Sub : public Assembly {
    Operand dst;
    Operand src;
  public:
    Sub(OperandSize operandSize, const Operand& dst, const Operand& src);
    virtual BinaryVec toBinaryVec();
  };
  class Mov : public Assembly {
    Operand dst;
    Operand src;
  public:
    Mov(OperandSize operandSize, const Operand& dst, const Operand& src);
    virtual BinaryVec toBinaryVec();
  };
  class Push : public Assembly {
    Operand op;
  public:
    Push(OperandSize operandSize, const Operand& op);
    virtual BinaryVec toBinaryVec();
  };
  class Pop : public Assembly {
    Operand op;
  public:
    Pop(OperandSize operandSize, const Operand& op);
    virtual BinaryVec toBinaryVec();
  };
  class Ret : public Assembly {
  public:
    Ret();
    virtual BinaryVec toBinaryVec();
  };
}

#endif // PICKC_LINKER_WINDOWS_X64_ASSEMBLY_H_