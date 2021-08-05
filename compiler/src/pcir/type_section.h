/**
 * @file type_section.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
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
#include <optional>
#include <vector>
#include <variant>

#include "utils/binary_vec.h"
#include "utils/lazy.h"

namespace pickc::pcir {
  class _TypeSection;
  class _Type;
  using TypeSection = std::shared_ptr<_TypeSection>;
  using WeakTypeSection = std::weak_ptr<_TypeSection>;
  using Type = std::shared_ptr<_Type>;
  using WeakType = std::weak_ptr<_Type>;

  enum struct TypeKind : uint16_t {
    Array = 0x01,
    Tuple = 0x02,
    Pointer = 0x03,
    Function = 0x04,
    Structure = 0x05,
    Class = 0x06,
  };

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
  
  struct FnType {
    WeakType returnType;
    std::vector<WeakType> arguments;
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
  
  class _Type : public std::enable_shared_from_this<_Type> {
    WeakTypeSection typeSection;
    TypeVariant typeVariant;
    friend bool operator<(const _Type&, const _Type&);
    friend bool operator>(const _Type&, const _Type&);
    friend bool operator==(const _Type&, const _Type&);
    friend bool operator!=(const _Type&, const _Type&);
  public:
    _Type(const WeakTypeSection& typeSection, const TypeVariant& typeVariant);
    TypeVariant getTypeVariant() const;
    uint32_t getIndex() const;
  };
  bool operator<(const _Type& a, const _Type& b);
  bool operator>(const _Type& a, const _Type& b);
  bool operator==(const _Type& a, const _Type& b);
  bool operator!=(const _Type& a, const _Type& b);

  class _TypeSection : public std::enable_shared_from_this<_TypeSection> {
    std::set<Type> types;
    std::set<Type> langDefTypes;
  private:
    _TypeSection() = default;
  public:
    static TypeSection create();
  public:
    Type createType(const TypeVariant& typeVariant);
    uint32_t getIndex(const Type& type) const;
    BinaryVec toBinaryVec() const;
  };
}

#endif // PICKC_PCIR_TYPE_SECTION_H_