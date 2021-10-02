/**
 * @file bundle.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-09-07
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "bundle.h"

#include <cassert>

namespace pickc::bundler {
  namespace {
    std::array<uint8_t, 16> convFileID(const uint8_t (&fileID)[16]) {
      std::array<uint8_t, 16> res;
      memcpy(res.data(), fileID, 16);
      return res;
    }
    TypeInfo getLangDefType(uint8_t index) {
      static TypeInfo types[0xFF];
      static bool inited = false;
      if(!inited) {
        types[0x00].variant = TypeLangDef::I8;
        types[0x01].variant = TypeLangDef::I16;
        types[0x02].variant = TypeLangDef::I32;
        types[0x03].variant = TypeLangDef::I64;
        types[0x0F].variant = TypeLangDef::ISize;

        types[0x10].variant = TypeLangDef::U8;
        types[0x11].variant = TypeLangDef::U16;
        types[0x12].variant = TypeLangDef::U32;
        types[0x13].variant = TypeLangDef::U64;
        types[0x1F].variant = TypeLangDef::USize;

        types[0x20].variant = TypeLangDef::F32;
        types[0x21].variant = TypeLangDef::F64;

        types[0x30].variant = TypeLangDef::Void;

        types[0x40].variant = TypeLangDef::Bool;

        inited = true;
      }

      return types[index];
    }
  }

  void _Bundle::insert(PCIR&& pcir) {
    pcirs.emplace(convFileID(pcir.header.fileID), std::move(pcir));
  }
  std::string _Bundle::findText(uint8_t (&fileID)[16], uint32_t index) {
    return findText(convFileID(fileID), index);
  }
  std::string _Bundle::findText(const std::array<uint8_t, 16>& fileID, uint32_t index) {
    return pcirs[fileID].textSection.texts[index];
  }
  TypeInfo _Bundle::findType(uint8_t (&fileID)[16], uint32_t index) {
    return findType(convFileID(fileID), index);
  }
  TypeInfo _Bundle::findType(const std::array<uint8_t, 16>& fileID, uint32_t index) {
    if((index & 0xFFFFFF00) == 0xFFFFFF00) {
      auto typeInfo = getLangDefType(static_cast<uint8_t>(index ^ 0xFFFFFF00));
      typeInfo.bundle = weak_from_this();
      return typeInfo;
    }

    TypeInfo typeInfo{};
    typeInfo.bundle = weak_from_this();

    // TODO: cache
    auto& type = pcirs[fileID].typeSection.types[index];
    switch(type.kind) {
      // Array
      case 0x0001: {
        // TODO
        assert(false);
        break;
      }
      // Tuple
      case 0x0002: {
        // TODO
        assert(false);
        break;
      }
      // Pointer
      case 0x0003: {
        // TODO
        assert(false);
        break;
      }
      // Function
      case 0x0004: {
        auto& pcirFn = std::get<PCIRTypeFunction>(type.variant);

        TypeFunction function;
        function.ret = findType(fileID, pcirFn.returnType);
        for(auto& arg : pcirFn.args) {
          function.args.push_back(findType(fileID, arg));
        }
        typeInfo.variant = function;
        break;
      }
      // Structure
      case 0x0005: {
        // TODO
        assert(false);
        break;
      }
      // Class
      case 0x0006: {
        // TODO
        assert(false);
        break;
      }
      // External
      case 0x0007: {
        // TODO
        assert(false);
        break;
      }
    }

    return typeInfo;
  }
  SymbolInfo _Bundle::findSymbol(uint8_t (&fileID)[16], uint32_t index) {
    return findSymbol(convFileID(fileID), index);
  }
  SymbolInfo _Bundle::findSymbol(const std::array<uint8_t, 16>& fileID, uint32_t index) {
    SymbolInfo symbolInfo{};
    symbolInfo.bundle = weak_from_this();
    
    auto& symbol = pcirs[fileID].symbolSection.symbols[index];

    symbolInfo.name = findText(fileID, symbol.name);
    switch(symbol.kind) {
      // Internal
      case 0x0001: {
        auto& internal = std::get<PCIRInternalSymbol>(symbol.variant);
        symbolInfo.type = findType(fileID, internal.type);
        symbolInfo.mutability = static_cast<Mutability>(internal.mutability);
        symbolInfo.init = findFunction(fileID, internal.initFn);
        break;
      }
      // External
      case 0x0002: {
        // TODO:
        assert(false);
        break;
      }
    }

    return symbolInfo;
  }
  FunctionInfo _Bundle::findFunction(uint8_t (&fileID)[16], uint32_t index) {
    return findFunction(convFileID(fileID), index);
  }
  FunctionInfo _Bundle::findFunction(const std::array<uint8_t, 16>& fileID, uint32_t index) {
    FunctionInfo fnInfo{};
    fnInfo.bundle = weak_from_this();

    auto& fn = pcirs[fileID].functionSection.functions[index];
    fnInfo.type = findType(fileID, fn.type);

    switch(fn.kind) {
      // Internal
      case 0x0001: {
        auto& pcirFn = std::get<PCIRInternalFunction>(fn.variant);
        InternalFunction internal{};
        for(auto& var : pcirFn.vars) {
          internal.variables.push_back(findType(fileID, var.type));
        }
        internal.blocks = pcirFn.blocks;
        fnInfo.variant = internal;
        break;
      }
      // External
      case 0x0002: {
        auto& pcirFn = std::get<PCIRExternalFunction>(fn.variant);
        ExternalFunction external{};
        external.extName = findText(fileID, pcirFn.name);
        fnInfo.variant = external;
        break;
      }
    }

    return fnInfo;
  }

  _Bundle::_Bundle() {}
  Bundle _Bundle::create() {
    return std::shared_ptr<_Bundle>(new _Bundle());
  }
  std::map<std::array<uint8_t, 16>, std::vector<FunctionInfo>> _Bundle::getFunctions() {
    std::map<std::array<uint8_t, 16>, std::vector<FunctionInfo>> res;
    for(const auto& [key, value] : pcirs) {
      res[key].resize(value.functionSection.numFunctions);
      for(uint32_t index = 0; index < value.functionSection.numFunctions; ++index) {
        res[key][index] = findFunction(key, index);
      }
    }
    return res;
  }
}