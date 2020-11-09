#include "module_analyzer.h"

#include "utils/vector_utils.h"

namespace pickc::pcir
{
  Result<Register*, std::vector<std::string>> ModuleAnalyzer::ifAnalyze(const parser::IfNode* ifNode, FlowNode** flow)
  {
    using namespace parser;

    auto result = new Register();
    result->type = Type(Types::Void);
    (*flow)->belong->regs.push_back(result);

    std::vector<std::string> errors;

    (*flow)->type = FlowType::Normal;

    auto ifFlow = new FlowNode();
    ifFlow->type = FlowType::ConditionalBranch;
    ifFlow->parentFlow = *flow;
    ifFlow->belong = (*flow)->belong;
    ifFlow->belong->flows.push_back(ifFlow);

    (*flow)->nextFlow = ifFlow;

    if(auto cond = exprAnalyze(ifNode->comp, &ifFlow)) ifFlow->cond = cond.get();
    else errors += cond.err();

    auto next = new FlowNode();
    next->type = FlowType::EndPoint;
    next->parentFlow = *flow;
    next->belong = (*flow)->belong;
    next->belong->flows.push_back(next);
    
    auto thenFlow = new FlowNode();
    thenFlow->type = FlowType::EndPoint;
    thenFlow->parentFlow = *flow;
    thenFlow->belong = (*flow)->belong;
    thenFlow->belong->flows.push_back(thenFlow);
    ifFlow->thenFlow = thenFlow;
    auto thenRes = exprAnalyze(ifNode->thenExpr, &thenFlow);
    if(thenRes) {
      if(thenRes.get() && thenRes.get()->curVar != nullptr) {
        if(thenRes.get()->curVar->status == VariableStatus::Uninited) {
          errors.push_back(createSemanticError(ifNode->thenExpr, "初期化されていない変数を返しました。"));
        }
        else if(thenRes.get()->curVar->status == VariableStatus::Moved) {
          errors.push_back(createSemanticError(ifNode->thenExpr, "既に移動された変数を返しました。"));
        }
      }
      thenFlow->type = FlowType::Normal;
      thenFlow->nextFlow = next;
    }
    else errors += thenRes.err();

    if(ifNode->elseExpr) {
      auto elseFlow = new FlowNode();
      elseFlow->type = FlowType::EndPoint;
      elseFlow->parentFlow = *flow;
      elseFlow->belong = (*flow)->belong;
      elseFlow->belong->flows.push_back(elseFlow);
      ifFlow->elseFlow = elseFlow;
      if(auto elseRes = exprAnalyze(ifNode->elseExpr, &elseFlow)) {
        elseFlow->type = FlowType::Normal;
        elseFlow->nextFlow = next;

        if(elseRes.get() && elseRes.get()->curVar != nullptr) {
          if(elseRes.get()->curVar->status == VariableStatus::Uninited) {
            errors.push_back(createSemanticError(ifNode->thenExpr, "初期化されていない変数を返しました。"));
          }
          else if(elseRes.get()->curVar->status == VariableStatus::Moved) {
            errors.push_back(createSemanticError(ifNode->thenExpr, "既に移動された変数を返しました。"));
          }
        }

        if(thenRes && elseRes && thenRes.get() && elseRes.get()) {
          result->type = Type::merge(Mov, thenRes.get()->type, elseRes.get()->type);

          auto phi = new PhiInstruction();
          phi->dist = result;
          phi->r1 = thenRes.get();
          phi->r2 = elseRes.get();
          next->insts.push_back(phi);
        }
      }
      else errors += elseRes.err();
    }

    *flow = next;

    if(errors.empty()) return ok(result);
    return error(errors);
  }
}