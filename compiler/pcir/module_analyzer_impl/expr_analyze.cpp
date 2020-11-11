#include "module_analyzer.h"

#include "utils/instanceof.h"
#include "utils/vector_utils.h"
#include "utils/dyn_cast.h"

namespace pickc::pcir
{
  Result<Register*, std::vector<std::string>> ModuleAnalyzer::exprAnalyze(const parser::ExpressionNode* expr, FlowNode** flow)
  {
    using namespace parser;
    if(instanceof<BlockNode>(expr)) {
      return blockAnalyze(dynCast<BlockNode>(expr), flow);
    }
    else if(instanceof<VariableNode>(expr)) {
      return varAnalyze(dynCast<VariableNode>(expr), flow);
    }
    else if(instanceof<LiteralNode>(expr)) {
      return literalAnalyze(dynCast<LiteralNode>(expr), flow);
    }
    else if(instanceof<UnaryNode>(expr)) {
      return unaryAnalyze(dynCast<UnaryNode>(expr), flow);
    }
    else if(instanceof<BinaryNode>(expr)) {
      return binaryAnalyze(dynCast<BinaryNode>(expr), flow);
    }
    else if(instanceof<VariableDefineNode>(expr)) {
      return varDefAnalyze(dynCast<VariableDefineNode>(expr), flow);
    }
    else if(instanceof<FunctionDefineNode>(expr)) {
      return fnDefAnalyze(dynCast<FunctionDefineNode>(expr), flow);
    }
    else if(instanceof<IfNode>(expr)) {
      return ifAnalyze(dynCast<IfNode>(expr), flow);
    }
    else if(instanceof<WhileNode>(expr)) {
      return whileAnalyze(dynCast<WhileNode>(expr), flow);
    }
    else {
      assert(false);
    }
  }
}