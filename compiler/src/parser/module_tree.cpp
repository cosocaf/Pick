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
  ModuleTree::ModuleTree(const ModuleTree& other) :
    name(other.name),
    filename(other.filename),
    directoryModule(other.directoryModule),
    submodules(other.submodules),
    tokenSequence(other.tokenSequence),
    rootNode(other.rootNode) {}
  ModuleTree::ModuleTree(ModuleTree&& other) :
    name(std::move(other.name)),
    filename(std::move(other.filename)),
    directoryModule(other.directoryModule),
    submodules(std::move(other.submodules)),
    tokenSequence(std::move(other.tokenSequence)),
    rootNode(std::move(other.rootNode)) {}
  ModuleTree::ModuleTree(const std::string& name, const std::string& filename, bool directoryModule) :
    name(name),
    filename(filename),
    directoryModule(directoryModule) {}

  ModuleTree& ModuleTree::push(const std::string& subname, const std::string& subfilename, bool subdirectoryModule) {
    return submodules.emplace_back(subname, subfilename, subdirectoryModule);
  }
  ModuleTree& ModuleTree::push(const ModuleTree& module) {
    return submodules.emplace_back(module);
  }
  std::string& ModuleTree::getName() {
    return name;
  }
  std::string& ModuleTree::getFilename() {
    return filename;
  }
  bool ModuleTree::isDirectoryModule() {
    return directoryModule;
  }
  TokenSequence& ModuleTree::getTokenSequence() {
    return tokenSequence;
  }
  RootNode& ModuleTree::getRootNode() {
    return rootNode;
  }
  std::vector<ModuleTree>::iterator ModuleTree::begin() {
    return submodules.begin();
  }
  std::vector<ModuleTree>::iterator ModuleTree::end() {
    return submodules.end();
  }
}