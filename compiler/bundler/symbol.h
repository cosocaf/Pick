#ifndef PICKC_BUNDLER_SYMBOL_H_
#define PICKC_BUNDLER_SYMBOL_H_

#include "utils/binary_vec.h"
#include "pcir/pcir_struct.h"

#include "function.h"

namespace pickc::bundler
{
  enum struct SymbolInitType
  {
    _NONE,
    // 解析段階中で、PCIRの関数を持っているだけ
    PCIR,
    // 整数リテラルなどの即値
    Immediate,
    // 関数アドレス
    Function,
    // 実行時(invokeMain)初期化
    Runtime,
  };
  struct Symbol
  {
    SymbolInitType initType;
    union {
      pcir::FunctionSection* pcirFn;
      BinaryVec imm;
      Function* fn;
    };
    explicit Symbol(pcir::FunctionSection* pcirFn);
    explicit Symbol(const BinaryVec& imm);
    explicit Symbol(SymbolInitType type, Function* fn);
    Symbol(const Symbol& symbol);
    ~Symbol();
    Symbol& operator=(const Symbol& symbol);
  private:
    void clear();
  };
}

#endif // PICKC_BUNDLER_SYMBOL_H_