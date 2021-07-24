/**
 * @file parser.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief パーサ
 * @version 0.1
 * @date 2021-07-22
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_PARSER_PARSER_H_
#define PICKC_PARSER_PARSER_H_

#include <optional>

#include "module_tree.h"

namespace pickc::parser {
  /**
   * @brief パーサ。
   * compilerOption::srcDirをルートモジュールとしてモジュールツリーを構築し、パースする。
   * 
   */
  class Parser {
    /**
     * @brief ルートモジュール
     * 
     */
    ModuleTree rootModule;
  public:
    /**
     * @brief パーサを構築する。
     * 
     */
    Parser();
    std::optional<ModuleTree> parse();
  private:
    void init();
    void constructModuleTree(const std::string& dir, ModuleTree& module);
    void tokenize(ModuleTree& module);
    void ast(ModuleTree& module);
  };
}

#endif // PICKC_PARSER_PARSER_H_