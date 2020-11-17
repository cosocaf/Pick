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
      if(auto block = blockAnalyze(dynCast<BlockNode>(expr), flow)) {
        if(block.get()) return ok(block.get());
        auto reg = new Register();
        reg->type = Types::Void;
        reg->vType = ValueType::RValue;
        (*flow)->addReg(reg);
        return ok(reg);
      }
      else {
        return error(block.err());
      }
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