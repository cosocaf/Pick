#ifndef PICKC_PCIR_MODULE_ANALYZER_H_
#define PICKC_PCIR_MODULE_ANALYZER_H_

#include <unordered_set>

#include "pickc/module_tree.h"
#include "utils/option.h"

namespace pickc::pcir
{
  class SemanticAnalyzer;
  class ModuleAnalyzer
  {
    SemanticAnalyzer* sa;
    ModuleTree* tree;
    std::unordered_set<ModuleTree*> trees;
    std::string createSemanticError(const parser::Node* node, const std::string& message);
    Result<Register*, std::vector<std::string>> exprAnalyze(const parser::ExpressionNode* expr, FlowNode** flow);
    // 何も返さない場合はnullptrを返す。
    Result<Register*, std::vector<std::string>> blockAnalyze(const parser::BlockNode* block, FlowNode** flow);
    Result<Register*, std::vector<std::string>> varAnalyze(const parser::VariableNode* var, FlowNode** flow);
    Result<Register*, std::vector<std::string>> literalAnalyze(const parser::LiteralNode* literal, FlowNode** flow);
    Result<Register*, std::vector<std::string>> unaryAnalyze(const parser::UnaryNode* unary, FlowNode** flow);
    Result<Register*, std::vector<std::string>> binaryAnalyze(const parser::BinaryNode* binary, FlowNode** flow);
    Result<Register*, std::vector<std::string>> varDefAnalyze(const parser::VariableDefineNode* varDef, FlowNode** flow);
    Result<Register*, std::vector<std::string>> fnDefAnalyze(const parser::FunctionDefineNode* fnDef, FlowNode** flow);
    Result<Register*, std::vector<std::string>> ifAnalyze(const parser::IfNode* ifNode, FlowNode** flow);
    Result<Symbol*, std::vector<std::string>> findGlobalVar(const parser::VariableNode* var);
    Result<Register*, std::vector<std::string>> whileAnalyze(const parser::WhileNode* whileNode, FlowNode** flow);
  public:
    ModuleAnalyzer(SemanticAnalyzer* sa, ModuleTree* tree, const std::unordered_set<ModuleTree*>& trees);
    Option<std::vector<std::string>> declare();
    Option<std::vector<std::string>> analyze();
  };
}

#endif // PICKC_PCIR_MODULE_ANALYZER_H_