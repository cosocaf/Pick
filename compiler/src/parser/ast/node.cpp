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

#include <cassert>

#include "utils/instanceof.h"
#include "utils/dyn_cast.h"

namespace pickc::parser {
  _ASTNode::_ASTNode(const RootNode& rootNode, const ASTNode& parentNode) :
    rootNode(rootNode), parentNode(parentNode) {}
  _ASTNode::~_ASTNode() {}

  _ControlNode::_ControlNode(const RootNode& rootNode, const ASTNode& parentNode, ControlNodeKind kind) :
    _StatementNode(rootNode, parentNode),
    kind(kind) {};

  _IntegerNode::_IntegerNode(const RootNode& rootNode, const ASTNode& parentNode, int value) :
    _ImmediateNode(rootNode, parentNode),
    value(value) {};

  _RootNode::_RootNode() :
    _ASTNode(nullptr, nullptr),
    anonymousFunctionCount(0) {
  }
}