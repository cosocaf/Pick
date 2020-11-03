#include "module_analyzer.h"

#include "utils/instanceof.h"
#include "utils/vector_utils.h"

namespace pickc::pcir
{
  Result<Register*, std::vector<std::string>> ModuleAnalyzer::blockAnalyze(const parser::BlockNode* block, FlowNode** parent)
  {
    using namespace parser;
    
    auto flow = new FlowNode();
    flow->parentFlow = *parent;
    flow->belong = (*parent)->belong;
    flow->belong->flows.push_back(flow);
    (*parent)->nextFlow = flow;

    std::vector<std::string> errors;
    for(auto node : block->nodes) {
      if(instanceof<ExpressionNode>(node)) {
        if(auto res = exprAnalyze(dynamic_cast<const ExpressionNode*>(node), &flow)) flow->result = res.get();
        else errors += res.err();
      }
      else if(instanceof<ReturnNode>(node)) {
        flow->result = nullptr;
        auto ret = dynamic_cast<const ReturnNode*>(node);
        if(ret->value) {
          if(auto value = exprAnalyze(ret->value, &flow)) {
            if(auto err = flow->retNotice(value.get()->type)) {
              errors.push_back(createSemanticError(ret, err.get()));
            }
            else {
              auto inst = new ReturnInstruction();
              inst->reg = value.get();
              flow->insts.push_back(inst);
            }
          }
          else {
            errors += value.err();
          }
        }
        else {
          if(auto err = flow->retNotice(Type(Types::Void))) {
            errors.push_back(createSemanticError(ret, err.get()));
          }
          else{
            auto inst = new ReturnInstruction();
            inst->reg = nullptr;
            flow->insts.push_back(inst);
          }
        }
      }
      else {
        assert(false);
      }
    }

    auto next = new FlowNode();
    next->parentFlow = *parent;
    next->belong = (*parent)->belong;
    next->belong->flows.push_back(next);
    flow->nextFlow = next;
    *parent = next;

    if(errors.empty()) return ok(flow->result);
    return error(errors);
  }
}