/**
 * @file module_tree.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief module_tree.hの実装
 * @version 0.1
 * @date 2021-07-23
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "module_tree.h"

namespace pickc::parser {
  _ModuleTree::_ModuleTree(const _ModuleTree& other) :
    name(other.name),
    filename(other.filename),
    directoryModule(other.directoryModule),
    parentModule(other.parentModule),
    submodules(other.submodules),
    tokenSequence(other.tokenSequence),
    rootNode(other.rootNode) {}
  _ModuleTree::_ModuleTree(_ModuleTree&& other) :
    name(std::move(other.name)),
    filename(std::move(other.filename)),
    directoryModule(other.directoryModule),
    parentModule(std::move(other.parentModule)),
    submodules(std::move(other.submodules)),
    tokenSequence(std::move(other.tokenSequence)),
    rootNode(std::move(other.rootNode)) {}
  _ModuleTree::_ModuleTree(const std::string& name, const std::string& filename, bool directoryModule, const ModuleTree& parentModule) :
    name(name),
    filename(filename),
    directoryModule(directoryModule),
    parentModule(parentModule) {}

  ModuleTree& _ModuleTree::push(const std::string& subname, const std::string& subfilename, bool subdirectoryModule) {
    return submodules.emplace_back(
      std::make_shared<_ModuleTree>(
        subname,
        subfilename,
        subdirectoryModule,
        shared_from_this()
      )
    );
  }
  std::string _ModuleTree::getName() const {
    return name;
  }
  std::string _ModuleTree::getFullyQualifiedName() const {
    if(parentModule) {
      return parentModule->getFullyQualifiedName() + "::" + name;
    }
    return name;
  }
  std::string _ModuleTree::getFilename() const {
    return filename;
  }
  bool _ModuleTree::isDirectoryModule() const {
    return directoryModule;
  }
  TokenSequence _ModuleTree::getTokenSequence() const {
    return tokenSequence;
  }
  void _ModuleTree::setTokenSequence(const TokenSequence& sequence) {
    tokenSequence = sequence;
  }
  RootNode _ModuleTree::getRootNode() const {
    return rootNode;
  }
  void _ModuleTree::setRootNode(const RootNode& node) {
    rootNode = node;
  }
  std::vector<ModuleTree>::iterator _ModuleTree::begin() {
    return submodules.begin();
  }
  std::vector<ModuleTree>::iterator _ModuleTree::end() {
    return submodules.end();
  }
  std::vector<ModuleTree>::const_iterator _ModuleTree::begin() const {
    return submodules.begin();
  }
  std::vector<ModuleTree>::const_iterator _ModuleTree::end() const {
    return submodules.end();
  }
}