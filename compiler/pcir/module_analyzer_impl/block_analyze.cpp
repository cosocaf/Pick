#include "module_analyzer.h"

#include "utils/instanceof.h"
#include "utils/dyn_cast.h"
#include "utils/vector_utils.h"

namespace pickc::pcir
{
  Result<Register*, std::vector<std::string>> ModuleAnalyzer::blockAnalyze(const parser::BlockNode* block, FlowNode** flow)
  {
    using namespace parser;

    (*flow)->type = FlowType::Normal;

    auto child = new FlowNode();
    child->type = FlowType::Normal;
    child->parentFlow = *flow;
    child->belong = (*flow)->belong;
    child->belong->flows.push_back(child);

    (*flow)->nextFlow = child;

    std::vector<std::string> errors;
    for(auto node : block->nodes) {
      if(instanceof<ExpressionNode>(node)) {
        if(auto res = exprAnalyze(dynCast<ExpressionNode>(node), &child)) child->result = res.get();
        else errors += res.err();
      }
      else if(instanceof<ReturnNode>(node)) {
        child->result = nullptr;
        auto ret = dynCast<ReturnNode>(node);
        if(ret->value) {
          if(auto value = exprAnalyze(ret->value, &child)) {
            if(auto err = child->retNotice(value.get()->type)) {
              errors.push_back(createSemanticError(ret, err.get()));
            }
            else {
              child->type = FlowType::EndPoint;
              child->retReg = value.get();
            }
          }
          else {
            errors += value.err();
          }
        }
        else {
          if(auto err = child->retNotice(Type(Types::Void))) {
            errors.push_back(createSemanticError(ret, err.get()));
          }
          else{
            child->type = FlowType::EndPoint;
            child->retReg = nullptr;
          }
        }
      }
      else {
        assert(false);
      }
    }

    if(child->type != FlowType::EndPoint) {
      auto next = new FlowNode();
      next->parentFlow = *flow;
      next->belong = (*flow)->belong;
      next->belong->flows.push_back(next);
      child->type = FlowType::Normal;
      child->nextFlow = next;
      *flow = next;
    }

    if(errors.empty()) return ok(child->result);
    return error(errors);
  }
}