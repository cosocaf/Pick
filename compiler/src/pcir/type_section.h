/**
 * @file type_section.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief タイプセクション
 * @version 0.1
 * @date 2021-07-31
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_PCIR_TYPE_SECTION_H_
#define PICKC_PCIR_TYPE_SECTION_H_

#include <memory>
#include <set>
#include <vector>
#include <variant>

#include "utils/binary_vec.h"

namespace pickc::pcir {
  class _TypeSection;
  class _Type;
  /**
   * @brief _TypeSectionのshared_ptrラッパー。
   * 
   */
  using TypeSection = std::shared_ptr<_TypeSection>;
  /**
   * @brief _TypeSectionのweak_ptrラッパー。
   * 
   */
  using WeakTypeSection = std::weak_ptr<_TypeSection>;
  /**
   * @brief _Typeのshared_ptrラッパー。
   * 
   */
  using Type = std::shared_ptr<_Type>;
  /**
   * @brief _Typeのweak_ptrラッパー。
   * 
   */
  using WeakType = std::weak_ptr<_Type>;

  /**
   * @brief 型の種別
   * 
   */
  enum struct TypeKind : uint16_t {
    Array = 0x01,
    Tuple = 0x02,
    Pointer = 0x03,
    Function = 0x04,
    Structure = 0x05,
    Class = 0x06,
  };

  /**
   * @brief 言語定義型の列挙。
   * 
   */
  enum struct LangDefinedTypes : uint32_t {
    I8    = 0xFFFFFF00,
    I16   = 0xFFFFFF01,
    I32   = 0xFFFFFF02,
    I64   = 0xFFFFFF03,
    ISize = 0xFFFFFF0F,
    U8    = 0xFFFFFF10,
    U16   = 0xFFFFFF11,
    U32   = 0xFFFFFF12,
    U64   = 0xFFFFFF13,
    USize = 0xFFFFFF1F,
    F32   = 0xFFFFFF20,
    F64   = 0xFFFFFF21,
    Void  = 0xFFFFFF30,
    Bool  = 0xFFFFFF40,
  };

  struct ArrayType {
    friend class _TypeSection;
  };
  bool operator<(const ArrayType& a, const ArrayType& b);
  bool operator>(const ArrayType& a, const ArrayType& b);
  bool operator==(const ArrayType& a, const ArrayType& b);
  bool operator!=(const ArrayType& a, const ArrayType& b);

  struct TupleType {
    friend class _TypeSection;
  };
  bool operator<(const TupleType& a, const TupleType& b);
  bool operator>(const TupleType& a, const TupleType& b);
  bool operator==(const TupleType& a, const TupleType& b);
  bool operator!=(const TupleType& a, const TupleType& b);
  
  /**
   * @brief 関数型。
   * 
   */
  struct FnType {
    /**
     * @brief 戻り値の型。
     * 
     */
    WeakType returnType;
    /**
     * @brief 引数の型。
     * 
     */
    std::vector<WeakType> arguments;
    /**
     * @brief FnTypeを構築する。
     * 
     * @param returnType 戻り値の型
     * @param arguments 引数の型
     */
    FnType(const WeakType& returnType, const std::vector<WeakType>& arguments);
  };
  bool operator<(const FnType& a, const FnType& b);
  bool operator>(const FnType& a, const FnType& b);
  bool operator==(const FnType& a, const FnType& b);
  bool operator!=(const FnType& a, const FnType& b);

  struct StructureType {
    friend class _TypeSection;
  };
  bool operator<(const StructureType& a, const StructureType& b);
  bool operator>(const StructureType& a, const StructureType& b);
  bool operator==(const StructureType& a, const StructureType& b);
  bool operator!=(const StructureType& a, const StructureType& b);

  struct ClassType {
    friend class _TypeSection;
  };
  bool operator<(const ClassType& a, const ClassType& b);
  bool operator>(const ClassType& a, const ClassType& b);
  bool operator==(const ClassType& a, const ClassType& b);
  bool operator!=(const ClassType& a, const ClassType& b);

  using TypeVariant = std::variant<LangDefinedTypes, ArrayType, TupleType, FnType, StructureType, ClassType>;
  
  /**
   * @brief 型。
   * 基本的にこのクラスを直接扱わず、TypeやWeakTypeを使用する。
   */
  class _Type : public std::enable_shared_from_this<_Type> {
    WeakTypeSection typeSection;
    TypeVariant typeVariant;
    friend bool operator<(const _Type&, const _Type&);
    friend bool operator>(const _Type&, const _Type&);
    friend bool operator==(const _Type&, const _Type&);
    friend bool operator!=(const _Type&, const _Type&);
  public:
    /**
     * @brief _Typeを構築する。
     * 
     * @param typeSection この型が属するセクション
     * @param typeVariant この型のバリアント
     */
    _Type(const WeakTypeSection& typeSection, const TypeVariant& typeVariant);
    /**
     * @brief この型のバリアントを取得する。
     * 
     * @return TypeVariant この型のバリアント
     */
    TypeVariant getTypeVariant() const;
    /**
     * @brief この型のインデックスを取得する。
     * 
     * @return uint32_t この型のインデックス。
     */
    uint32_t getIndex() const;
  };
  bool operator<(const _Type& a, const _Type& b);
  bool operator>(const _Type& a, const _Type& b);
  bool operator==(const _Type& a, const _Type& b);
  bool operator!=(const _Type& a, const _Type& b);

  /**
   * @brief タイプセクション。
   * 基本的にこのクラスを直接扱わず、TypeSectionやWeakTypeSectionを使用する。
   */
  class _TypeSection : public std::enable_shared_from_this<_TypeSection> {
    std::set<Type> types;
    std::set<Type> langDefTypes;
  private:
    _TypeSection() = default;
  public:
    /**
     * @brief タイプセクションを作成する。
     * 
     * @return TypeSection 作成したタイプセクション
     */
    static TypeSection create();
  public:
    /**
     * @brief 型を作成する。
     * 
     * @param typeVariant 作成する型のバリアント
     * @return Type 作成した型
     */
    Type createType(const TypeVariant& typeVariant);
    /**
     * @brief 指定の型のインデックスを取得する。
     * 
     * @param type インデックスを取得する型
     * @return uint32_t 型のインデックス
     */
    uint32_t getIndex(const Type& type) const;
    /**
     * @brief このセクションのPCIRバイトコード表現を出力する。
     * 
     * @return BinaryVec 出力結果
     */
    BinaryVec toBinaryVec() const;
  };
}

#endif // PICKC_PCIR_TYPE_SECTION_H_