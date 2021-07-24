/**
 * @file module_tree.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief モジュールの親子関係をまとめる
 * @version 0.1
 * @date 2021-07-23
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_PARSER_MODULE_TREE_H_
#define PICKC_PARSER_MODULE_TREE_H_

#include <vector>
#include <string>
#include <iterator>
#include "token/token.h"
#include "ast/node.h"

namespace pickc::parser {
  /**
   * @brief モジュールの親子関係を木構造で表現する。
   * 
   */
  class ModuleTree {
    /**
     * @brief モージュール名。
     * 
     */
    std::string name;
    /**
     * @brief このモジュールのもととなるソースファイルへのパス。
     * 
     */
    std::string filename;
    /**
     * @brief 子モジュールをまとめるためのモジュールであるかどうか。
     * 
     */
    bool directoryModule;
    /**
     * @brief 子モジュールの一覧。
     * 
     */
    std::vector<ModuleTree> submodules;
    /**
     * @brief このモジュールのトークン列。
     * 
     */
    TokenSequence tokenSequence;
    /**
     * @brief このモジュールのAST木のルートノード。
     * 
     */
    RootNode rootNode;
  public:
    /**
     * @brief コピーコンストラクタ。
     * 
     * @param other コピー元
     */
    ModuleTree(const ModuleTree& other);
    /**
     * @brief ムーブコンストラクタ。
     * 
     * @param other ムーブ元
     */
    ModuleTree(ModuleTree&& other);
    /**
     * @brief ModuleTreeを構築する。
     * 
     * @param name モジュール名
     * @param filename ソースファイルのパス
     * @param directoryModule ディレクトリモジュールであるならtrue
     */
    ModuleTree(const std::string& name, const std::string& filename, bool directoryModule);
    /**
     * @brief 子モジュールを追加する。
     * 
     * @param subname 子モジュール名
     * @param filename 子モジュールのソースファイルのパス
     * @param directoryModule 子モジュールがディレクトリモジュールであるならtrue
     * @return ModuleTree& 作成した子モジュールへの参照
     */
    ModuleTree& push(const std::string& subname, const std::string& filename, bool directoryModule);
    /**
     * @brief 子モジュールを追加する。
     * 
     * @param module 追加するモジュール
     * @return ModuleTree& 追加したモジュールへの参照
     */
    ModuleTree& push(const ModuleTree& module);
    /**
     * @brief モジュール名を取得する。
     * 
     * @return std::string& モジュール名
     */
    std::string& getName();
    /**
     * @brief ソースファイルのパスを取得する。
     * 
     * @return std::string& ソースファイルのパス
     */
    std::string& getFilename();
    /**
     * @brief ディレクトリモジュールであるかを取得する。
     * 
     * @return true ディレクトリモジュールであるとき
     * @return false ディレクトリモジュールでないとき
     */
    bool isDirectoryModule();
    /**
     * @brief トークン列を取得する。
     * 
     * @return TokenSequence& トークン列
     */
    TokenSequence& getTokenSequence();
    /**
     * @brief ルートノードを取得する。
     * 
     * @return RootNode ルートノード
     */
    RootNode& getRootNode();
    /**
     * @brief 子モジュールの開始イテレータを取得する。
     * 
     * @return std::vector<ModuleTree>::iterator 子モジュールの開始イテレータ
     */
    std::vector<ModuleTree>::iterator begin();
    /**
     * @brief 子モジュールの終了イテレータを取得する。
     * 
     * @return std::vector<ModuleTree>::iterator 子モジュールの終了イテレータ
     */
    std::vector<ModuleTree>::iterator end();
  };
}

#endif // PICKC_PARSER_MODULE_TREE_H_