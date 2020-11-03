#ifndef PICKC_WINDOWS_X64_ROUTINE_COMPILER_H_
#define PICKC_WINDOWS_X64_ROUTINE_COMPILER_H_

#include <unordered_map>

#include "utils/result.h"
#include "utils/binary_vec.h"
#include "utils/option.h"
#include "ssa/ssa_struct.h"

#include "routine.h"

namespace pickc::windows::x64
{
  enum struct RegisterInfo
  {
    // ルーチン中で一度も使用していない。
    Unused,
    // ルーチン中で現在使用中。
    InUse,
    // ルーチン中で使用したことがある。
    Used,
  };
  class RoutineCompiler
  {
    /**
     * pickcが保存用レジスタとして使うレジスタ。
     * RAX, RDXはMul, Divで使用する。
    */
    static constexpr auto useRegs = {
      Register::RCX,
      Register::RBX,
      Register::R8,
      Register::R9,
      Register::R10,
      Register::R11,
      Register::RSI,
      Register::RDI,
      Register::R12,
      Register::R13,
      Register::R14,
      Register::R15
    };
    /**
     * useRegsの中で、関数の呼び出し後に保存されないレジスタ。
     * これらが関数呼び出し前に使用されている場合にスタックに避難させる。
    */
    static constexpr auto volatileRegs = {
      Register::RCX,
      Register::R8,
      Register::R9,
      Register::R10,
      Register::R11
    };
    Routine* routine;
    size_t curLifeTime;
    std::unordered_map<Register, RegisterInfo> regInfo;
    std::unordered_map<ssa::SSARegister*, Operand> regs;
    Operand createOperand();
    void saveRegs();
    void freeRegs();
    OperationSize getSize(const pcir::TypeSection* type);
  public:
    RoutineCompiler(const ssa::SSAFunction& ssaFn);
    Result<Routine*, std::vector<std::string>> compile();
  };
}

#endif // PICKC_WINDOWS_X64_ROUTINE_COMPILER_H_