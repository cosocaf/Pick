/**
 * @file node.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief node.hの実装
 * @version 0.1
 * @date 2021-07-24
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "node.h"

#include "output_buffer.h"

namespace pickc::parser {
  _ASTNode::_ASTNode(const RootNode& rootNode, const ASTNode& parentNode) :
    rootNode(rootNode), parentNode(parentNode) {}
  _ASTNode::~_ASTNode() {}

  std::string _FunctionNode::dump(const std::string& indent) const {
    std::string res = "FunctionNode(";
    res += name;
    res += ")\n";
    res += indent;
    res += "+-Arguments";
    // TODO: Dump arguments.
    res += '\n';
    res += indent;
    res += "+-Body\n";
    res += indent;
    res += "  +-";
    res += body->dump(indent + "    ");
    return res;
  }

  _ControlNode::_ControlNode(const RootNode& rootNode, const ASTNode& parentNode, ControlNodeKind kind) :
    _StatementNode(rootNode, parentNode),
    kind(kind) {};
  std::string _ControlNode::dump(const std::string& indent) const {
    std::string res;
    switch(kind) {
      case ControlNodeKind::Return:
        res = "ControlNode(Return)\n";
        break;
      case ControlNodeKind::Break:
        res = "ControlNode(Break)\n";
        break;
      case ControlNodeKind::Continue:
        res = "ControlNode(Continue)\n";
        break;
    }
    res += indent;
    res += "+-Result\n";
    res += indent;
    res += "  +-";
    res += formula->dump(indent + "    ");
    return res;
  }

  _IntegerNode::_IntegerNode(const RootNode& rootNode, const ASTNode& parentNode, int value) :
    _ImmediateNode(rootNode, parentNode),
    value(value) {};
  std::string _IntegerNode::dump(const std::string& indent) const {
    return "IntegerNode(" + std::to_string(value) + ")";
  }

  std::string _BlockNode::dump(const std::string& indent) const {
    std::string res = "BlockNode\n";
    res += indent;
    res += "+-Statements";
    for(size_t i = 0, l = statements.size(); i < l; ++i) {
      res += '\n';
      res += indent;
      res += "  ";
      std::string ind;
      if(i < l - 1) {
        res += "+-";
        ind = "  | ";
      }
      else {
        res += "+-";
        ind = "    ";
      }
      res += statements[i]->dump(indent + ind);
    }
    return res;
  }

  _RootNode::_RootNode() :
    _ASTNode(nullptr, nullptr),
    anonymousFunctionCount(0) {
  }
  std::string _RootNode::dump(const std::string& indent) const {
    std::string res = "RootNode\n";
    res += indent;
    res += "+-Functions";
    for(size_t i = 0, l = functions.size(); i < l; ++i) {
      res += '\n';
      res += indent;
      res += "  ";
      std::string ind;
      if(i < l - 1) {
        res += "+-";
        ind = "  | ";
      }
      else {
        res += "+-";
        ind = "    ";
      }
      res += functions[i]->dump(indent + ind);
    }
    return res;
  }
}