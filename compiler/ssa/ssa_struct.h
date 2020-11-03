#ifndef PICKC_SSA_SSA_STRUCT_H_
#define PICKC_SSA_SSA_STRUCT_H_

#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "pcir/pcir_struct.h"

namespace pickc::ssa
{
  struct SSARegister
  {
    /**
     * このレジスタが依存するレジスタの一覧。
     * 例えば
     * dist = left + right
     * の場合、distはleftとrightに依存する。
     * これにより、関数の戻り値に影響するレジスタを割り出すことができるため、
     * 関数の結果に関係ない代入などのデッドコードを排除できる。
     * ただし、副作用のある関数呼び出しなどがある場合、
     * 排除してはならないため注意する。
    */
    std::unordered_set<SSARegister*> dependents;
    /**
     * このレジスタが副作用を持った値を指しているかどうかのフラグ。
     * IOの結果を保持するレジスタなどに立つ。
     * このフラグはこのレジスタに依存するレジスタにも伝播する。
    */
    bool hasSideEffect;
    // このレジスタの型
    pcir::TypeSection* type;
    // このレジスタのライフタイムの始まりのSSAFunction::insts上のインデックス
    size_t lifeBegin;
    // このレジスタのライフタイムの終わりのSSAFunction::insts上のインデックス
    size_t lifeEnd;
  };
  struct SSAInstruction
  {
    virtual ~SSAInstruction() = 0;
  };
  struct SSAAddInstruction : public SSAInstruction
  {
    SSARegister* dist;
    SSARegister* left;
    SSARegister* right;
  };
  struct SSAImmInstruction : public SSAInstruction
  {
    SSARegister* dist;
    int64_t imm;
  };
  struct SSARetInstruction : public SSAInstruction
  {
    SSARegister* value;
  };
  struct SSALoadFnInstruction : public SSAInstruction
  {
    SSARegister* dist;
    pcir::FunctionSection* fn;
  };
  struct SSALoadArgInstruction : public SSAInstruction
  {
    SSARegister* dist;
    uint32_t indexOfArg;
  };
  struct SSALoadSymbolInstruction : public SSAInstruction
  {
    SSARegister* dist;
    pcir::SymbolSection* symbol;
  };
  struct SSACallInstruction : public SSAInstruction
  {
    SSARegister* dist;
    SSARegister* call;
    std::vector<SSARegister*> args;
  };

  struct SSAMovInstruction : public SSAInstruction
  {
    SSARegister* dist;
    SSARegister* src;
  };

  struct SSAFunction
  {
    pcir::TypeSection* type;
    // 関数の命令列
    std::vector<SSAInstruction*> insts;
    // 関数が使用するレジスタ一覧
    std::unordered_set<SSARegister*> regs;
    // この関数が副作用を持つかどうかのフラグ。
    // IO処理をしていたり、副作用を持つ関数を呼び出している場合に立つ。
    bool hasSideEffect;
  };
  struct SSABundle
  {
    std::unordered_map<std::string, std::unordered_set<pcir::SymbolSection*>> modules;
    std::unordered_set<pcir::SymbolSection*> symbols;
    std::unordered_map<pcir::FunctionSection*, SSAFunction> fns;
  };
}

#endif // PICKC_SSA_SSA_STRUCT_H_