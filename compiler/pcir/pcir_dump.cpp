#include "pcir_dump.h"

#include <iostream>
#include <iomanip>

#include "utils/vector_utils.h"
#include "pcir_format.h"
#include "pcir_struct.h"

namespace pickc::pcir
{
  void dump(const std::string& path)
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
        std::cout << "        Symbol #" << indexOf(pcir.symbolSection, module->symbols[j]) << std::endl;
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
      std::cout << "    Init:                         Function #" << indexOf(pcir.fnSection, symbol->init) << std::endl;
    }
    std::cout << '\n';

    std::cout << "PCIR FUNCTION SECTION" << std::endl;
    std::cout << "Number of functions:              " << std::hex << std::setw(8) << std::setfill('0') << pcir.fnSection.size() << std::endl;
    for(size_t i = 0, l = pcir.fnSection.size(); i < l; ++i) {
      auto fn = pcir.fnSection[i];
      std::cout << "Function #" << i << std::endl;
      std::cout << "    Type:                         " << fn->type->type.toString() << std::endl;
      std::cout << "    Entry Flow:                   Flow #" << indexOf(fn->flows, fn->entryFlow) << std::endl;
      std::cout << "    Registers:" << std::endl;
      for(size_t j = 0, jl = fn->regs.size(); j < jl; ++j) {
        std::cout << "        Register #" << j << std::endl;
        std::cout << "            Type:                 " << fn->regs[j]->type->type.toString() << std::endl;
      }
      std::cout << "    Flows:" << std::endl;
      for(size_t j = 0, jl = fn->flows.size(); j < jl; ++j) {
        auto flow = fn->flows[j];
        std::cout << "        Flow #" << j << std::endl;
        std::cout << "            Parent:               ";
        if(flow->parent == nullptr) std::cout << "none" << std::endl;
        else std::cout << "Flow #" << indexOf(fn->flows, flow->parent) << std::endl;
        std::cout << "            Flow Type:            ";
        if(flow->flowType & FLOW_TYPE_NORMAL) {
          std::cout << "NORMAL" << std::endl;
          std::cout << "            Next Flow:            ";
          if(flow->next == nullptr) std::cout << "none" << std::endl;
          else std::cout << "Flow #" << indexOf(fn->flows, flow->next) << std::endl;
        }
        else if(flow->flowType & FLOW_TYPE_COND_BRANCH) {
          std::cout << "COND_BRANCH" << std::endl;
          std::cout << "            Cond:                 Register #" << indexOf(fn->regs, flow->cond) << std::endl;
          std::cout << "            Then Flow:            Flow #" << indexOf(fn->flows, flow->thenFlow) << std::endl;
          std::cout << "            Else Flow:            Flow #" << indexOf(fn->flows, flow->elseFlow) << std::endl;
        }
        else if(flow->flowType & FLOW_TYPE_END_POINT) {
          std::cout << "END_POINT" << std::endl;
          std::cout << "            Return Register       ";
          if(flow->retReg != nullptr) std::cout << "#" << indexOf(fn->regs, flow->retReg) << std::endl;
          else std::cout << "void" << std::endl;
        }

        std::cout << "            Code:" << std::endl;
        for(size_t k = 0, kl = flow->code.size(); k < kl; ++k) {
          const auto binary = [&](const std::string& inst) {
            std::cout << inst << " #" << get32(flow->code, k) << " #" << get32(flow->code, k) << " #" << get32(flow->code, k) << std::endl;
          };
          const auto unary = [&](const std::string& inst) {
            std::cout << inst << " #" << get32(flow->code, k) << " #" << get32(flow->code, k) << std::endl;
          };
          std::cout << "                ";
          switch(flow->code[k]) {
            case Add: binary("ADD"); break;
            case Sub: binary("SUB"); break;
            case Mul: binary("MUL"); break;
            case Div: binary("DIV"); break;
            case Mod: binary("MOD"); break;
            case Inc: unary("INC"); break;
            case Dec: unary("DEC"); break;
            case Pos: unary("POS"); break;
            case Neg: unary("NEG"); break;
            case EQ: binary("EQ"); break;
            case NEQ: binary("NEQ"); break;
            case GT: binary("GT"); break;
            case GE: binary("GE"); break;
            case LT: binary("LT"); break;
            case LE: binary("LE"); break;
            case Imm: {
              auto dist = get32(flow->code, k);
              std::cout << "IMM #" << dist << " ";
              switch(fn->regs[dist]->type->types) {
                case Types::I8:
                case Types::U8:
                  std::cout << (int)get8(flow->code, k) << std::endl;
                  break;
                case Types::I16:
                case Types::U16:
                  std::cout << get16(flow->code, k) << std::endl;
                  break;
                case Types::I32:
                case Types::U32:
                  std::cout << get32(flow->code, k) << std::endl;
                  break;
                case Types::I64:
                case Types::U64:
                  std::cout << get64(flow->code, k) << std::endl;
                  break;
              }
              break;
            }
            case Call: {
              std::cout << "CALL #" << get32(flow->code, k) << " #";
              auto call = get32(flow->code, k);
              std::cout << call << " (";
              auto type = fn->regs[call]->type;
              assert(type->types == Types::Function);
              for(size_t arg = 0; arg < type->numOfArgs; ++arg) {
                std::cout << "#" << get32(flow->code, k);
                if(arg + 1 < type->numOfArgs) std::cout << " ";
              }
              std::cout << ")" << std::endl;
              break;
            }
            case LoadFn: std::cout << "LOADFN Register #" << get32(flow->code, k) << " Function #" << get32(flow->code, k) << std::endl; break;
            case LoadArg: std::cout << "LOADARG Register #" << get32(flow->code, k) << " Arg #" << get32(flow->code, k) << std::endl; break;
            case LoadSymbol: std::cout << "LOADSYMBOL Register #" << get32(flow->code, k) << " Symbol " << pcir.textSection[get32(flow->code, k)]->text << std::endl; break;
            case Mov: std::cout << "MOV #" << get32(flow->code, k) << " #" << get32(flow->code, k) << std::endl; break;
            case Phi: std::cout << "Phi #" << get32(flow->code, k) << " #" << get32(flow->code, k) << " #" << get32(flow->code, k) << std::endl; break;
            default:
              assert(false);
              std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)flow->code[k] << std::endl;
              break;
          }
        }
      }
    }
  }
}