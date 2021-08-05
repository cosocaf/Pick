/**
 * @file type_section.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-07-31
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "type_section.h"

#include <algorithm>

namespace pickc::pcir {
  bool operator<(const ArrayType& a, const ArrayType& b) {
    return true;
  }
  bool operator>(const ArrayType& a, const ArrayType& b) {
    return b < a;
  }
  bool operator==(const ArrayType& a, const ArrayType& b) {
    return !(a > b) && !(b < a);
  }
  bool operator!=(const ArrayType& a, const ArrayType& b) {
    return !(a == b);
  }

  bool operator<(const TupleType& a, const TupleType& b) {
    return true;
  }
  bool operator>(const TupleType& a, const TupleType& b) {
    return b < a;
  }
  bool operator==(const TupleType& a, const TupleType& b) {
    return !(a > b) && !(b < a);
  }
  bool operator!=(const TupleType& a, const TupleType& b) {
    return !(a == b);
  }

  FnType::FnType(const WeakType& returnType, const std::vector<WeakType>& arguments) :
    returnType(returnType), arguments(arguments) {}
  bool operator<(const FnType& a, const FnType& b) {
    auto aRetType = a.returnType.lock();
    auto bRetType = b.returnType.lock();
    if(aRetType != bRetType) {
      return aRetType < bRetType;
    }
    for(size_t i = 0, l = std::min(a.arguments.size(), b.arguments.size()); i < l; ++i) {
      auto aArgType = a.arguments[i].lock();
      auto bArgType = b.arguments[i].lock();
      if(aArgType != bArgType) {
        return aArgType < bArgType;
      }
    }
    return a.arguments.size() < b.arguments.size();
  }
  bool operator>(const FnType& a, const FnType& b) {
    return b < a;
  }
  bool operator==(const FnType& a, const FnType& b) {
    return !(a < b) && !(b > a);
  }
  bool operator!=(const FnType& a, const FnType& b) {
    return !(a == b);
  }

  bool operator<(const StructureType& a, const StructureType& b) {
    return true;
  }
  bool operator>(const StructureType& a, const StructureType& b) {
    return b < a;
  }
  bool operator==(const StructureType& a, const StructureType& b) {
    return !(a < b) && !(b > a);
  }
  bool operator!=(const StructureType& a, const StructureType& b) {
    return !(a == b);
  }

  bool operator<(const ClassType& a, const ClassType& b) {
    return true;
  }
  bool operator>(const ClassType& a, const ClassType& b) {
    return b < a;
  }
  bool operator==(const ClassType& a, const ClassType& b) {
    return !(a < b) && !(b > a);
  }
  bool operator!=(const ClassType& a, const ClassType& b) {
    return !(a == b);
  }

  _Type::_Type(const WeakTypeSection& typeSection, const TypeVariant& typeVariant) :
    typeSection(typeSection), typeVariant(typeVariant) {}
  bool operator<(const _Type& a, const _Type& b) {
    return a.typeVariant < b.typeVariant;
  }
  bool operator>(const _Type& a, const _Type& b) {
    return a.typeVariant > b.typeVariant;
  }
  bool operator==(const _Type& a, const _Type& b) {
    return a.typeVariant == b.typeVariant;
  }
  bool operator!=(const _Type& a, const _Type& b) {
    return a.typeVariant != b.typeVariant;
  }

  TypeVariant _Type::getTypeVariant() const {
    return typeVariant;
  }
  uint32_t _Type::getIndex() const {
    if(std::holds_alternative<LangDefinedTypes>(typeVariant)) {
      return static_cast<uint32_t>(std::get<LangDefinedTypes>(typeVariant));
    }
    return typeSection.lock()->getIndex(std::const_pointer_cast<_Type>(shared_from_this()));
  }

  TypeSection _TypeSection::create() {
    return TypeSection(new _TypeSection());
  }
  Type _TypeSection::createType(const TypeVariant& typeVariant) {
    auto type = std::make_shared<_Type>(weak_from_this(), typeVariant);
    // 言語定義型はそれ専用のインデックスがあるため、typesに挿入しない。
    if(std::holds_alternative<LangDefinedTypes>(typeVariant)) {
      langDefTypes.insert(type);
    }
    else {
      types.insert(type);
    }
    return type;
  }
  uint32_t _TypeSection::getIndex(const Type& type) const {
    return static_cast<uint32_t>(std::distance(
      types.begin(),
      types.find(type)
    ));
  }
  BinaryVec _TypeSection::toBinaryVec() const {
    BinaryVec bin;
    bin << static_cast<uint32_t>(0)
        << static_cast<uint32_t>(types.size());
    for(const auto& type : types) {
      auto typeVariant = type->getTypeVariant();
      if(std::holds_alternative<ArrayType>(typeVariant)) {
        // TODO
      }
      else if(std::holds_alternative<TupleType>(typeVariant)) {
        // TODO
      }
      else if(std::holds_alternative<FnType>(typeVariant)) {
        auto fnType = std::get<FnType>(typeVariant);
        bin << TypeKind::Function
            << fnType.returnType.lock()->getIndex()
            << static_cast<uint32_t>(fnType.arguments.size());
        for(const auto& arg : fnType.arguments) {
          bin << arg.lock()->getIndex();
        }
      }
    }
    write(bin, 0, static_cast<uint32_t>(bin.size()));
    return bin;
  }
}