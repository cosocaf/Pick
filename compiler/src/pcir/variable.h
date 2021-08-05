/**
 * @file variable.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 変数
 * @version 0.1
 * @date 2021-07-29
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_PCIR_VARIABLE_H_
#define PICKC_PCIR_VARIABLE_H_

#include <string>
#include <map>
#include <memory>

#include "type_section.h"

namespace pickc::pcir {
  class _VariableTable;
  /**
   * @brief _VariableTableのshared_ptrラッパー。
   * 
   */
  using VariableTable = std::shared_ptr<_VariableTable>;
  using WeakVariableTable = std::weak_ptr<_VariableTable>;

  /**
   * @brief 変数。
   * @note 実装に未確定なことが多いため、今後大きく変更される可能性がある。
   */
  class Variable {
    std::string name;
    Type type;
    bool isMut;
    bool inited;
    WeakVariableTable variableTable;
  public:
    /**
     * @brief Variableを構築する。
     * 
     * @param name 変数の名前
     * @param type 変数の型
     * @param isMut 変数がミュータブルであるか
     * @param inited 変数が初期化済みであるか
     * @param variableTable 変数テーブル
     */
    Variable(const std::string& name, const Type& type, bool isMut, bool inited, const WeakVariableTable& variableTable);
    /**
     * @brief この変数のインデックスを取得する。
     * 
     * @return uint32_t 変数のインデックス
     */
    uint32_t getIndex() const;
    /**
     * @brief この変数の名前を取得する。
     * 
     * @return std::string 変数名
     */
    std::string getName() const;
    /**
     * @brief この変数の型を取得する。
     * 
     * @return Type 変数の型
     */
    Type getType() const;
  };

  /**
   * @brief 変数テーブル。
   * 基本的にこのクラスを直接扱わず、VariableTableやWeakVariableTableを使用する。
   */
  class _VariableTable : public std::enable_shared_from_this<_VariableTable> {
    std::map<std::string, Variable> variables;
    size_t anonymousVariableCount;
  private:
    _VariableTable() = default;
  public:
    /**
     * @brief 変数テーブルを作成する。
     * 
     * @return VariableTable 作成した変数テーブル。
     */
    static VariableTable create();
  public:
    /**
     * @brief 変数を作成する。
     * 
     * @param type 変数の型
     * @param name 変数の名前
     * @return Variable 作成した変数
     */
    Variable createVariable(const Type& type, const std::string& name);
    /**
     * @brief 無名変数を作成する。
     * 
     * @param type 変数の型
     * @return Variable 作成した変数
     */
    Variable createAnonymousVariable(const Type& type);
    /**
     * @brief 変数テーブルに含まれる変数の数を取得する。
     * 
     * @return uint32_t 変数の数
     */
    uint32_t size() const;
    /**
     * @brief 変数テーブルの開始イテレータを取得する。
     * 
     * @return std::map<std::string, Variable>::iterator 開始イテレータ
     */
    std::map<std::string, Variable>::iterator begin();
    /**
     * @brief 変数テーブルの終了イテレータを取得する。
     * 
     * @return std::map<std::string, Variable>::iterator 終了イテレータ
     */
    std::map<std::string, Variable>::iterator end();
    /**
     * @brief 変数テーブルの開始イテレータを取得する。
     * 
     * @return std::map<std::string, Variable>::const_iterator 開始イテレータ
     */
    std::map<std::string, Variable>::const_iterator begin() const;
    /**
     * @brief 変数テーブルの終了イテレータを取得する。
     * 
     * @return std::map<std::string, Variable>::const_iterator 終了イテレータ
     */
    std::map<std::string, Variable>::const_iterator end() const;

    uint32_t getIndex(const Variable& var) const;
  private:
    std::string nextAnonymousVariableName();
  };
}

#endif // PICKC_PCIR_VARIABLE_H_