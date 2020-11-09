#ifndef PICKC_PCIR_PCIR_CODE_H_
#define PICKC_PCIR_PCIR_CODE_H_

#include <cstdint>

namespace pickc::pcir
{
  /**
   * SSA実現のためdistレジスタはすべて未使用でなければならない。
  */

  /**
   * Add
   * 加算命令。dist = left + right
   * dist, left, rightの型は符号付き整数型または符号なし整数型または浮動小数型であり、型をそろえる必要がある。
   * Add dist(u32, index of regs) left(u32, index of regs) right(u32, index of regs)
  */
  constexpr uint8_t Add = 0x00;
  /**
   * Sub
   * 減算命令。dist = left - right
   * dist, left, rightの型は符号付き整数型または符号なし整数型または浮動小数型であり、型をそろえる必要がある。
   * Sub dist(u32, index of regs) left(u32, index of regs) right(u32, index of regs)
  */
  constexpr uint8_t Sub = 0x01;
  /**
   * Mul
   * 乗算命令。dist = left * right
   * dist, left, rightの型は符号付き整数型または符号なし整数型または浮動小数型であり、型をそろえる必要がある。
   * Mul dist(u32, index of regs) left(u32, index of regs) right(u32, index of regs)
  */
  constexpr uint8_t Mul = 0x02;
  /**
   * Div
   * 除算命令。dist = left / right
   * dist, left, rightの型は符号付き整数型または符号なし整数型または浮動小数型であり、型をそろえる必要がある。
   * Div dist(u32, index of regs) left(u32, index of regs) right(u32, index of regs)
  */
  constexpr uint8_t Div = 0x03;
  /**
   * Mod
   * 剰余命令。dist = left % right
   * dist, left, rightの型は符号付き整数型または符号なし整数型または浮動小数型であり、型をそろえる必要がある。
   * Mod dist(u32, index of regs) left(u32, index of regs) right(u32, index of regs)
  */
  constexpr uint8_t Mod = 0x04;
  /**
   * Inc
   * インクリメント命令。dist = reg + 1
   * dist, regの型は符号付き整数型または符号なし整数型または浮動小数型であり、型をそろえる必要がある。
   * Inc dist(u32, index of regs) reg(u32, index of regs)
  */
  constexpr uint8_t Inc = 0x05;
  /**
   * Dec
   * デクリメント命令。dist = reg - 1
   * dist, regの型は符号付き整数型または符号なし整数型または浮動小数型であり、型をそろえる必要がある。
   * Dec dist(u32, index of regs) reg(u32, index of regs)
  */
  constexpr uint8_t Dec = 0x06;
  /**
   * Pos
   * 符号付与命令。dist = +reg
   * Negの対を作るための便宜上の命令で、実態は何もしない。
   * dist, regの型は符号付き整数型または符号なし整数型または浮動小数型であり、型をそろえる必要がある。
   * Pos dist(u32, index of regs) reg(u32, index of regs)
  */
  constexpr uint8_t Pos = 0x07;
  /**
   * Neg
   * 符号反転命令。dist = -reg
   * dist, regの型は符号付き整数型または符号なし整数型または浮動小数型であり、型をそろえる必要がある。
   * Neg dist(u32, index of regs) reg(u32, index of regs)
  */
  constexpr uint8_t Neg = 0x08;
  /**
   * EQ
   * 等比較命令。dist = r1 == r2
   * EQ dist(u32, index of regs) r1(u32, index of regs) r2(u32, index of regs)
  */
  constexpr uint8_t EQ = 0x09;
  /**
   * NEQ
   * 不等比較命令。dist = r1 != r2
   * NEQ dist(u32, index of regs) r1(u32, index of regs) r2(u32, index of regs)
  */
  constexpr uint8_t NEQ = 0x0A;
  /**
   * GT
   * 大なり比較命令。dist = r1 > r2
   * GT dist(u32, index of regs) r1(u32, index of regs) r2(u32, index of regs)
  */
  constexpr uint8_t GT = 0x0B;
  /**
   * GE
   * 大なり等比較命令。dist = r1 >= r2
   * GE dist(u32, index of regs) r1(u32, index of regs) r2(u32, index of regs)
  */
  constexpr uint8_t GE = 0x0C;
  /**
   * LT
   * 小なり比較命令。dist = r1 < r2
   * LT dist(u32, index of regs) r1(u32, index of regs) r2(u32, index of regs)
  */
  constexpr uint8_t LT = 0x0D;
  /**
   * LE
   * 小なり等比較命令。dist = r1 <= r2
   * LE dist(u32, index of regs) r1(u32, index of regs) r2(u32, index of regs)
  */
  constexpr uint8_t LE = 0x0E;
  /**
   * Imm
   * 即値確保命令。dist = imm
   * immのバイト数はdistの型の大きさにする必要がある。
   * Imm dist(u32, index of regs) imm
  */
  constexpr uint8_t Imm = 0x10;
  /**
   * Call
   * 関数呼び出し命令。dist = Call([args])
   * argsの数はfnの引数の数と等しくする必要がある。
   * Call dist(u32, index of regs) fn(u32, index of regs) [args(u32, index of regs)]
  */
  constexpr uint8_t Call = 0x20;
  /**
   * LoadFn
   * 関数テーブルから関数を読み込む。fn = fnTable[index]
   * LoadFn dist(u32, index of regs), index(u32, index of fnTable)
  */
  constexpr uint8_t LoadFn = 0x30;
  /**
   * LoadArg
   * 引数を読み込む。dist = args[index]
   * LoadArg dist(u32, index of regs), index(u32, index of args)
  */
  constexpr uint8_t LoadArg = 0x31;
  /**
   * LoadSymbol
   * シンボルを読み込む。dist = symbol
   * 外部PCIR空の読み込みにも対応するため、シンボル名からの指定になる。
   * そのため、シンボル名は完全修飾名でなければならない。
   * LoadSymbol dist(u32, index of regs), name(u32, index of text section)
  */
  constexpr uint8_t LoadSymbol = 0x32;
  /**
   * Mov
   * 移動命令。レジスタの持つ値をコピーする。dist = src
   * この命令はシャローコピーを行う。
   * Mov dist(u32, index of regs) src(u32, index of regs)
  */
  constexpr uint8_t Mov = 0x40;
  /**
   * Phi
   * φ命令。二つのレジスタを同一のレジスタとしてみなす。
   * 各レジスタの定義されるフローは条件分岐などによって独立していなければならず、
   * 親子関係であってはならない。
   * Phi dist(u32, index of regs) r1(u32, index of regs) r2(u32, index of regs)
  */
  constexpr uint8_t Phi = 0x41;
}

#endif // PICKC_PCIR_PCIR_CODE_H_