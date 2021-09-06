#ifndef PICKC_PCIR_SYMBOL_SECTION_H_
#define PICKC_PCIR_SYMBOL_SECTION_H_

#include <memory>
#include <vector>
#include <optional>

#include "text_section.h"
#include "type_section.h"
#include "fn_section.h"
#include "mutability.h"

#include "utils/binary_vec.h"

namespace pickc::pcir {
  class _SymbolSection;
  class _Symbol;
  /**
   * @brief _SymbolSectionのshared_ptrラッパー。
   * 
   */
  using SymbolSection = std::shared_ptr<_SymbolSection>;
  /**
   * @brief _SymbolSectionのweak_ptrラッパー。
   * 
   */
  using WeakSymbolSection = std::weak_ptr<_SymbolSection>;
  /**
   * @brief _Symbolのshared_ptrラッパー。
   * 
   */
  using Symbol = std::shared_ptr<_Symbol>;
  /**
   * @brief _Symbolのweak_ptrラッパー。
   * 
   */
  using WeakSymbol = std::weak_ptr<_Symbol>;

  enum struct SymbolKind : uint16_t {
    Internal = 0x0001,
    External = 0x0002,
  };

  /**
   * @brief シンボル。
   * 基本的にこのクラスを直接扱わず、SymbolやWeakSymbolを使用する。
   */
  class _Symbol {
    WeakSymbolSection symbolSection;
    uint32_t index;
    Text name;
    Type type;
    Mutability mutability;
    std::optional<Fn> initFn;
  public:
    /**
     * @brief _Symbolを構築する。
     * 
     * @param symbolSection このシンボルが属するセクション
     * @param index シンボルのインデックス
     * @param name シンボルの名前
     * @param type シンボルの型
     * @param initFn シンボルの初期化関数
     */
    _Symbol(const WeakSymbolSection& symbolSection, uint32_t index, const Text& name, const Type& type, const std::optional<Fn>& initFn);
    /**
     * @brief シンボルの名前を取得する。
     * 
     * @return Text シンボルの名前
     */
    Text getName() const;
    /**
     * @brief シンボルの型を取得する。
     * 
     * @return Type シンボルの型
     */
    Type getType() const;
    /**
     * @brief シンボルのミュータビリティを取得する。
     * 
     * @return Mutability シンボルのミュータビリティ
     */
    Mutability getMutability() const;
    /**
     * @brief シンボルのインデックスを取得する。
     * 
     * @return uint32_t シンボルのインデックス
     */
    uint32_t getIndex() const;
    /**
     * @brief シンボルの初期化関数を取得する。
     * 
     * @return std::optional<Fn> シンボルの初期化関数
     */
    std::optional<Fn> getInitFn() const;
    /**
     * @brief シンボルの初期化関数を設定する。
     * 
     * @param fn シンボルの初期化関数
     */
    void setInitFn(const Fn& fn);
  };

  /**
   * @brief シンボルセクション。
   * 基本的にこのクラスを直接扱わず、SymbolSectionやWeakSymbolSectionを使用する。
   */
  class _SymbolSection : public std::enable_shared_from_this<_SymbolSection> {
    std::vector<Symbol> symbols;
  private:
    _SymbolSection() = default;
  public:
    /**
     * @brief シンボルセクションを作成する。
     * 
     * @return SymbolSection 作成したセクション
     */
    static SymbolSection create();
    /**
     * @brief シンボルを作成する。
     * 
     * @param name シンボルの名前
     * @param type シンボルの型
     * @param initFn シンボルの初期化関数
     * @return Symbol 作成したシンボル
     */
    Symbol createSymbol(const Text& name, const Type& type, const std::optional<Fn>& initFn);
    /**
     * @brief このセクションのPCIRバイトコード表現を出力する。
     * 
     * @return BinaryVec 出力結果
     */
    BinaryVec toBinaryVec() const;
  };
}

#endif // PICKC_PCIR_SYMBOL_SECTION_H_