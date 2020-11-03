#define _CRT_SECURE_NO_WARNINGS

#include "semantic_analyzer.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <ctime>

#include "pickc/config.h"
#include "utils/vector_utils.h"
#include "utils/binary_vec.h"
#include "utils/instanceof.h"
#include "module_analyzer.h"
#include "pcir_format.h"
#include "pcir_struct.h"

namespace pickc::pcir
{
  SemanticAnalyzer::SemanticAnalyzer(ModuleTree* rootTree) : rootTree(rootTree) {}
  Option<std::vector<std::string>> SemanticAnalyzer::declare(ModuleTree* tree)
  {
    std::vector<std::string> errors;
    if(auto errs = ModuleAnalyzer(tree).declare()) errors += errs.get();
    for(auto sub : tree->submodules) if(auto errs = declare(sub.second)) errors += errs.get();
    if(errors.empty()) return none;
    return some(errors);
  }
  Option<std::vector<std::string>> SemanticAnalyzer::analyze(ModuleTree* tree)
  {
    std::vector<std::string> errors;
    if(auto errs = ModuleAnalyzer(tree).analyze()) errors += errs.get();
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
          auto bin = dynamic_cast<BinaryInstruction*>(inst);
          switch(bin->inst) {
            case BinaryInstructions::Add: code << Add; break;
            case BinaryInstructions::Sub: code << Sub; break;
            case BinaryInstructions::Mul: code << Mul; break;
            case BinaryInstructions::Div: code << Div; break;
            case BinaryInstructions::Mod: code << Mod; break;
            default: assert(false);
          }
          code << static_cast<uint32_t>(std::distance(fn->regs.begin(), std::find(fn->regs.begin(), fn->regs.end(), bin->dist)));
          code << static_cast<uint32_t>(std::distance(fn->regs.begin(), std::find(fn->regs.begin(), fn->regs.end(), bin->left)));
          code << static_cast<uint32_t>(std::distance(fn->regs.begin(), std::find(fn->regs.begin(), fn->regs.end(), bin->right)));
        }
        else if(instanceof<UnaryInstruction>(inst)) {
          auto uni = dynamic_cast<UnaryInstruction*>(inst);
          switch(uni->inst) {
            case UnaryInstructions::Inc: code << Inc; break;
            case UnaryInstructions::Dec: code << Dec; break;
            case UnaryInstructions::Pos: code << Pos; break;
            case UnaryInstructions::Neg: code << Neg; break;
            default: assert(false);
          }
          code << static_cast<uint32_t>(std::distance(fn->regs.begin(), std::find(fn->regs.begin(), fn->regs.end(), uni->dist)));
          code << static_cast<uint32_t>(std::distance(fn->regs.begin(), std::find(fn->regs.begin(), fn->regs.end(), uni->reg)));
        }
        else if(instanceof<ImmMove>(inst)) {
          auto imm = dynamic_cast<const ImmMove*>(inst);
          code << Imm;
          code << static_cast<uint32_t>(std::distance(fn->regs.begin(), std::find(fn->regs.begin(), fn->regs.end(), imm->dist)));
          switch(imm->dist->type.type) {
            case Types::I8: code << imm->imm.i8; break;
            case Types::I16: code << imm->imm.i16; break;
            case Types::I32: code << imm->imm.i32; break;
            case Types::I64: code << imm->imm.i64; break;
            default: assert(false);
          }
        }
        else if(instanceof<CallInstruction>(inst)) {
          auto call = dynamic_cast<const CallInstruction*>(inst);
          code << Call;
          code << static_cast<uint32_t>(std::distance(fn->regs.begin(), std::find(fn->regs.begin(), fn->regs.end(), call->dist)));
          code << static_cast<uint32_t>(std::distance(fn->regs.begin(), std::find(fn->regs.begin(), fn->regs.end(), call->fn)));
          for(auto& arg : call->args) {
            code << static_cast<uint32_t>(std::distance(fn->regs.begin(), std::find(fn->regs.begin(), fn->regs.end(), arg)));
          }
        }
        else if(instanceof<ReturnInstruction>(inst)) {
          auto ret = dynamic_cast<const ReturnInstruction*>(inst);
          if(ret->reg) {
            code << Ret;
            code << static_cast<uint32_t>(std::distance(fn->regs.begin(), std::find(fn->regs.begin(), fn->regs.end(), ret->reg)));
          }
          else {
            code << RetV;
          }
        }
        else if(instanceof<LoadFnInstruction>(inst)) {
          auto loadFn = dynamic_cast<const LoadFnInstruction*>(inst);
          code << LoadFn;
          code << static_cast<uint32_t>(std::distance(fn->regs.begin(), std::find(fn->regs.begin(), fn->regs.end(), loadFn->reg)));
          code << static_cast<uint32_t>(std::distance(functions.begin(), functions.find(loadFn->fn)));
        }
      }
      BinaryVec normal;
      normal << FLOW_TYPE_NORMAL;
      if(flow->parentFlow) {
        normal << static_cast<uint32_t>(std::distance(fn->flows.begin(), std::find(fn->flows.begin(), fn->flows.end(), flow->parentFlow)));
      }
      else {
        normal << static_cast<uint32_t>(-1);
      }
      if(flow->nextFlow) {
        normal << static_cast<uint32_t>(std::distance(fn->flows.begin(), std::find(fn->flows.begin(), fn->flows.end(), flow->nextFlow)));
      }
      else {
        normal << static_cast<uint32_t>(-1);
      }
      normal << static_cast<uint32_t>(code.size());
      normal << code;
      flows.push_back(std::move(normal));
    }

    BinaryVec result;
    result << static_cast<uint32_t>(std::distance(types.begin(), types.find(fn->type)));
    result << static_cast<uint32_t>(fn->regs.size());
    for(const auto& reg : fn->regs) {
      result << static_cast<uint32_t>(std::distance(types.begin(), types.find(reg->type)));
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
      moduleSection << static_cast<uint32_t>(std::distance(texts.begin(), texts.find(mod.first)));
      moduleSection << static_cast<uint32_t>(mod.second->symbols.size());
      for(const auto& sym : mod.second->symbols) {
        moduleSection << static_cast<uint32_t>(std::distance(symbols.begin(), std::find(symbols.begin(), symbols.end(), sym.second)));
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
                    << static_cast<uint32_t>(std::distance(types.begin(), types.find(*type.array.elem)))
                    << type.array.length;
      else if(type.isPtr())
        typeSection << TYPE_PTR << type.size()
                    << static_cast<uint32_t>(std::distance(types.begin(), types.find(*type.ptr.elem)));
      else if(type.isFn()) {
        typeSection << TYPE_FUNCTION << type.size();
        typeSection << static_cast<uint32_t>(std::distance(types.begin(), types.find(*type.fn.retType)));
        typeSection << static_cast<uint32_t>(type.fn.args.size());
        for(const auto& arg : type.fn.args) {
          typeSection << static_cast<uint32_t>(std::distance(types.begin(), types.find(*arg)));
        }
      }
      else {
        assert(false);
      }
    }

    BinaryVec symbolSection;
    symbolSection << static_cast<uint32_t>(symbols.size());
    for(const auto& sym : symbols) {
      symbolSection << static_cast<uint32_t>(std::distance(texts.begin(), texts.find(sym->name)));
      uint32_t access = 0;
      if(sym->scope == Scope::Public) access |= ACCESS_PUBLIC;
      if(sym->mut == Mutability::Mutable) access |= ACCESS_MUTABLE;
      symbolSection << access;
      symbolSection << static_cast<uint32_t>(std::distance(types.begin(), types.find(sym->init->result->type)));
      symbolSection << static_cast<uint32_t>(std::distance(functions.begin(), functions.find(sym->init)));
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
      dumpPCIR(path.string());
    }

    return none;
  }
  void SemanticAnalyzer::dumpPCIR(const std::string& path)
  {
    auto res = PCIRLoader().load(path);
    if(!res) return;
    auto pcir = res.get();

    std::cout << "PCIR Dump" << std::endl;
    std::cout << "Dump of file " << path << std::endl;

    std::cout << "PCIR FILE HEADER" << std::endl;
    std::cout << "Magic:                            ";
    std::cout.write(pcir.magic, 4);
    std::cout << std::endl;
    std::cout << "Time stamp:                       " << std::hex << std::setw(16) << std::setfill('0')
              << pcir.timeStamp << " "
              << std::asctime(std::localtime((std::time_t*)&pcir.timeStamp));
    std::cout << "Major version:                    " << pcir.majorVersion << std::endl;
    std::cout << "Minor version:                    " << pcir.minorVersion << std::endl;
    std::cout << "Pointer to text header:           " << std::hex << std::setw(8) << std::setfill('0') << pcir.ptrToTextHeader << std::endl;
    std::cout << "Pointer to module header:         " << std::hex << std::setw(8) << std::setfill('0') << pcir.ptrToModuleHeader << std::endl;
    std::cout << "Pointer to type table header:     " << std::hex << std::setw(8) << std::setfill('0') << pcir.ptrToTypeTableHeader << std::endl;
    std::cout << "Pointer to symbol table header:   " << std::hex << std::setw(8) << std::setfill('0') << pcir.ptrToSymbolTableHeader << std::endl;
    std::cout << "Pointer to function table header: " << std::hex << std::setw(8) << std::setfill('0') << pcir.ptrToFunctionTableHeader << std::endl;
    std::cout << '\n';

    std::cout << "PCIR TEXT SECTION" << std::endl;
    std::cout << "Number of texts:                  " << std::hex << std::setw(8) << std::setfill('0') << pcir.textSection.size() << std::endl;
    for(size_t i = 0, l = pcir.textSection.size(); i < l; ++i) {
      std::string outStr;
      outStr += "Text #";
      outStr += std::to_string(i);
      outStr += ":";
      outStr.append(34 - outStr.size(), ' ');
      std::cout << outStr << pcir.textSection[i]->text << std::endl;
    }
    std::cout << '\n';

    std::cout << "PCIR MODULE SECTION" << std::endl;
    std::cout << "Number of modules:                " << std::hex << std::setw(8) << std::setfill('0') << pcir.moduleSection.size() << std::endl;
    for(size_t i = 0, l = pcir.moduleSection.size(); i < l; ++i) {
      auto module = pcir.moduleSection[i];
      std::cout << "Module #" << i << std::endl;
      std::cout << "    Name:                         " << module->name->text << std::endl;
      std::cout << "    Symbols:" << std::endl;
      for(size_t j = 0, jl = module->symbols.size(); j < jl; ++j) {
        std::cout << "        Symbol #" << std::distance(pcir.symbolSection.begin(), std::find(pcir.symbolSection.begin(), pcir.symbolSection.end(), module->symbols[j])) << std::endl;
      }
    }
    std::cout << '\n';

    std::cout << "PCIR TYPE SECTION" << std::endl;
    std::cout << "Number of types:                  " << std::hex << std::setw(8) << std::setfill('0') << pcir.typeSection.size() << std::endl;
    for(size_t i = 0, l = pcir.typeSection.size(); i < l; ++i) {
      std::cout << "Type #" << i << std::endl;
      std::cout << "    Name:                         " << pcir.typeSection[i]->type.toString() << std::endl;
    }
    std::cout << '\n';

    std::cout << "PCIR SYMBOL SECTION" << std::endl;
    std::cout << "Number of symbols:                " << std::hex << std::setw(8) << std::setfill('0') << pcir.symbolSection.size() << std::endl;
    for(size_t i = 0, l = pcir.symbolSection.size(); i < l; ++i) {
      auto symbol = pcir.symbolSection[i];
      std::cout << "Symbol #" << i << std::endl;
      std::cout << "    Name:                         " << symbol->name->text << std::endl;
      std::cout << "    Scope:                        " << (symbol->scope == Scope::Public ? "public" : "private") << std::endl;
      std::cout << "    Mutability:                   " << (symbol->mut == Mutability::Mutable ? "mutable" : "immutable") << std::endl;
      std::cout << "    Type:                         " << symbol->type->type.toString() << std::endl;
      std::cout << "    Init:                         Function #" << std::distance(pcir.fnSection.begin(), std::find(pcir.fnSection.begin(), pcir.fnSection.end(), symbol->init)) << std::endl;
    }
    std::cout << '\n';

    std::cout << "PCIR FUNCTION SECTION" << std::endl;
    std::cout << "Number of functions:              " << std::hex << std::setw(8) << std::setfill('0') << pcir.fnSection.size() << std::endl;
    for(size_t i = 0, l = pcir.fnSection.size(); i < l; ++i) {
      auto fn = pcir.fnSection[i];
      std::cout << "Function #" << i << std::endl;
      std::cout << "    Type:                         " << fn->type->type.toString() << std::endl;
      std::cout << "    Entry Flow:                   Flow #" << std::distance(fn->flows.begin(), std::find(fn->flows.begin(), fn->flows.end(), fn->entryFlow)) << std::endl;
      std::cout << "    Registers:" << std::endl;
      for(size_t j = 0, jl = fn->regs.size(); j < jl; ++j) {
        std::cout << "        Register #" << j << std::endl;
        std::cout << "            Type:                 " << fn->regs[j]->type->type.toString() << std::endl;
      }
      std::cout << "    Flows:" << std::endl;
      for(size_t j = 0, jl = fn->flows.size(); j < jl; ++j) {
        auto flow = fn->flows[j];
        std::cout << "        Flow #" << j << std::endl;
        if(flow == fn->entryFlow) {
          std::cout << "            *Entry Flow*" << std::endl;
        }
        else {
          std::cout << "            Parent:               ";
          if(flow->parent == nullptr) std::cout << "none" << std::endl;
          else std::cout << "Flow #" << std::distance(fn->flows.begin(), std::find(fn->flows.begin(), fn->flows.end(), flow->parent)) << std::endl;
        }
        std::cout << "            Next Flow:            ";
        if(flow->normal.next == nullptr) std::cout << "none" << std::endl;
        else std::cout << "Flow #" << std::distance(fn->flows.begin(), std::find(fn->flows.begin(), fn->flows.end(), flow->normal.next)) << std::endl;
        std::cout << "            Flow Type:            ";
        if(flow->flowType == FLOW_TYPE_NORMAL) {
          std::cout << "Normal" << std::endl;
          std::cout << "            Code:" << std::endl;
          for(size_t k = 0, kl = flow->normal.code.size(); k < kl; ++k) {
            const auto binary = [&](const std::string& inst) {
              std::cout << inst << " #" << get32(flow->normal.code, k) << " #" << get32(flow->normal.code, k) << " #" << get32(flow->normal.code, k) << std::endl;
            };
            const auto unary = [&](const std::string& inst) {
              std::cout << inst << " #" << get32(flow->normal.code, k) << " #" << get32(flow->normal.code, k) << std::endl;
            };
            std::cout << "                ";
            switch(flow->normal.code[k]) {
              case Add: binary("ADD"); break;
              case Sub: binary("SUB"); break;
              case Mul: binary("MUL"); break;
              case Div: binary("DIV"); break;
              case Mod: binary("MOD"); break;
              case Inc: unary("INC"); break;
              case Dec: unary("DEC"); break;
              case Pos: unary("POS"); break;
              case Neg: unary("NEG"); break;
              case Imm: {
                auto dist = get32(flow->normal.code, k);
                std::cout << "IMM #" << dist << " ";
                switch(fn->regs[dist]->type->types) {
                  case Types::I8:
                  case Types::U8:
                    std::cout << (int)get8(flow->normal.code, k) << std::endl;
                    break;
                  case Types::I16:
                  case Types::U16:
                    std::cout << get16(flow->normal.code, k) << std::endl;
                    break;
                  case Types::I32:
                  case Types::U32:
                    std::cout << get32(flow->normal.code, k) << std::endl;
                    break;
                  case Types::I64:
                  case Types::U64:
                    std::cout << get64(flow->normal.code, k) << std::endl;
                    break;
                }
                break;
              }
              case Call: {
                std::cout << "CALL #" << get32(flow->normal.code, k) << " #";
                auto call = get32(flow->normal.code, k);
                std::cout << call << " (";
                auto type = fn->regs[call]->type;
                assert(type->types == Types::Function);
                for(size_t arg = 0; arg < type->numOfArgs; ++arg) {
                  std::cout << "#" << get32(flow->normal.code, k);
                  if(arg + 1 < type->numOfArgs) std::cout << " ";
                }
                std::cout << ")" << std::endl;
                break;
              }
              case Ret: std::cout << "RET #" << get32(flow->normal.code, k) << std::endl; break;
              case RetV: std::cout << "RETV" << std::endl; break;
              case LoadFn: std::cout << "LOADFN Register #" << get32(flow->normal.code, k) << " Function #" << get32(flow->normal.code, k) << std::endl; break;
              case Mov: std::cout << "MOV #" << get32(flow->normal.code, k) << " #" << get32(flow->normal.code, k) << std::endl; break;
              default:
                std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)flow->normal.code[k] << std::endl;
                break;
            }
          }
        }
        else {
          assert(false);
        }
      }
    }
  }
}