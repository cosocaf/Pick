/**
 * @file module_section.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief モジュールセクション
 * @version 0.1
 * @date 2021-08-02
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_PCIR_MODULE_SECTION_H_
#define PICKC_PCIR_MODULE_SECTION_H_

#include <string>
#include <memory>
#include <map>

#include "text_section.h"
#include "type_section.h"
#include "symbol_section.h"
#include "fn_section.h"
#include "accessibility.h"

#include "utils/binary_vec.h"

namespace pickc::pcir {
  class _ModuleSection;
  class _Module;
  /**
   * @brief _ModuleSectionのshared_ptrラッパー。
   * 
   */
  using ModuleSection = std::shared_ptr<_ModuleSection>;
  /**
   * @brief _ModuleSectionのweak_ptrラッパー。
   * 
   */
  using WeakModuleSection = std::weak_ptr<_ModuleSection>;
  /**
   * @brief _Moduleのshared_ptrラッパー。
   * 
   */
  using Module = std::shared_ptr<_Module>;
  /**
   * @brief _Moduleのweak_ptrラッパー。
   * 
   */
  using WeakModule = std::weak_ptr<_Module>;

  /**
   * @brief モジュール。
   * 基本的にこのクラスを直接扱わず、ModuleやWeakModuleを使用する。
   */
  class _Module {
    WeakModuleSection moduleSection;
    Text moduleName;
    Text parentName;
    Accessibility accessibility;
    std::map<Text, Type> types;
    std::map<Text, Symbol> symbols;
    std::vector<Fn> fns;
  public:
    /**
     * @brief _Moduleを構築する。
     * 
     * @param moduleSection このモジュールが属するモジュールセクション
     * @param moduleName モジュール名
     * @param parentName 親モジュール名
     */
    _Module(const WeakModuleSection& moduleSection, const Text& moduleName, const Text& parentName);
    /**
     * @brief 型を追加する。
     * 
     * @param name 型の名前
     * @param type 型
     */
    void addType(const Text& name, const Type& type);
    /**
     * @brief シンボルを追加する。
     * 
     * @param name シンボルの名前
     * @param symbol シンボル
     */
    void addSymbol(const Text& name, const Symbol& symbol);
    /**
     * @brief 関数を追加する。
     * 
     * @param fn 追加する関数
     */
    void addFn(const Fn& fn);
    /**
     * @brief 特定の名前のシンボルを返す。
     * もし、シンボルが見つからなかった場合、例外が送出される。
     * @param name シンボルの名前
     * @return Symbol 見つかったシンボル
     */
    Symbol findSymbol(const Text& name);
    
    /**
     * @brief モジュール名を取得する。
     * 
     * @return Text モジュール名
     */
    Text getModuleName() const;
    /**
     * @brief 親モジュール名を取得する。
     * 
     * @return Text 親モジュール名
     */
    Text getParentName() const;
    /**
     * @brief モジュールのアクセシビリティを取得する。
     * 
     * @return Accessibility アクセシビリティ
     */
    Accessibility getAccessibility() const;
    /**
     * @brief このモジュールに含まれる型の数を返す。
     * 
     * @return uint32_t このモジュールに含まれる型の数
     */
    uint32_t getTypesSize() const;
    /**
     * @brief このモジュールに含まれるシンボルの数を返す。
     * 
     * @return uint32_t このモジュールに含まれるシンボルの数
     */
    uint32_t getSymbolsSize() const;
    /**
     * @brief このモジュールに含まれる型の開始イテレータを返す。
     * 
     * @return std::map<Text, Type>::iterator 開始イテレータ
     */
    std::map<Text, Type>::iterator beginTypes();
    /**
     * @brief このモジュールに含まれる型の終了イテレータを返す。
     * 
     * @return std::map<Text, Type>::iterator 終了イテレータ
     */
    std::map<Text, Type>::iterator endTypes();
    /**
     * @brief このモジュールに含まれる型の開始イテレータを返す。
     * 
     * @return std::map<Text, Type>::const_iterator 開始イテレータ
     */
    std::map<Text, Type>::const_iterator beginTypes() const;
    /**
     * @brief このモジュールに含まれる型の終了イテレータを返す。
     * 
     * @return std::map<Text, Type>::const_iterator 終了イテレータ
     */
    std::map<Text, Type>::const_iterator endTypes() const;
    /**
     * @brief このモジュールに含まれるシンボルの開始イテレータを返す。
     * 
     * @return std::map<Text, Symbol>::iterator 開始イテレータ
     */
    std::map<Text, Symbol>::iterator beginSymbols();
    /**
     * @brief このモジュールに含まれるシンボルの終了イテレータを返す。
     * 
     * @return std::map<Text, Symbol>::iterator 終了イテレータ
     */
    std::map<Text, Symbol>::iterator endSymbols();
    /**
     * @brief このモジュールに含まれるシンボルの開始イテレータを返す。
     * 
     * @return std::map<Text, Symbol>::const_iterator 開始イテレータ
     */
    std::map<Text, Symbol>::const_iterator beginSymbols() const;
    /**
     * @brief このモジュールに含まれるシンボルの終了イテレータを返す。
     * 
     * @return std::map<Text, Symbol>::const_iterator 終了イテレータ
     */
    std::map<Text, Symbol>::const_iterator endSymbols() const;
  };
  /**
   * @brief モジュールセクション。
   * 基本的にこのクラスを直接扱わず、ModuleSectionやWeakModuleSectionを使用する。
   */
  class _ModuleSection : public std::enable_shared_from_this<_ModuleSection> {
    /**
     * @brief モジュール。
     * キーは完全修飾名。
     * モジュールの完全修飾名自体はPCIRで保持しないため、Textではなくstringを使用する。
     */
    std::map<std::string, Module> modules;
    _ModuleSection() = default;
  public:
    /**
     * @brief モジュールセクションを作成する。
     * 
     * @return ModuleSection 作成したセクション
     */
    static ModuleSection create();
    /**
     * @brief モジュールを作成する。
     * 
     * @param moduleName モジュール名
     * @param parentName 親モジュール名
     * @return Module 作成したモジュール
     */
    Module createModule(const Text& moduleName, const Text& parentName);
    /**
     * @brief モジュールを取得する。
     * 
     * @param fqn モジュールの完全修飾名
     * @return Module モジュール
     */
    Module getModule(const std::string& fqn);
    /**
     * @brief このセクションのPCIRバイトコード表現を出力する。
     * 
     * @return BinaryVec 出力結果
     */
    BinaryVec toBinaryVec() const;
  };
}

#endif // PICKC_PCIR_MODULE_SECTION_H_