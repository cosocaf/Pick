#include "module_analyzer.h"

#include "utils/instanceof.h"
#include "utils/vector_utils.h"

namespace pickc::pcir
{
  Result<Register*, std::vector<std::string>> ModuleAnalyzer::exprAnalyze(const parser::ExpressionNode* expr, FlowNode** flow)
  {
    using namespace parser;
    if(instanceof<BlockNode>(expr)) {
      return blockAnalyze(dynamic_cast<const BlockNode*>(expr), flow);
    }
    else if(instanceof<VariableNode>(expr)) {
      return varAnalyze(dynamic_cast<const VariableNode*>(expr), flow);
    }
    else if(instanceof<LiteralNode>(expr)) {
      return literalAnalyze(dynamic_cast<const LiteralNode*>(expr), flow);
    }
    else if(instanceof<UnaryNode>(expr)) {
      return unaryAnalyze(dynamic_cast<const UnaryNode*>(expr), flow);
    }
    else if(instanceof<BinaryNode>(expr)) {
      return binaryAnalyze(dynamic_cast<const BinaryNode*>(expr), flow);
    }
    else if(instanceof<VariableDefineNode>(expr)) {
      return varDefAnalyze(dynamic_cast<const VariableDefineNode*>(expr), flow);
    }
    else if(instanceof<FunctionDefineNode>(expr)) {
      return fnDefAnalyze(dynamic_cast<const FunctionDefineNode*>(expr), flow);
    }
    else {
      assert(false);
    }
  }
}