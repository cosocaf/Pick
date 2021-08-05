/**
 * @file generator.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-07-24
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_PCIR_GENERATOR_H_
#define PICKC_PCIR_GENERATOR_H_

#include <vector>
#include <optional>

#include "text_section.h"
#include "module_section.h"
#include "type_section.h"
#include "symbol_section.h"
#include "fn_section.h"

#include "parser/module_tree.h"

namespace pickc::pcir {
  class PCIRGenerator {
    parser::ModuleTree rootModule;
    TextSection textSection;
    ModuleSection moduleSection;
    TypeSection typeSection;
    SymbolSection symbolSection;
    FnSection fnSection;
    friend class _TextSection;
    friend class _ModuleSection;
    friend class _TypeSection;
    friend class _SymbolSection;
    friend class _FnSection;
    friend class FnGenerator;
  public:
    PCIRGenerator(const parser::ModuleTree& rootModule);
    bool generate();
    bool writeToFile(const std::string& filename);
  private:
    bool declare(const std::string& parentName, const parser::ModuleTree& module);
    bool generate(const parser::ModuleTree& module);
  };
}

#endif // PICKC_PCIR_GENERATOR_H_