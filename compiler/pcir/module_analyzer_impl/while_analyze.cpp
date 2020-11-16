#include "module_analyzer.h"

#include "utils/vector_utils.h"

namespace pickc::pcir
{
  Result<Register*, std::vector<std::string>> ModuleAnalyzer::whileAnalyze(const parser::WhileNode* whileNode, FlowNode** flow)
  {
    auto result = new Register();
    result->type = Type(Types::Void);
    (*flow)->belong->regs.push_back(result);

    std::vector<std::string> errors;

    (*flow)->type = FlowType::Normal;

    auto whileFlow = new FlowNode();
    whileFlow->type = FlowType::Undecided;
    whileFlow->parentFlow = *flow;
    whileFlow->belong = (*flow)->belong;
    whileFlow->belong->flows.push_back(whileFlow);
    (*flow)->nextFlow = whileFlow;

    auto begin = whileFlow->currentVars();

    if(auto cond = exprAnalyze(whileNode->comp, &whileFlow)) {
      if(whileFlow->type != FlowType::EndPoint) {
        whileFlow->type = FlowType::ConditionalBranch;
        whileFlow->cond = cond.get();
      }
    }
    else errors += cond.err();

    auto bodyFlow = new FlowNode();
    bodyFlow->type = FlowType::Undecided;
    bodyFlow->parentFlow = *flow;
    bodyFlow->belong = (*flow)->belong;
    bodyFlow->belong->flows.push_back(bodyFlow);

    auto next = new FlowNode();
    next->type = FlowType::Undecided;
    next->parentFlow = *flow;
    next->belong = (*flow)->belong;
    next->belong->flows.push_back(next);

    whileFlow->thenFlow = bodyFlow;
    whileFlow->elseFlow = next;
    auto thenRes = exprAnalyze(whileNode->body, &bodyFlow);
    if(!thenRes) errors += thenRes.err();
    
    if(!errors.empty()) return error(errors);

    auto end = bodyFlow->currentVars();

    for(auto& var : begin) {
      if(end[var.first] != var.second) {
        auto phi = new PhiInstruction();
        phi->dist = new Register();
        phi->r1 = var.second;
        phi->r2 = end[var.first];
        phi->dist->type = var.second->type;
        phi->dist->curVar = var.second->curVar;
        var.second->curVar->reg = phi->dist;
        whileFlow->insts.push_back(phi);
        whileFlow->addReg(phi->dist);
      }
    }

    if(bodyFlow->type == FlowType::Undecided) {
      bodyFlow->type = FlowType::Normal;
      bodyFlow->nextFlow = whileFlow;
    }

    *flow = next;

    return ok(nullptr);
  }
}