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
#include <memory>

#include "token/token.h"
#include "ast/node.h"

namespace pickc::parser {
  class _ModuleTree;
  using ModuleTree = std::shared_ptr<_ModuleTree>;
  /**
   * @brief モジュールの親子関係を木構造で表現する。
   * 
   */
  class _ModuleTree : public std::enable_shared_from_this<_ModuleTree> {
    /**
     * @brief モージュール名。完全修飾名ではない。
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
     * @brief 親モジュール。
     * 
     */
    ModuleTree parentModule;
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
    _ModuleTree(const _ModuleTree& other);
    /**
     * @brief ムーブコンストラクタ。
     * 
     * @param other ムーブ元
     */
    _ModuleTree(_ModuleTree&& other);
    /**
     * @brief ModuleTreeを構築する。
     * 
     * @param name モジュール名
     * @param filename ソースファイルのパス
     * @param directoryModule ディレクトリモジュールであるならtrue
     */
    _ModuleTree(const std::string& name, const std::string& filename, bool directoryModule, const ModuleTree& parentModule);
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
     * @brief モジュール名を取得する。このモジュール名は完全修飾名ではない。
     * 完全修飾名が必要であれば、ModuleTree::getFullyQualifiedNameを参照。
     * 
     * @return std::string モジュール名
     */
    std::string getName() const;
    /**
     * @brief 完全修飾モジュール名を取得する。
     * 
     * @return std::string 完全修飾モジュール名
     */
    std::string getFullyQualifiedName() const;
    /**
     * @brief ソースファイルのパスを取得する。
     * 
     * @return std::string ソースファイルのパス
     */
    std::string getFilename() const;
    /**
     * @brief ディレクトリモジュールであるかを取得する。
     * 
     * @return true ディレクトリモジュールであるとき
     * @return false ディレクトリモジュールでないとき
     */
    bool isDirectoryModule() const;
    /**
     * @brief トークン列を取得する。
     * 
     * @return TokenSequence トークン列
     */
    TokenSequence getTokenSequence() const;
    void setTokenSequence(const TokenSequence& sequence);
    /**
     * @brief ルートノードを取得する。
     * 
     * @return RootNode ルートノード
     */
    RootNode getRootNode() const;
    void setRootNode(const RootNode& node);
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
    /**
     * @brief 子モジュールの開始イテレータを取得する。
     * 
     * @return std::vector<ModuleTree>::const_iterator 子モジュールの開始イテレータ
     */
    std::vector<ModuleTree>::const_iterator begin() const;
    /**
     * @brief 子モジュールの終了イテレータを取得する。
     * 
     * @return std::vector<ModuleTree>::const_iterator 子モジュールの終了イテレータ
     */
    std::vector<ModuleTree>::const_iterator end() const;
  };
}

#endif // PICKC_PARSER_MODULE_TREE_H_