/**
 * @file variable.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-07-29
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_PCIR_VARIABLE_H_
#define PICKC_PCIR_VARIABLE_H_

#include <string>
#include <map>
#include <memory>

#include "type_section.h"

namespace pickc::pcir {
  class _VariableTable;
  using VariableTable = std::shared_ptr<_VariableTable>;
  class Variable {
    std::string name;
    Type type;
    bool isMut;
    bool inited;
    VariableTable variableTable;
  public:
    Variable(const std::string& name, const Type& type, bool isMut, bool inited, const VariableTable& variableTable);
    uint32_t getIndex() const;
    Type getType() const;
  };
  class _VariableTable : public std::enable_shared_from_this<_VariableTable> {
    friend class Variable;
    std::map<std::string, Variable> variables;
    size_t anonymousVariableCount;
  private:
    _VariableTable() = default;
  public:
    static VariableTable create();
  public:
    Variable createVariable(const Type& type, const std::string& name);
    Variable createAnonymousVariable(const Type& type);
    uint32_t size() const;
    std::map<std::string, Variable>::iterator begin();
    std::map<std::string, Variable>::iterator end();
    std::map<std::string, Variable>::const_iterator begin() const;
    std::map<std::string, Variable>::const_iterator end() const;
  private:
    std::string nextAnonymousVariableName();
  };
}

#endif // PICKC_PCIR_VARIABLE_H_