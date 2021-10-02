/**
 * @file bundle.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-09-06
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_BUNDLER_BUNDLE_H_
#define PICKC_BUNDLER_BUNDLE_H_

#include <map>
#include <array>
#include <variant>
#include <memory>
#include <string>

#include "pcir_decoder.h"
#include "utils/binary_vec.h"
#include "utils/lazy.h"

namespace pickc::bundler {
  class _Bundle;
  using Bundle = std::shared_ptr<_Bundle>;
  using WeakBundle = std::weak_ptr<_Bundle>;

  struct TypeInfo;
  enum struct TypeLangDef {
    I8, I16, I32, I64, ISize,
    U8, U16, U32, U64, USize,
    F32, F64,
    Void,
    Bool
  };
  struct TypeArray {};
  struct TypePointer {};
  struct TypeFunction {
    Lazy<TypeInfo> ret;
    std::vector<TypeInfo> args;
  };
  struct TypeStructure {};
  struct TypeClass {};
  struct TypeInfo {
    WeakBundle bundle;
    std::variant<TypeLangDef, TypeArray, TypePointer, TypeFunction, TypeStructure, TypeClass> variant;
  };

  struct InternalFunction {
    std::vector<TypeInfo> variables;
    std::vector<PCIRBlock> blocks;
  };
  struct ExternalFunction {
    std::string extName;
  };
  struct FunctionInfo {
    WeakBundle bundle;
    // type::variantは必ずTypeFunction
    TypeInfo type;
    std::variant<InternalFunction, ExternalFunction> variant;
  };

  enum struct Mutability : uint16_t {
    Mutable = 0x0001,
    Immutable = 0x0002,
  };
  struct SymbolInfo {
    WeakBundle bundle;
    std::string name;
    TypeInfo type;
    Mutability mutability;
    FunctionInfo init;
  };

  class _Bundle : public std::enable_shared_from_this<_Bundle> {
    std::map<std::array<uint8_t, 16>, PCIR> pcirs;
    _Bundle();
  public:
    static Bundle create();
    void insert(PCIR&& pcir);
    std::string findText(uint8_t (&fileID)[16], uint32_t index);
    std::string findText(const std::array<uint8_t, 16>& fileID, uint32_t index);
    TypeInfo findType(uint8_t (&fileID)[16], uint32_t index);
    TypeInfo findType(const std::array<uint8_t, 16>& fileID, uint32_t index);
    SymbolInfo findSymbol(uint8_t (&fileID)[16], uint32_t index);
    SymbolInfo findSymbol(const std::array<uint8_t, 16>& fileID, uint32_t index);
    FunctionInfo findFunction(uint8_t (&fileID)[16], uint32_t index);
    FunctionInfo findFunction(const std::array<uint8_t, 16>& fileID, uint32_t index);
    std::map<std::array<uint8_t, 16>, std::vector<FunctionInfo>> getFunctions();
  };
}

#endif // PICKC_BUNDLER_BUNDLE_H_