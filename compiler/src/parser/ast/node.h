/**
 * @file node.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief ASTのノードについて定義する
 * @version 0.1
 * @date 2021-07-24
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_PARSER_AST_NODE_H_
#define PICKC_PARSER_AST_NODE_H_

#include <memory>
#include <string>
#include <vector>

namespace pickc::parser {
  struct _ASTNode;
  struct _StatementNode;
  struct _FunctionNode;
  struct _ControlNode;
  struct _FormulaNode;
  struct _ImmediateNode;
  struct _IntegerNode;
  struct _BlockNode;
  struct _ConditionalNode;
  struct _RootNode;
  using ASTNode = std::shared_ptr<_ASTNode>;
  using StatementNode = std::shared_ptr<_StatementNode>;
  using FunctionNode = std::shared_ptr<_FunctionNode>;
  using ControlNode = std::shared_ptr<_ControlNode>;
  using FormulaNode = std::shared_ptr<_FormulaNode>;
  using ImmediateNode = std::shared_ptr<_ImmediateNode>;
  using IntegerNode = std::shared_ptr<_IntegerNode>;
  using BlockNode = std::shared_ptr<_BlockNode>;
  using ConditionalNode = std::shared_ptr<_ConditionalNode>;
  using RootNode = std::shared_ptr<_RootNode>;

  /**
   * @brief ASTNodeを作成するためのヘルパー関数。
   * ASTNodeをstd::shared_ptrで管理するため、
   * インスタンス化が少々面倒なので、
   * この関数を使用して作成する。
   * 
   * @tparam T ASTNodeの具象構造体
   * @tparam Value Tのコンストラクタ引数の型
   * @param values Tのコンストラクタ引数
   * @return T 作成したTのオブジェクト
   */
  template<typename T, typename... Value>
  T makeASTNode(Value&&... values) {
    return std::make_shared<std::remove_reference<decltype(*(T()))>::type>(std::forward<Value>(values)...);
  }

  /**
   * @brief 全てのASTノードの抽象構造体。
   * 子ノードをどう保持するかはこの構造体を継承した構造体に委ねる。
   */
  struct _ASTNode {
    std::weak_ptr<_RootNode> rootNode;
    std::weak_ptr<_ASTNode> parentNode;
    _ASTNode(const RootNode& rootNode, const ASTNode& parentNode);
    virtual ~_ASTNode();
    virtual std::string dump(const std::string& indent) const = 0;
  };
  /**
   * @brief 文を表現する抽象ノード。
   * 
   */
  struct _StatementNode : public _ASTNode {
    using _ASTNode::_ASTNode;
  };
  /**
   * @brief 関数を表現するノード。
   * 
   */
  struct _FunctionNode : public _StatementNode {
    std::string name;
    FormulaNode body;
    using _StatementNode::_StatementNode;
    virtual ~_FunctionNode() = default;
    virtual std::string dump(const std::string& indent) const override;
  };
  enum struct ControlNodeKind {
    Return,
    Break,
    Continue,
  };
  /**
   * @brief returnやbreak、continueなどの制御構文を表現するノード。
   * if、while、loop、forはConditionalNodeを使用する。
   */
  struct _ControlNode : public _StatementNode {
    ControlNodeKind kind;
    FormulaNode formula;
    _ControlNode(const RootNode& rootNode, const ASTNode& parentNode, ControlNodeKind kind);
    virtual ~_ControlNode() = default;
    virtual std::string dump(const std::string& indent) const override;
  };
  /**
   * @brief 式を表現する抽象ノード。
   * 
   */
  struct _FormulaNode : public _StatementNode {
    using _StatementNode::_StatementNode;
  };
  /**
   * @brief 即値を表現するノード。
   * 
   */
  struct _ImmediateNode : public _FormulaNode {
    using _FormulaNode::_FormulaNode;
  };
  /**
   * @brief 整数リテラルを表現するノード。
   * 
   */
  struct _IntegerNode : public _ImmediateNode {
    int value;
    _IntegerNode(const RootNode& rootNode, const ASTNode& parentNode, int value);
    ~_IntegerNode() = default;
    virtual std::string dump(const std::string& indent) const override;
  };
  /**
   * @brief ブロックを表現するノード。
   * 
   */
  struct _BlockNode : public _FormulaNode {
    std::vector<StatementNode> statements;
    using _FormulaNode::_FormulaNode;
    virtual std::string dump(const std::string& indent) const override;
  };
  /**
   * @brief ifやwhileなどの条件分岐を表現するノード。
   * 
   */
  struct _ConditionalNode : public _FormulaNode {};
  /**
   * @brief ASTの根になるノード。
   * 一つのモジュールを表現する。
   */
  struct _RootNode : public _ASTNode {
    /**
     * @brief このモジュール内に存在する無名関数の数。
     * 無名関数は暗黙的に${anonymousFunctionCount}という名前になる。
     */
    size_t anonymousFunctionCount;
    std::vector<FunctionNode> functions;
    _RootNode();
    virtual ~_RootNode() = default;
    virtual std::string dump(const std::string& indent) const override;
  };
}

#endif // PICKC_PARSER_AST_NODE_H_