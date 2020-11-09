#define _CRT_SECURE_NO_WARNINGS

#include "semantic_analyzer.h"

#include <fstream>
#include <filesystem>
#include <ctime>

#include "pickc/config.h"
#include "utils/vector_utils.h"
#include "utils/set_utils.h"
#include "utils/map_utils.h"
#include "utils/binary_vec.h"
#include "utils/instanceof.h"
#include "utils/dyn_cast.h"
#include "module_analyzer.h"
#include "pcir_format.h"
#include "pcir_dump.h"

namespace pickc::pcir
{
  SemanticAnalyzer::SemanticAnalyzer(ModuleTree* rootTree) : rootTree(rootTree) {}
  Option<std::vector<std::string>> SemanticAnalyzer::declare(ModuleTree* tree)
  {
    std::vector<std::string> errors;
    if(auto errs = ModuleAnalyzer(this, tree).declare()) errors += errs.get();
    for(auto sub : tree->submodules) if(auto errs = declare(sub.second)) errors += errs.get();
    if(errors.empty()) return none;
    return some(errors);
  }
  Option<std::vector<std::string>> SemanticAnalyzer::analyze(ModuleTree* tree)
  {
    std::vector<std::string> errors;
    if(auto errs = ModuleAnalyzer(this, tree).analyze()) errors += errs.get();
    for(auto sub : tree->submodules) if(auto errs = analyze(sub.second)) errors += errs.get();
    if(errors.empty()) return none;
    return some(errors);
  }
  void SemanticAnalyzer::findModules(ModuleTree* mod)
  {
    texts.insert(mod->name);
    modules[mod->name] = &mod->module;
    for(auto& sym : mod->module.symbols) {
      texts.insert(sym.first);
      for(const auto& reg : sym.second->init->regs) {
        insertType(reg->type);
      }
      symbols.push_back(sym.second);
    }
    for(auto& fn : mod->module.functions) {
      insertType(fn->type);
      for(auto& reg : fn->regs) {
        insertType(reg->type);
      }
      functions[fn];
    }
    for(auto& sub : mod->submodules) {
      findModules(sub.second);
    }
  }
  void SemanticAnalyzer::insertType(const Type& type)
  {
    types.insert(type);
    if(type.isArray()) insertType(*type.array.elem);
    else if(type.isPtr()) insertType(*type.ptr.elem);
    else if(type.isFn()) {
      insertType(*type.fn.retType);
      for(const auto arg : type.fn.args) {
        insertType(*arg);
      }
    }
  }
  BinaryVec SemanticAnalyzer::compileFunction(const Function* fn)
  {
    std::vector<BinaryVec> flows;
    for(const auto& flow : fn->flows) {
      BinaryVec code;
      for(const auto inst : flow->insts) {
        if(instanceof<BinaryInstruction>(inst)) {
          auto bin = dynCast<BinaryInstruction>(inst);
          switch(bin->inst) {
            case BinaryInstructions::Add: code << Add; break;
            case BinaryInstructions::Sub: code << Sub; break;
            case BinaryInstructions::Mul: code << Mul; break;
            case BinaryInstructions::Div: code << Div; break;
            case BinaryInstructions::Mod: code << Mod; break;
            case BinaryInstructions::EQ: code << EQ; break;
            case BinaryInstructions::NEQ: code << NEQ; break;
            case BinaryInstructions::GT: code << GT; break;
            case BinaryInstructions::GE: code << GE; break;
            case BinaryInstructions::LT: code << LT; break;
            case BinaryInstructions::LE: code << LE; break;
            default: assert(false);
          }
          code << static_cast<uint32_t>(indexOf(fn->regs, bin->dist));
          code << static_cast<uint32_t>(indexOf(fn->regs, bin->left));
          code << static_cast<uint32_t>(indexOf(fn->regs, bin->right));
        }
        else if(instanceof<UnaryInstruction>(inst)) {
          auto uni = dynCast<UnaryInstruction>(inst);
          switch(uni->inst) {
            case UnaryInstructions::Inc: code << Inc; break;
            case UnaryInstructions::Dec: code << Dec; break;
            case UnaryInstructions::Pos: code << Pos; break;
            case UnaryInstructions::Neg: code << Neg; break;
            default: assert(false);
          }
          code << static_cast<uint32_t>(indexOf(fn->regs, uni->dist));
          code << static_cast<uint32_t>(indexOf(fn->regs, uni->reg));
        }
        else if(instanceof<ImmMove>(inst)) {
          auto imm = dynCast<ImmMove>(inst);
          code << Imm;
          code << static_cast<uint32_t>(indexOf(fn->regs, imm->dist));
          switch(imm->dist->type.type) {
            case Types::I8: code << imm->imm.i8; break;
            case Types::I16: code << imm->imm.i16; break;
            case Types::I32: code << imm->imm.i32; break;
            case Types::I64: code << imm->imm.i64; break;
            default: assert(false);
          }
        }
        else if(instanceof<CallInstruction>(inst)) {
          auto call = dynCast<CallInstruction>(inst);
          code << Call;
          code << static_cast<uint32_t>(indexOf(fn->regs, call->dist));
          code << static_cast<uint32_t>(indexOf(fn->regs, call->fn));
          for(auto& arg : call->args) {
            code << static_cast<uint32_t>(indexOf(fn->regs, arg));
          }
        }
        else if(instanceof<LoadFnInstruction>(inst)) {
          auto loadFn = dynCast<LoadFnInstruction>(inst);
          code << LoadFn;
          code << static_cast<uint32_t>(indexOf(fn->regs, loadFn->reg));
          code << static_cast<uint32_t>(indexOf(functions, loadFn->fn));
        }
        else if(instanceof<LoadArgInstruction>(inst)) {
          auto loadArg = dynCast<LoadArgInstruction>(inst);
          code << LoadArg;
          code << static_cast<uint32_t>(indexOf(fn->regs, loadArg->reg));
          code << loadArg->indexOfArg;
        }
        else if(instanceof<LoadSymbolInstruction>(inst)) {
          auto loadSymbol = dynCast<LoadSymbolInstruction>(inst);
          code << LoadSymbol;
          code << static_cast<uint32_t>(indexOf(fn->regs, loadSymbol->reg));
          code << static_cast<uint32_t>(indexOf(texts, loadSymbol->name));
        }
        else if(instanceof<MovInstruction>(inst)) {
          auto mov = dynCast<MovInstruction>(inst);
          code << Mov;
          code << static_cast<uint32_t>(indexOf(fn->regs, mov->dist));
          code << static_cast<uint32_t>(indexOf(fn->regs, mov->src));
        }
        else if(instanceof<PhiInstruction>(inst)) {
          auto phi = dynCast<PhiInstruction>(inst);
          code << Phi;
          code << static_cast<uint32_t>(indexOf(fn->regs, phi->dist));
          code << static_cast<uint32_t>(indexOf(fn->regs, phi->r1));
          code << static_cast<uint32_t>(indexOf(fn->regs, phi->r2));
        }
        else {
          assert(false);
        }
      }

      BinaryVec flowVec;
      uint32_t typeFlag = 0;
      switch(flow->type) {
        case FlowType::Normal:
          typeFlag |= FLOW_TYPE_NORMAL;
          break;
        case FlowType::ConditionalBranch:
          typeFlag |= FLOW_TYPE_COND_BRANCH;
          break;
        case FlowType::EndPoint:
          typeFlag |= FLOW_TYPE_END_POINT;
          break;
        default:
          assert(false);
      }
      flowVec << typeFlag;
      if(includes(fn->flows, flow->parentFlow)) {
        flowVec << static_cast<uint32_t>(indexOf(fn->flows, flow->parentFlow));
      }
      else {
        flowVec << static_cast<uint32_t>(-1);
      }
      if(typeFlag & FLOW_TYPE_NORMAL) {
        flowVec << static_cast<uint32_t>(indexOf(fn->flows, flow->nextFlow));
      }
      else if(typeFlag & FLOW_TYPE_COND_BRANCH) {
        flowVec << static_cast<uint32_t>(indexOf(fn->regs, flow->cond));
        flowVec << static_cast<uint32_t>(indexOf(fn->flows, flow->thenFlow));
        flowVec << static_cast<uint32_t>(indexOf(fn->flows, flow->elseFlow));
      }
      else if(typeFlag & FLOW_TYPE_END_POINT) {
        if(flow->retReg != nullptr) {
          flowVec << static_cast<uint32_t>(indexOf(fn->regs, flow->retReg));
        }
        else {
          flowVec << static_cast<uint32_t>(-1);
        }
      }
      flowVec << static_cast<uint32_t>(code.size());
      flowVec << code;
      flows.push_back(flowVec);
    }

    BinaryVec result;
    result << static_cast<uint32_t>(indexOf(types, fn->type));
    result << static_cast<uint32_t>(fn->regs.size());
    for(const auto& reg : fn->regs) {
      result << static_cast<uint32_t>(indexOf(types, reg->type));
    }
    result << static_cast<uint32_t>(flows.size());
    result << static_cast<uint32_t>(0);
    for(const auto& flow : flows) {
      result << flow;
    }
    return result;
  }
  Option<std::vector<std::string>> SemanticAnalyzer::write(const CompilerOption& option)
  {
    if(auto err = declare(rootTree)) return some(err.get());
    if(auto err = analyze(rootTree)) return some(err.get());

    findModules(rootTree);
    for(auto& fn : functions) fn.second = compileFunction(fn.first);

    BinaryVec pcir;
    pcir << "PCIR";
    pcir << static_cast<uint64_t>(std::time(nullptr));
    pcir << MAJOR_VERSION;
    pcir << MINOR_VERSION;
    
    BinaryVec textSection;
    textSection << static_cast<uint32_t>(texts.size());
    for(auto& text : texts) {
      textSection << static_cast<uint32_t>(text.size()) << text;
    }

    BinaryVec moduleSection;
    moduleSection << static_cast<uint32_t>(modules.size());
    for(const auto& mod : modules) {
      moduleSection << static_cast<uint32_t>(indexOf(texts, mod.first));
      moduleSection << static_cast<uint32_t>(mod.second->symbols.size());
      for(const auto& sym : mod.second->symbols) {
        moduleSection << static_cast<uint32_t>(indexOf(symbols, sym.second));
      }
    }

    BinaryVec typeSection;
    typeSection << static_cast<uint32_t>(types.size());
    for(const auto& type : types) {
      if(type.isSignedInt() || type.type == Types::Integer) typeSection << TYPE_SIGNED_INTEGER << type.size();
      else if(type.isUnsignedInt()) typeSection << TYPE_UNSIGNED_INTEGER << type.size();
      else if(type.isFloat()) typeSection << TYPE_FLOAT << type.size();
      else if(type.isVoid()) typeSection << TYPE_VOID << type.size();
      else if(type.isBool()) typeSection << TYPE_BOOL << type.size();
      else if(type.isChar()) typeSection << TYPE_CHAR << type.size();
      else if(type.isArray())
        typeSection << TYPE_ARRAY << type.size()
                    << static_cast<uint32_t>(indexOf(types, *type.array.elem))
                    << type.array.length;
      else if(type.isPtr())
        typeSection << TYPE_PTR << type.size()
                    << static_cast<uint32_t>(indexOf(types, *type.ptr.elem));
      else if(type.isFn()) {
        typeSection << TYPE_FUNCTION << type.size()
                    << static_cast<uint32_t>(indexOf(types, *type.fn.retType))
                    << static_cast<uint32_t>(type.fn.args.size());
        for(const auto& arg : type.fn.args) {
          typeSection << static_cast<uint32_t>(indexOf(types, *arg));
        }
      }
      else {
        assert(false);
      }
    }

    BinaryVec symbolSection;
    symbolSection << static_cast<uint32_t>(symbols.size());
    for(const auto& sym : symbols) {
      symbolSection << static_cast<uint32_t>(indexOf(texts, sym->name));
      uint32_t access = 0;
      if(sym->scope == Scope::Public) access |= ACCESS_PUBLIC;
      if(sym->mut == Mutability::Mutable) access |= ACCESS_MUTABLE;
      symbolSection << access;
      symbolSection << static_cast<uint32_t>(indexOf(types, sym->init->result->type));
      symbolSection << static_cast<uint32_t>(indexOf(functions, sym->init));
    }

    BinaryVec fnSection;
    fnSection << static_cast<uint32_t>(functions.size());
    for(const auto& fn : functions) {
      fnSection << fn.second;
    }

    uint32_t ptrToText = 36;
    uint32_t ptrToModule = ptrToText + static_cast<uint32_t>(textSection.size());
    uint32_t ptrToType = ptrToModule + static_cast<uint32_t>(moduleSection.size());
    uint32_t ptrToSym = ptrToType + static_cast<uint32_t>(typeSection.size());
    uint32_t ptrToFn = ptrToSym + static_cast<uint32_t>(symbolSection.size());

    pcir << ptrToText << ptrToModule << ptrToType << ptrToSym << ptrToFn;
    pcir << textSection;
    pcir << moduleSection;
    pcir << typeSection;
    pcir << symbolSection;
    pcir << fnSection;

    auto path = std::filesystem::path(option.outDir) / (option.out + ".pcir");
    std::filesystem::create_directories(option.outDir);
    std::ofstream stream(path, std::ios::binary);
    if(!stream) return some(std::vector{ "ファイル " + path.string() + " が開けません。" });
    stream.write((char*)pcir.data(), pcir.size());
    stream.close();

    if(option.compilerDebug) {
      dump(path.string());
    }

    return none;
  }
}