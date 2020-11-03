#include "converter.h"

#include <filesystem>

#include "utils/vector_utils.h"

#include "pcir/pcir_code.h"
#include "pcir/pcir_format.h"

namespace pickc::ssa
{
  SSAConverter::SSAConverter() {}
  Result<SSABundle, std::vector<std::string>> SSAConverter::convert(const CompilerOption& option)
  {
    std::vector<std::string> errors;

    auto path = std::filesystem::path(option.outDir) / (option.out + ".pcir");
    auto res = pcir::PCIRLoader().load(path.string());
    if(!res) errors += res.err();
    pcirs.push_back(res.get());
    for(const auto& lib : option.libraries) {
      // TODO: Load PCIRs
    }

    if(!errors.empty()) return error(errors);

    for(const auto& pcir : pcirs) {
      for(const auto& symbol : pcir.symbolSection) {
        bundle.symbols.insert(symbol);
      }
      for(const auto& module : pcir.moduleSection) {
        for(const auto& symbol : module->symbols) {
          bundle.modules[module->name->text].insert(symbol);
        }
      }
    }

    for(const auto& pcir : pcirs) {
      for(const auto& fn : pcir.fnSection) {
        if(auto conv = fnConvert(pcir, fn)) {
          bundle.fns[fn] = conv.get();
        }
        else {
          errors += conv.err();
        }
      }
    }

    if(errors.empty()) return ok(bundle);
    return error(errors);
  }
  Result<SSAFunction, std::vector<std::string>> SSAConverter::fnConvert(const pcir::PCIRFile& pcir, const pcir::FunctionSection* pcirFn)
  {
    SSAFunction fn{};
    std::vector<std::string> errors;
    std::unordered_map<pcir::RegisterStruct*, SSARegister*> pcirRegs;
    auto flow = pcirFn->entryFlow;
    size_t lifeTime = 0;
    do {
      switch(flow->flowType) {
        case pcir::FLOW_TYPE_NORMAL:
          for(size_t i = 0, l = flow->normal.code.size(); i < l; ++i, ++lifeTime) {
            switch(flow->normal.code[i]) {
              case pcir::Add: {
                auto dist = pcirFn->regs[get32(flow->normal.code, i)];
                auto left = pcirFn->regs[get32(flow->normal.code, i)];
                auto right = pcirFn->regs[get32(flow->normal.code, i)];
                auto reg = new SSARegister();
                reg->type = dist->type;
                reg->lifeBegin = reg->lifeEnd = lifeTime;
                pcirRegs[dist] = reg;
                fn.regs.insert(reg);
                auto add = new SSAAddInstruction();
                add->dist = reg;
                add->left = pcirRegs[left];
                add->right = pcirRegs[right];
                add->left->lifeEnd = add->right->lifeEnd = lifeTime;
                fn.insts.push_back(add);
                break;
              }
              case pcir::Imm: {
                auto dist = pcirFn->regs[get32(flow->normal.code, i)];
                auto reg = new SSARegister();
                reg->type = dist->type;
                reg->lifeBegin = reg->lifeEnd = lifeTime;
                auto imm = new SSAImmInstruction();
                imm->dist = reg;
                switch(dist->type->types) {
                  case pcir::Types::I8:
                  case pcir::Types::U8:
                    imm->imm = get8(flow->normal.code, i);
                    break;
                  case pcir::Types::I16:
                  case pcir::Types::U16:
                    imm->imm = get16(flow->normal.code, i);
                    break;
                  case pcir::Types::I32:
                  case pcir::Types::U32:
                    imm->imm = get32(flow->normal.code, i);
                    break;
                  case pcir::Types::I64:
                  case pcir::Types::U64:
                    imm->imm = get64(flow->normal.code, i);
                    break;
                  default:
                    assert(false);
                }
                fn.regs.insert(reg);
                fn.insts.push_back(imm);
                pcirRegs[dist] = reg;
                break;
              }
              case pcir::Ret: {
                auto value = pcirFn->regs[get32(flow->normal.code, i)];
                auto ret = new SSARetInstruction();
                ret->value = pcirRegs[value];
                ret->value->lifeEnd = lifeTime;
                fn.insts.push_back(ret);
                break;
              }
              case pcir::LoadFn: {
                auto dist = pcirFn->regs[get32(flow->normal.code, i)];
                auto fnSec = pcir.fnSection[get32(flow->normal.code, i)];
                auto reg = new SSARegister();
                reg->type = dist->type;
                reg->lifeBegin = reg->lifeEnd = lifeTime;
                fn.regs.insert(reg);
                auto loadFn = new SSALoadFnInstruction();
                loadFn->dist = reg;
                loadFn->fn = fnSec;
                pcirRegs[dist] = reg;
                fn.insts.push_back(loadFn);
                break;
              }
              case pcir::Call: {
                auto dist = pcirFn->regs[get32(flow->normal.code, i)];
                auto call = pcirFn->regs[get32(flow->normal.code, i)];
                assert(call->type->types == pcir::Types::Function);
                std::vector<pcir::RegisterStruct*> args;
                for(size_t arg = 0; arg < call->type->numOfArgs; ++arg) {
                  args.push_back(pcirFn->regs[get32(flow->normal.code, i)]);
                }
                auto callInst = new SSACallInstruction();
                callInst->dist = new SSARegister();
                callInst->dist->type = dist->type;
                callInst->dist->lifeBegin = callInst->dist->lifeEnd = lifeTime;
                fn.regs.insert(callInst->dist);
                pcirRegs[dist] = callInst->dist;
                callInst->call = pcirRegs[call];
                callInst->call->lifeEnd = lifeTime;
                for(auto& arg : args) {
                  callInst->args.push_back(pcirRegs[arg]);
                  pcirRegs[arg]->lifeEnd = lifeTime;
                }
                fn.insts.push_back(callInst);
                break;
              }
              default:
                assert(false);
            }
          }
          flow = flow->normal.next;
          break;
        default:
          assert(false);
      }
    } while(flow != nullptr);
    if(errors.empty()) return ok(fn);
    return error(errors);
  }
}