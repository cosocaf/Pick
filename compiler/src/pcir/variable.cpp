/**
 * @file variable.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief variable.hの実装
 * @version 0.1
 * @date 2021-08-01
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "variable.h"

#include "output_buffer.h"

namespace pickc::pcir {
  Variable::Variable(const std::string& name, const Type& type, bool isMut, bool inited, const WeakVariableTable& variableTable):
    name(name), type(type), isMut(isMut), inited(inited), variableTable(variableTable) {}
  uint32_t Variable::getIndex() const {
    return variableTable.lock()->getIndex(*this);
  }
  std::string Variable::getName() const {
    return name;
  }
  Type Variable::getType() const {
    return type;
  }

  VariableTable _VariableTable::create() {
    return VariableTable(new _VariableTable());
  }
  Variable _VariableTable::createAnonymousVariable(const Type& type) {
    return createVariable(type, nextAnonymousVariableName());
  }
  Variable _VariableTable::createVariable(const Type& type, const std::string& name) {
    auto res = variables.try_emplace(name, name, type, false, false, shared_from_this());
    if(!res.second) {
      outputBuffer.emplace<CompileErrorMessage>("", 0, 0, 0, "変数" + name + "が複数定義されました。");
    }
    return res.first->second;
  }
  std::string _VariableTable::nextAnonymousVariableName() {
    return "@" + std::to_string(anonymousVariableCount++);
  }
  uint32_t _VariableTable::getIndex(const Variable& var) const {
    return static_cast<uint32_t>(std::distance(
      variables.begin(),
      variables.find(var.getName())
    ));
  }

  uint32_t _VariableTable::size() const {
    return variables.size();
  }
  std::map<std::string, Variable>::iterator _VariableTable::begin() {
    return variables.begin();
  }
  std::map<std::string, Variable>::iterator _VariableTable::end() {
    return variables.end();
  }
  std::map<std::string, Variable>::const_iterator _VariableTable::begin() const {
    return variables.begin();
  }
  std::map<std::string, Variable>::const_iterator _VariableTable::end() const {
    return variables.end();
  }
}