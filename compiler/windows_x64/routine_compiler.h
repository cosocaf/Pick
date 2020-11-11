#ifndef PICKC_WINDOWS_X64_ROUTINE_COMPILER_H_
#define PICKC_WINDOWS_X64_ROUTINE_COMPILER_H_

#include <unordered_map>

#include "utils/result.h"
#include "utils/binary_vec.h"
#include "utils/option.h"
#include "bundler/function.h"

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
     * pickcが値の保存用レジスタとして使うレジスタ。
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
    /**
     * useRegsの中で、関数の呼び出しによって変化しないレジスタ。
     * これらを関数内で使用した場合、関数の終了時に値を復元する必要がある。
    */
    static constexpr auto nonvolatileRegs = {
      Register::RBX,
      Register::RSI,
      Register::RDI,
      Register::R12,
      Register::R13,
      Register::R14,
      Register::R15
    };
    static constexpr auto argRegs = {
      Register::RCX,
      Register::RDX,
      Register::R8,
      Register::R9
    };
    WindowsX64* x64;
    Routine* routine;
    std::vector<Operation*> prologue;
    std::vector<Operation*> body;
    std::vector<Operation*> epilogue;
    // スタックの空き状況
    // バイト単位での使用状況を保持し、
    // trueなら使用中、falseなら不使用を表す。
    std::vector<bool> stackStatus;
    std::unordered_map<bundler::Register*, Operand> regs;
    std::unordered_map<Register, RegisterInfo> regInfo;
    // std::unordered_map<ssa::SSARegister*, Operand> regs;
    std::vector<Operand> args;
    // 現在のスタックの伸び
    int32_t stack;
    // 最も伸びたスタックの長さ
    int32_t minStack;
    size_t lifetime;
    Operand createOperand();
    // スタックを確保する。
    // スタック領域を線形探索し、numBytes以上の領域が使用可能であればその領域を指すメモリを返す。
    // 空き領域がなければスタックを伸ばし、その領域を指すメモリを返す。
    // 現状フラグメンテーションは無視する。
    Memory allocStack(size_t numBytes);
    void saveRegs();
    void freeRegs();
    OperationSize getSize(const pcir::TypeSection* type);
  public:
    RoutineCompiler(bundler::Function* fn, WindowsX64* x64);
    Result<Routine*, std::vector<std::string>> compile();
  };
}

#endif // PICKC_WINDOWS_X64_ROUTINE_COMPILER_H_