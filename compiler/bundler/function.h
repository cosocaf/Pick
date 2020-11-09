#ifndef PICKC_BUNDLER_FUNCTION_H_
#define PICKC_BUNDLER_FUNCTION_H_

#include <vector>
#include <unordered_set>

#include "pcir/pcir_struct.h"

namespace pickc::bundler
{
  struct Register {
    pcir::TypeSection* type;
  };
  struct Instruction
  {
    virtual ~Instruction() = 0;
  };
  struct BinaryInstruction : public Instruction
  {
    Register* dist;
    Register* left;
    Register* right;
  };
  struct AddInstruction : public BinaryInstruction {};
  struct SubInstruction : public BinaryInstruction {};
  struct MulInstruction : public BinaryInstruction {};
  struct DivInstruction : public BinaryInstruction {};
  struct ModInstruction : public BinaryInstruction {};
  struct EqInstruction : public BinaryInstruction {};
  struct NeqInstruction : public BinaryInstruction {};
  struct GtInstruction : public BinaryInstruction {};
  struct GeInstruction : public BinaryInstruction {};
  struct LtInstruction : public BinaryInstruction {};
  struct LeInstruction : public BinaryInstruction {};
  struct LoadFnInstruction : public Instruction
  {
    Register* dist;
    pcir::FunctionSection* fn;
  };
  struct LoadArgInstruction : public Instruction
  {
    Register* dist;
    uint32_t indexOfArg;
  };
  struct LoadSymbolInstruction : public Instruction
  {
    Register* dist;
    pcir::SymbolSection* symbol;
  };
  struct CallInstruction : public Instruction
  {
    Register* dist;
    std::vector<Register*> args;
    Register* fn;
  };
  struct ImmInstruction : public Instruction
  {
    Register* dist;
    int64_t imm;
  };
  struct RetInstruction : public Instruction
  {
    // voidの場合はnullptr
    Register* value;
  };
  struct JmpInstruction : public Instruction
  {
    // Function::insts上の条件が真か無条件の場合にジャンプしたい命令への絶対インデックス
    size_t then;
    // Function::insts上の条件が偽の場合にジャンプしたい命令への絶対インデックス
    size_t els;
    // ジャンプする条件。無条件ジャンプならnullptr
    Register* cond;
  };
  

  struct PhiGroup
  {
    std::unordered_set<Register*> regs;
  };
  struct Function
  {
    pcir::TypeSection* type;
    std::unordered_set<Register*> regs;
    std::unordered_map<Register*, PhiGroup*> phis;
    std::vector<Instruction*> insts;
  };
}

#endif // PICKC_BUNDLER_FUNCTION_H_