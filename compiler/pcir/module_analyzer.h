#ifndef PICKC_PCIR_MODULE_ANALYZER_H_
#define PICKC_PCIR_MODULE_ANALYZER_H_

#include <memory>

#include "pickc/module_tree.h"
#include "utils/option.h"

namespace pickc::pcir
{
  class ModuleAnalyzer
  {
    ModuleTree* tree;
    std::string createSemanticError(const parser::Node* node, const std::string& message);
    // void型を返す時はnullptrを返す。
    Result<Register*, std::vector<std::string>> exprAnalyze(const parser::ExpressionNode* expr, FlowNode** flow);
    // 何も返さない場合はnullptrを返す。
    Result<Register*, std::vector<std::string>> blockAnalyze(const parser::BlockNode* block, FlowNode** flow);
    Result<Register*, std::vector<std::string>> varAnalyze(const parser::VariableNode* var, FlowNode** flow);
    Result<Register*, std::vector<std::string>> literalAnalyze(const parser::LiteralNode* literal, FlowNode** flow);
    Result<Register*, std::vector<std::string>> unaryAnalyze(const parser::UnaryNode* unary, FlowNode** flow);
    Result<Register*, std::vector<std::string>> binaryAnalyze(const parser::BinaryNode* binary, FlowNode** flow);
    Result<Register*, std::vector<std::string>> varDefAnalyze(const parser::VariableDefineNode* varDef, FlowNode** flow);
    Result<Register*, std::vector<std::string>> fnDefAnalyze(const parser::FunctionDefineNode* fnDef, FlowNode** flow);
  public:
    ModuleAnalyzer(ModuleTree* tree);
    Option<std::vector<std::string>> declare();
    Option<std::vector<std::string>> analyze();
  };
}

#endif // PICKC_PCIR_MODULE_ANALYZER_H_