/**
 * @file generator.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief PCIRのジェネレータ
 * @version 0.1
 * @date 2021-07-24
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_PCIR_GENERATOR_H_
#define PICKC_PCIR_GENERATOR_H_

#include "text_section.h"
#include "module_section.h"
#include "type_section.h"
#include "symbol_section.h"
#include "fn_section.h"

#include "parser/module_tree.h"

namespace pickc::pcir {
  /**
   * @brief PCIRジェネレータ。
   * PCIRを生成し、ファイル出力を行う。
   */
  class PCIRGenerator {
    parser::ModuleTree rootModule;
    TextSection textSection;
    ModuleSection moduleSection;
    TypeSection typeSection;
    SymbolSection symbolSection;
    FnSection fnSection;
  public:
    /**
     * @brief PCIRGeneratorを構築する。
     * 
     * @param rootModule 生成元のルートモジュール
     */
    PCIRGenerator(const parser::ModuleTree& rootModule);
    /**
     * @brief PCIRを生成する。
     * 
     * @return true 生成に成功した場合
     * @return false 生成に失敗した場合
     */
    bool generate();
    /**
     * @brief PCIRをファイルに書き出す。
     * 
     * @param filename 書き出すファイルのパス
     * @return true 書き出しに成功した場合
     * @return false 書き出しに失敗した場合
     */
    bool writeToFile(const std::string& filename);

    /**
     * @brief Typeを作成する。
     * この関数は_TypeSection::createTypeのラッパー関数。
     * 
     * @param typeVariant 作成する型
     * @return Type 作成した型
     */
    Type createType(const TypeVariant& typeVariant);
  private:
    bool declare(const std::string& parentName, const parser::ModuleTree& module);
    bool generate(const parser::ModuleTree& module);
  };
}

#endif // PICKC_PCIR_GENERATOR_H_