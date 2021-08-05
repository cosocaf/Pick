/**
 * @file module_section.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
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
#include "accessibility.h"

#include "utils/binary_vec.h"

namespace pickc::pcir {
  class _ModuleSection;
  class _Module;
  using ModuleSection = std::shared_ptr<_ModuleSection>;
  using WeakModuleSection = std::weak_ptr<_ModuleSection>;
  using Module = std::shared_ptr<_Module>;
  using WeakModule = std::weak_ptr<_Module>;

  class _Module {
    WeakModuleSection moduleSection;
    Text moduleName;
    Text parentName;
    Accessibility accessibility;
    std::map<Text, Type> types;
    std::map<Text, Symbol> symbols;
  public:
    _Module(const WeakModuleSection& moduleSection, const Text& moduleName, const Text& parentName);
    void addType(const Text& name, const Type& type);
    void addSymbol(const Text& name, const Symbol& symbol);
    Symbol findSymbol(const Text& name);
    
    Text getModuleName() const;
    Text getParentName() const;
    Accessibility getAccessibility() const;
    uint32_t getTypesSize() const;
    uint32_t getSymbolsSize() const;
    std::map<Text, Type>::iterator beginTypes();
    std::map<Text, Type>::iterator endTypes();
    std::map<Text, Type>::const_iterator beginTypes() const;
    std::map<Text, Type>::const_iterator endTypes() const;
    std::map<Text, Symbol>::iterator beginSymbols();
    std::map<Text, Symbol>::iterator endSymbols();
    std::map<Text, Symbol>::const_iterator beginSymbols() const;
    std::map<Text, Symbol>::const_iterator endSymbols() const;
  };
  class _ModuleSection : public std::enable_shared_from_this<_ModuleSection> {
    /**
     * @brief モジュール。
     * キーは完全修飾名。
     * モジュールの完全修飾名自体はPCIRで保持しないため、Textではなくstringを使用する。
     */
    std::map<std::string, Module> modules;
    _ModuleSection() = default;
  public:
    static ModuleSection create();
    Module createModule(const Text& moduleName, const Text& parentName);
    Module getModule(const std::string& fqn);
    BinaryVec toBinaryVec() const;
  };
}

#endif // PICKC_PCIR_MODULE_SECTION_H_