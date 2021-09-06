/**
 * @file symbol_section.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief symbol_section.hの実装
 * @version 0.1
 * @date 2021-08-02
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "symbol_section.h"

namespace pickc::pcir {
  _Symbol::_Symbol(const WeakSymbolSection& symbolSection, uint32_t index, const Text& name, const Type& type, const std::optional<Fn>& initFn) :
    symbolSection(symbolSection),
    index(index),
    name(name),
    type(type),
    mutability(Mutability::Immutable),
    initFn(initFn) {}
  Text _Symbol::getName() const {
    return name;
  }
  Type _Symbol::getType() const {
    return type;
  }
  Mutability _Symbol::getMutability() const {
    return mutability;
  }
  uint32_t _Symbol::getIndex() const {
    return index;
  }
  std::optional<Fn> _Symbol::getInitFn() const {
    return initFn;
  }
  void _Symbol::setInitFn(const Fn& fn) {
    initFn = fn;
  }

  SymbolSection _SymbolSection::create() {
    return SymbolSection(new _SymbolSection());
  }
  Symbol _SymbolSection::createSymbol(const Text& name, const Type& type, const std::optional<Fn>& initFn) {
    auto symbol = std::make_shared<_Symbol>(
      weak_from_this(),
      static_cast<uint32_t>(symbols.size()),
      name,
      type,
      initFn
    );
    symbols.push_back(symbol);
    return symbol;
  }
  BinaryVec _SymbolSection::toBinaryVec() const {
    BinaryVec bin;
    bin << static_cast<uint32_t>(0)
        << static_cast<uint32_t>(symbols.size())
        << static_cast<uint32_t>(0);
    for(const auto& symbol : symbols) {
      bin << symbol->getName().getIndex()
          << SymbolKind::Internal
          << symbol->getType()->getIndex()
          << symbol->getMutability()
          << symbol->getInitFn().value()->getIndex();
    }
    // TODO: external symbols.
    write(bin, 0, static_cast<uint32_t>(bin.size()));
    return bin;
  }
}