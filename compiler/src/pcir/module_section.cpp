/**
 * @file module_section.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-08-02
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "module_section.h"

#include <algorithm>

namespace pickc::pcir {
  _Module::_Module(const WeakModuleSection& moduleSection, const Text& moduleName, const Text& parentName) :
    moduleSection(moduleSection),
    moduleName(moduleName),
    parentName(parentName),
    accessibility(Accessibility::Public) {}
  void _Module::addType(const Text& name, const Type& type) {
    types.try_emplace(name, type);
  }
  void _Module::addSymbol(const Text& name, const Symbol& symbol) {
    symbols.try_emplace(name, symbol);
  }
  Symbol _Module::findSymbol(const Text& name) {
    return symbols.at(name);
  }

  Text _Module::getModuleName() const {
    return moduleName;
  }
  Text _Module::getParentName() const {
    return parentName;
  }
  Accessibility _Module::getAccessibility() const {
    return accessibility;
  }
  uint32_t _Module::getTypesSize() const {
    return static_cast<uint32_t>(types.size());
  }
  uint32_t _Module::getSymbolsSize() const {
    return static_cast<uint32_t>(symbols.size());
  }
  std::map<Text, Type>::iterator _Module::beginTypes() {
    return types.begin();
  }
  std::map<Text, Type>::iterator _Module::endTypes() {
    return types.end();
  }
  std::map<Text, Type>::const_iterator _Module::beginTypes() const {
    return types.begin();
  }
  std::map<Text, Type>::const_iterator _Module::endTypes() const {
    return types.end();
  }
  std::map<Text, Symbol>::iterator _Module::beginSymbols() {
    return symbols.begin();
  }
  std::map<Text, Symbol>::iterator _Module::endSymbols() {
    return symbols.end();
  }
  std::map<Text, Symbol>::const_iterator _Module::beginSymbols() const {
    return symbols.begin();
  }
  std::map<Text, Symbol>::const_iterator _Module::endSymbols() const {
    return symbols.end();
  }

  ModuleSection _ModuleSection::create() {
    return ModuleSection(new _ModuleSection());
  }
  Module _ModuleSection::createModule(const Text& moduleName, const Text& parentName) {
    std::string prefix;
    if(!parentName.getValue().empty()) {
      prefix += parentName.getValue() + "::";
    }
    auto res = modules.try_emplace(
      prefix + moduleName.getValue(),
      std::make_shared<_Module>(
        weak_from_this(),
        moduleName,
        parentName
      )
    );
    return res.first->second;
  }
  Module _ModuleSection::getModule(const std::string& fqn) {
    return modules.at(fqn);
  }
  BinaryVec _ModuleSection::toBinaryVec() const {
    BinaryVec bin;
    bin << static_cast<uint32_t>(0)
        << static_cast<uint32_t>(modules.size());
    for(const auto& [moduleName, module] : modules) {
      bin << module->getModuleName().getIndex()
          << module->getParentName().getIndex()
          << module->getAccessibility()
          << module->getTypesSize()
          << module->getSymbolsSize();
      std::for_each(
        module->beginTypes(),
        module->endTypes(),
        [&bin](const auto& type) {
          bin << type.second->getIndex()
              << Accessibility::Public;
        }
      );
      std::for_each(
        module->beginSymbols(),
        module->endSymbols(),
        [&bin](const auto& symbol) {
          bin << symbol.second->getIndex()
              << Accessibility::Public;
        }
      );
    }
    write(bin, 0, static_cast<uint32_t>(bin.size()));
    return bin;
  }
}