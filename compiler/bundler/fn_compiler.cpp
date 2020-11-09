#include "fn_compiler.h"

#include <sstream>

#include "pcir/pcir_format.h"
#include "utils/vector_utils.h"
#include "utils/map_utils.h"
#include "utils/set_utils.h"

namespace pickc::bundler
{
  FnCompiler::FnCompiler(Bundle* bundle, pcir::PCIRFile* pcir, pcir::FunctionSection* pcirFn) : bundle(bundle), pcir(pcir), pcirFn(pcirFn) {}
  Result<Function*, std::vector<std::string>> FnCompiler::compile()
  {
    std::vector<std::string> errors;
    fn = new Function();
    fn->type = pcirFn->type;

    flowCompile(pcirFn->entryFlow);
    joinFlows(pcirFn->entryFlow);
    for(auto& jmp : jmpTo) {
      jmp.first->then = flowIndexes[jmp.second.first];
      if(jmp.second.second) {
        jmp.first->els = flowIndexes[jmp.second.second];
      }
    }
    for(auto& reg : regs) {
      fn->regs.insert(reg.second);
    }

    if(errors.empty()) return ok(fn);
    return error(errors);
  }

  namespace {
    template<typename Instruction>
    void binaryInstructions(std::unordered_map<pickc::pcir::FlowStruct *, std::vector<pickc::bundler::Instruction *>>& flows, pcir::FlowStruct* flow, pcir::FunctionSection* pcirFn, std::unordered_map<pcir::RegisterStruct*, Register*>& regs, Function* fn, size_t& i, std::vector<std::string>& errors)
    {
      auto distIndex = get32(flow->code, i);
      auto leftIndex = get32(flow->code, i);
      auto rightIndex = get32(flow->code, i);
      assert(keyExists(regs, pcirFn->regs[leftIndex]));
      assert(keyExists(regs, pcirFn->regs[rightIndex]));
      auto left = regs[pcirFn->regs[leftIndex]];
      auto right = regs[pcirFn->regs[rightIndex]];
      Register* dist;
      if(!keyExists(regs, pcirFn->regs[distIndex])) {
        dist = new Register();
        dist->type = pcirFn->regs[distIndex]->type;
        regs[pcirFn->regs[distIndex]] = dist;
      }
      else {
        dist = regs[pcirFn->regs[distIndex]];
      }
      auto inst = new Instruction();
      inst->dist = dist;
      inst->left = left;
      inst->right = right;
      flows[flow].push_back(inst);
    };
  }
  Result<_, std::vector<std::string>> FnCompiler::flowCompile(pcir::FlowStruct* flow)
  {
    using namespace pcir;
    if(keyExists(flows, flow)) return ok();
    flows[flow] = {};
    std::vector<std::string> errors;
    for(size_t i = 0, l = flow->code.size(); i < l; ++i) {
      switch(flow->code[i]) {
        case Add: binaryInstructions<AddInstruction>(flows, flow, pcirFn, regs, fn, i, errors); break;
        case Sub: binaryInstructions<SubInstruction>(flows, flow, pcirFn, regs, fn, i, errors); break;
        case Mul: binaryInstructions<MulInstruction>(flows, flow, pcirFn, regs, fn, i, errors); break;
        case Div: binaryInstructions<DivInstruction>(flows, flow, pcirFn, regs, fn, i, errors); break;
        case Mod: binaryInstructions<ModInstruction>(flows, flow, pcirFn, regs, fn, i, errors); break;
        case Inc:
          // TODO
          assert(false);
          break;
        case Dec:
          // TODO
          assert(false);
          break;
        case Pos:
          // TODO
          assert(false);
          break;
        case Neg:
          // TODO
          assert(false);
          break;
        case EQ: binaryInstructions<EqInstruction>(flows, flow, pcirFn, regs, fn, i, errors); break;
        case NEQ: binaryInstructions<NeqInstruction>(flows, flow, pcirFn, regs, fn, i, errors); break;
        case GT: binaryInstructions<GtInstruction>(flows, flow, pcirFn, regs, fn, i, errors); break;
        case GE: binaryInstructions<GeInstruction>(flows, flow, pcirFn, regs, fn, i, errors); break;
        case LT: binaryInstructions<LtInstruction>(flows, flow, pcirFn, regs, fn, i, errors); break;
        case LE: binaryInstructions<LeInstruction>(flows, flow, pcirFn, regs, fn, i, errors); break;
        case Imm: {
          auto index = get32(flow->code, i);
          Register* dist;
          if(!keyExists(regs, pcirFn->regs[index])) {
            dist = new Register();
            dist->type = pcirFn->regs[index]->type;
            regs[pcirFn->regs[index]] = dist;
          }
          else {
            dist = regs[pcirFn->regs[index]];
          }
          auto inst = new ImmInstruction();
          inst->dist = dist;
          switch(pcirFn->regs[index]->type->types) {
            case Types::I8: inst->imm = get8(flow->code, i); break;
            case Types::I16: inst->imm = get16(flow->code, i); break;
            case Types::I32: inst->imm = get32(flow->code, i); break;
            case Types::I64: inst->imm = get64(flow->code, i); break;
            case Types::U8: inst->imm = get8(flow->code, i); break;
            case Types::U16: inst->imm = get16(flow->code, i); break;
            case Types::U32: inst->imm = get32(flow->code, i); break;
            case Types::U64: inst->imm = get64(flow->code, i); break;
            default: assert(false); errors.push_back("PCIRエラー: 不正な型のレジスタに即値を代入しました。");
          }
          flows[flow].push_back(inst);
          break;
        }
        case Call: {
          auto dist = get32(flow->code, i);
          auto fn = get32(flow->code, i);
          if(!pcirFn->regs[fn]->type->type.isFn()) {
            std::stringstream ss;
            ss << "PCIRエラー: レジスタ#" << fn << "は関数ではありません。";
            errors.push_back(ss.str());
          }
          if(!keyExists(regs, pcirFn->regs[dist])) {
            regs[pcirFn->regs[dist]] = new Register();
            regs[pcirFn->regs[dist]]->type = pcirFn->regs[dist]->type;
          }
          auto inst = new CallInstruction();
          inst->dist = regs[pcirFn->regs[dist]];
          inst->fn = regs[pcirFn->regs[fn]];
          for(size_t j = 0, l = pcirFn->regs[fn]->type->type.fn.args.size(); j < l; ++j) {
            auto arg = get32(flow->code, i);
            inst->args.push_back(regs[pcirFn->regs[arg]]);
          }
          flows[flow].push_back(inst);
          break;
        }
        case LoadFn: {
          auto dist = get32(flow->code, i);
          auto src = get32(flow->code, i);
          if(keyExists(regs, pcirFn->regs[dist])) {
            std::stringstream ss;
            ss << "PCIRエラー: レジスタ#" << dist << "は既に使用されています。";
            errors.push_back(ss.str());
          }
          regs[pcirFn->regs[dist]] = new Register();
          regs[pcirFn->regs[dist]]->type = pcirFn->regs[dist]->type;
          auto inst = new LoadFnInstruction();
          inst->dist = regs[pcirFn->regs[dist]];
          inst->fn = pcir->fnSection[src];
          flows[flow].push_back(inst);
          break;
        }
        case LoadArg: {
          auto dist = get32(flow->code, i);
          auto arg = get32(flow->code, i);
          if(keyExists(regs, pcirFn->regs[dist])) {
            std::stringstream ss;
            ss << "PCIRエラー: レジスタ#" << dist << "は既に使用されています。";
            errors.push_back(ss.str());
          }
          regs[pcirFn->regs[dist]] = new Register();
          regs[pcirFn->regs[dist]]->type = pcirFn->regs[dist]->type;
          auto inst = new LoadArgInstruction();
          inst->dist = regs[pcirFn->regs[dist]];
          inst->indexOfArg = arg;
          flows[flow].push_back(inst);
          break;
        }
        case LoadSymbol: {
          auto dist = get32(flow->code, i);
          auto sym = get32(flow->code, i);
          if(keyExists(regs, pcirFn->regs[dist])) {
            std::stringstream ss;
            ss << "PCIRエラー: レジスタ#" << dist << "は既に使用されています。";
            errors.push_back(ss.str());
          }
          regs[pcirFn->regs[dist]] = new Register();
          regs[pcirFn->regs[dist]]->type = pcirFn->regs[dist]->type;
          auto inst = new LoadSymbolInstruction();
          inst->dist = regs[pcirFn->regs[dist]];
          inst->symbol = bundle->symbolNames[pcir->textSection[sym]->text];
          flows[flow].push_back(inst);
          break;
        }
        case Mov:
          // TODO
          assert(false);
          break;
        case Phi: {
          auto dist = get32(flow->code, i);
          auto r1 = get32(flow->code, i);
          auto r2 = get32(flow->code, i);
          if(!keyExists(regs, pcirFn->regs[dist])) {
            regs[pcirFn->regs[dist]] = new Register();
            regs[pcirFn->regs[dist]]->type = pcirFn->regs[dist]->type;
          }
          if(!keyExists(regs, pcirFn->regs[r1])) {
            regs[pcirFn->regs[r1]] = new Register();
            regs[pcirFn->regs[r1]]->type = pcirFn->regs[r1]->type;
          }
          if(!keyExists(regs, pcirFn->regs[r2])) {
            regs[pcirFn->regs[r2]] = new Register();
            regs[pcirFn->regs[r2]]->type = pcirFn->regs[r2]->type;
          }
          if(keyExists(fn->phis, regs[pcirFn->regs[dist]])) {
            fn->phis[regs[pcirFn->regs[dist]]]->regs.insert(regs[pcirFn->regs[r1]]);
            fn->phis[regs[pcirFn->regs[dist]]]->regs.insert(regs[pcirFn->regs[r2]]);
          }
          else if(keyExists(fn->phis, regs[pcirFn->regs[r1]])) {
            fn->phis[regs[pcirFn->regs[r1]]]->regs.insert(regs[pcirFn->regs[dist]]);
            fn->phis[regs[pcirFn->regs[r1]]]->regs.insert(regs[pcirFn->regs[r2]]);
          }
          else if(keyExists(fn->phis, regs[pcirFn->regs[r2]])) {
            fn->phis[regs[pcirFn->regs[r2]]]->regs.insert(regs[pcirFn->regs[dist]]);
            fn->phis[regs[pcirFn->regs[r2]]]->regs.insert(regs[pcirFn->regs[r1]]);
          }
          else {
            auto group = new PhiGroup();
            group->regs.insert(regs[pcirFn->regs[dist]]);
            group->regs.insert(regs[pcirFn->regs[r1]]);
            group->regs.insert(regs[pcirFn->regs[r2]]);
            fn->phis[regs[pcirFn->regs[dist]]] = group;
            fn->phis[regs[pcirFn->regs[r1]]] = group;
            fn->phis[regs[pcirFn->regs[r2]]] = group;
          }
          break;
        }
        default: {
          assert(false);
          std::stringstream ss;
          ss << "PCIRエラー: 予期しない命令コード: 0x" << std::hex << (int)flow->code[i];
          errors.push_back(ss.str());
        }
      }
    }
    if(flow->flowType & pcir::FLOW_TYPE_NORMAL) {
      auto jmp = new JmpInstruction();
      jmp->cond = nullptr;
      flows[flow].push_back(jmp);
      jmpTo[jmp] = { flow->next, nullptr};
      auto res = flowCompile(flow->next);
      if(!res) errors += res.err();
    }
    else if(flow->flowType & pcir::FLOW_TYPE_COND_BRANCH) {
      auto res = flowCompile(flow->thenFlow);
      if(!res) errors += res.err();
      res = flowCompile(flow->elseFlow);
      if(!res) errors += res.err();
      auto jmp = new JmpInstruction();
      jmp->cond = regs[flow->cond];
      flows[flow].push_back(jmp);
      jmpTo[jmp] = { flow->thenFlow, flow->elseFlow };
    }
    else if(flow->flowType & pcir::FLOW_TYPE_END_POINT) {
      auto ret = new RetInstruction();
      if(flow->retReg != nullptr) {
        ret->value = regs[flow->retReg];
      }
      else {
        ret->value = nullptr;
      }
      flows[flow].push_back(ret);
    }
    else {
      assert(false);
      std::stringstream ss;
      ss << "PCIRエラー: flow typeが予期しないものでした。flow type: 0x" << std::hex << flow->flowType;
      errors.push_back(ss.str());
      return error(errors);
    }
    if(errors.empty()) return ok();
    return error(errors);
  }

  void FnCompiler::joinFlows(pcir::FlowStruct* flow)
  {
    if(exists(joinedFlows, flow)) return;
    flowIndexes[flow] = fn->insts.size();
    fn->insts += flows[flow];
    joinedFlows.insert(flow);
    if(flow->flowType & pcir::FLOW_TYPE_NORMAL) {
      joinFlows(flow->next);
    }
    else if(flow->flowType & pcir::FLOW_TYPE_COND_BRANCH) {
      // elseを先にくっつけたほうが、elseを考慮せずに済むためジャンプ命令が減る。
      joinFlows(flow->elseFlow);
      joinFlows(flow->thenFlow);
    }
    else if(flow->flowType & pcir::FLOW_TYPE_END_POINT) {
      // do nothing.
    }
    else {
      assert(false);
    }
  }
}