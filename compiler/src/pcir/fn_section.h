/**
 * @file fn.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 関数セクション
 * @version 0.1
 * @date 2021-07-28
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_PCIR_FN_SECTION_H_
#define PICKC_PCIR_FN_SECTION_H_

#include <vector>
#include <memory>
#include <variant>

#include "type_section.h"
#include "variable.h"
#include "operator.h"

#include "utils/binary_vec.h"

namespace pickc::pcir {
  class _FnSection;
  class _Fn;
  class _CodeBlock;
  /**
   * @brief _FnSectionのshared_ptrラッパー
   * 
   */
  using FnSection = std::shared_ptr<_FnSection>;
  /**
   * @brief _FnSectionのweak_ptrラッパー
   * 
   */
  using WeakFnSection = std::weak_ptr<_FnSection>;
  /**
   * @brief _Fnのshared_ptrラッパー
   * 
   */
  using Fn = std::shared_ptr<_Fn>;
  /**
   * @brief _Fnのweak_ptrラッパー
   * 
   */
  using WeakFn = std::weak_ptr<_Fn>;
  /**
   * @brief _CodeBlockのshared_ptrラッパー
   * 
   */
  using CodeBlock = std::shared_ptr<_CodeBlock>;
  /**
   * @brief _CodeBlockのweak_ptrラッパー
   * 
   */
  using WeakCodeBlock = std::weak_ptr<_CodeBlock>;

  enum struct FnKind : uint16_t {
    Internal = 0x0001,
    External = 0x0002,
  };

  /**
   * @brief ブロックの種別の列挙。
   * 
   */
  enum struct BlockKind : uint16_t {
    /**
     * @brief 非終端ブロックであることを表す。
     * 
     */
    NonTerminal = 0x01,
    /**
     * @brief 終端ブロックであることを表す。
     * 
     */
    Terminal = 0x02,
    /**
     * @brief 条件分岐ブロックであることを表す。
     * 
     */
    ConditionalBranch = 0x03,
  };

  /**
   * @brief 非終端ブロック。
   * このブロックは通常のブロックであり、
   * このブロックの処理が終わった後は次のブロックへジャンプする。
   */
  struct NonTerminalBlock {
    /**
     * @brief このブロックの処理が終わった後にジャンプするブロック。
     * 
     */
    WeakCodeBlock nextBlock;
    /**
     * @brief NonTerminalBlockを構築する。
     * 
     * @param nextBlock このブロックの処理が終わった後にジャンプするブロック
     */
    NonTerminalBlock(const WeakCodeBlock& nextBlock);
  };
  /**
   * @brief 終端ブロック。
   * ソースコード上でreturnが書かれていたり、これ以上関数本文が続かないブロックは
   * このブロックを使用する。
   */
  struct TerminalBlock {
    /**
     * @brief 関数の戻り値。
     * 
     */
    Variable returnValue;
    /**
     * @brief TerminalBlockを構築する。
     * 
     * @param returnValue 関数の戻り値
     */
    TerminalBlock(const Variable& returnValue);
  };
  /**
   * @brief 条件分岐ブロック。
   * if, for, whileなどの構文は全てこのブロックにされる。
   */
  struct ConditionalBranchBlock {
    /**
     * @brief 条件変数。
     * 
     */
    Variable conditional;
    /**
     * @brief 条件がtrueであった場合にジャンプするブロック。
     * 
     */
    WeakCodeBlock thenBlock;
    /**
     * @brief 条件がfalseであった場合にジャンプするブロック。
     * 
     */
    WeakCodeBlock elseBlock;
  };
  /**
   * @brief コードブロック。
   * 基本的にこのクラスを直接扱わず、CodeBlockやWeakCodeBlockを使用する。
   */
  class _CodeBlock : public std::enable_shared_from_this<_CodeBlock> {
    WeakFn fn;
    uint32_t index;
    std::vector<std::unique_ptr<Operator>> operators;
    std::variant<void*, NonTerminalBlock, TerminalBlock, ConditionalBranchBlock> blockVariant;
  public:
    /**
     * @brief _CodeBlockを構築する。
     * 
     * @param fn このコードブロックが属する関数。
     * @param index このコードブロックのインデックス。
     */
    _CodeBlock(const WeakFn& fn, uint32_t index);
    /**
     * @brief ブロックに命令を挿入する。
     * 
     * @tparam Operator 挿入する命令。
     * @tparam Vals Operatorのコンストラクタ引数の型。
     * @param vals Operatorのコンストラクタ引数の値。
     */
    template<typename Operator, typename... Vals>
    void emplace(Vals&&... vals) {
      operators.emplace_back(
        std::make_unique<Operator>(std::forward<Vals>(vals)...)
      );
    }
    /**
     * @brief このコードブロックを非終端ブロックとして扱う。
     * 
     * @param nonTerminalBlock 非終端ブロック。
     */
    void toNonTerminalBlock(const NonTerminalBlock& nonTerminalBlock);
    /**
     * @brief このコードブロックを終端ブロックとして扱う。
     * 
     * @param terminalBlock 終端ブロック。
     */
    void toTerminalBlock(const TerminalBlock& terminalBlock);
    /**
     * @brief このコードブロックを条件分岐ブロックとして扱う。
     * 
     * @param conditional 条件分岐ブロック。
     */
    void toConditionalBranchBlock(const ConditionalBranchBlock& conditional);
    /**
     * @brief このコードブロックのインデックスを取得する。
     * 
     * @return uint32_t コードブロックのインデックス
     */
    uint32_t getIndex() const;
    /**
     * @brief このコードブロックのPCIRバイトコード表現を出力する。
     * この関数を実行する時点でこのコードブロックの種別が確定していなければならない。
     * 
     * @return BinaryVec 出力結果
     */
    BinaryVec toBinaryVec() const;
  };

  /**
   * @brief 関数を表す。
   * 基本的にこのクラスを直接扱わず、FnやWeakFnを使用する。
   */
  class _Fn : public std::enable_shared_from_this<_Fn>{
    WeakFnSection fnSection;
    uint32_t index;
    Type type;
    std::vector<CodeBlock> codeBlocks;
    CodeBlock entryBlock;
    VariableTable variableTable;
  public:
    /**
     * @brief _Fnを構築する。
     * 
     * @param fnSection この関数が属するセクション
     * @param index この関数のインデックス
     * @param type この関数の型
     */
    _Fn(const WeakFnSection& fnSection, uint32_t index, const Type& type);
    /**
     * @brief この関数が最初に実行するコードブロックを取得する。
     * 
     * @return CodeBlock この関数のエントリーブロック
     */
    CodeBlock getEntryBlock();
    /**
     * @brief コードブロックを追加する。
     * 
     * @return CodeBlock 追加したコードブロック。
     */
    CodeBlock addBlock();
    /**
     * @brief 変数を追加する。
     * 
     * @param name 変数の名前
     * @param type 変数の型
     * @return Variable 追加した変数
     */
    Variable addVariable(const std::string& name, const Type& type);
    /**
     * @brief 変数を追加する。
     * 変数名は自動で付与される。
     * 
     * @param type 変数の型
     * @return Variable 追加した変数
     */
    Variable addVariable(const Type& type);
    
    /**
     * @brief この関数のインデックスを取得する。
     * 
     * @return uint32_t この関数のインデックス。
     */
    uint32_t getIndex() const;
    /**
     * @brief この関数の型を取得する。
     * 
     * @return Type この関数の型
     */
    Type getType() const;
    /**
     * @brief この関数の変数テーブルを取得する。
     * 
     * @return VariableTable この関数の変数テーブル
     */
    VariableTable getVariableTable() const;
    /**
     * @brief この関数のコードブロックの数を取得する。
     * 
     * @return uint32_t この関数のコードブロックの数
     */
    uint32_t getCodeBlocksSize() const;
    /**
     * @brief この関数のコードブロックの開始イテレータを返す。
     * 
     * @return std::vector<CodeBlock>::iterator コードブロックの開始イテレータ
     */
    std::vector<CodeBlock>::iterator begin();
    /**
     * @brief この関数のコードブロックの終了イテレータを返す。
     * 
     * @return std::vector<CodeBlock>::iterator コードブロックの終了イテレータ
     */
    std::vector<CodeBlock>::iterator end();
    /**
     * @brief この関数のコードブロックの開始イテレータを返す。
     * 
     * @return std::vector<CodeBlock>::const_iterator コードブロックの開始イテレータ
     */
    std::vector<CodeBlock>::const_iterator begin() const;
    /**
     * @brief この関数のコードブロックの終了イテレータを返す。
     * 
     * @return std::vector<CodeBlock>::const_iterator コードブロックの終了イテレータ
     */
    std::vector<CodeBlock>::const_iterator end() const;
  };

  /**
   * @brief 関数セクション。
   * 基本的にこのクラスを直接扱わず、FnSectionやWeakFnSectionを使用する。
   */
  class _FnSection : public std::enable_shared_from_this<_FnSection> {
    std::vector<Fn> fns;
  private:
    _FnSection() = default;
  public:
    /**
     * @brief FnSectionを作成する。
     * 
     * @return FnSection 作成したセクション
     */
    static FnSection create();
    /**
     * @brief 関数を作成する。
     * 
     * @param type 関数の型
     * @return Fn 作成した関数
     */
    Fn createFn(const Type& type);
    /**
     * @brief このセクションのPCIRバイトコード表現を出力する。
     * 
     * @return BinaryVec 出力結果
     */
    BinaryVec toBinaryVec() const;
  };
}

#endif // PICKC_PCIR_FN_SECTION_H_