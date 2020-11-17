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
    child->type = FlowType::Undecided;
    child->parentFlow = *flow;
    child->belong = (*flow)->belong;
    child->belong->flows.push_back(child);

    (*flow)->nextFlow = child;

    std::vector<std::string> errors;
    for(auto node : block->nodes) {
      if(instanceof<ExpressionNode>(node)) {
        if(auto res = exprAnalyze(dynCast<ExpressionNode>(node), &child)) {
          child->result = res.get();
          child->type = FlowType::Undecided;
        }
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
              child->retReg = value.get();
              child->result = child->retReg;
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
            child->retReg = nullptr;
          }
        }
        child->type = FlowType::EndPoint;
        // デッドコードも解析だけはする。
        auto deadFlow = new FlowNode();
        deadFlow->type = FlowType::Undecided;
        deadFlow->parentFlow = child;
        deadFlow->belong = child->belong;
        deadFlow->belong->flows.push_back(deadFlow);
        deadFlow->retReg = child->retReg;
        deadFlow->result = child->result;
        child = deadFlow;
      }
      else {
        assert(false);
      }
    }

    if(child->type == FlowType::Undecided) {
      auto next = new FlowNode();
      next->type = FlowType::Undecided;
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