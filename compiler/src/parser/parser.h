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
    /**
     * @brief パースする。
     * 
     * @return std::optional<ModuleTree> パースの結果。失敗した場合std::nullopt。
     */
    std::optional<ModuleTree> parse();
  private:
    /**
     * @brief パーサを初期化する。
     * 
     */
    void init();
    /**
     * @brief モジュールツリーを構築する。
     * 
     * @param dir モジュールのソースファイルが存在するディレクトリ
     * @param module 親モジュール
     */
    void constructModuleTree(const std::string& dir, ModuleTree& module);
    /**
     * @brief Tokenizerを使用して、字句解析をする。
     * 
     * @param module module::tokenSequenceに結果を格納する。
     */
    void tokenize(ModuleTree& module);
    /**
     * @brief ASTGeneratorを使用して、ASTを作成する。
     * 
     * @param module module::rootNodeに結果を格納する。
     */
    void ast(ModuleTree& module);
  };
}

#endif // PICKC_PARSER_PARSER_H_