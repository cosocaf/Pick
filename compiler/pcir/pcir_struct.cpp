#include "pcir_struct.h"

#include <fstream>

#include "pcir_format.h"

namespace pickc::pcir
{
  PCIRLoader::PCIRLoader() {}
  FlowStruct::FlowStruct() {}
  FlowStruct::~FlowStruct()
  {
    switch(flowType) {
      case FLOW_TYPE_NORMAL:
        normal.~NormalFlowStruct();
        break;
    }
    flowType = static_cast<uint32_t>(-1);
  }
  Result<PCIRFile, std::vector<std::string>> PCIRLoader::load(const std::string& path)
  {
    std::ifstream stream(path, std::ios::binary);
    if(!stream) return error(std::vector{ "ファイル " + path + " が開けません。" });

    stream.read(file.magic, 4);
    if(file.magic[0] != 'P' || file.magic[1] != 'C' || file.magic[2] != 'I' || file.magic[3] != 'R') {
      return error(std::vector{ path + "はPCIRファイルではありません。" });
    }

    stream.read((char*)&file.timeStamp, 8);
    stream.read((char*)&file.majorVersion, 2);
    stream.read((char*)&file.minorVersion, 2);

    stream.read((char*)&file.ptrToTextHeader, 4);
    stream.read((char*)&file.ptrToModuleHeader, 4);
    stream.read((char*)&file.ptrToTypeTableHeader, 4);
    stream.read((char*)&file.ptrToSymbolTableHeader, 4);
    stream.read((char*)&file.ptrToFunctionTableHeader, 4);

    stream.seekg(file.ptrToTextHeader, std::ios::beg);
    uint32_t numOfTexts;
    stream.read((char*)&numOfTexts, 4);
    for(uint32_t i = 0; i < numOfTexts; ++i) {
      uint32_t sizeOfText;
      stream.read((char*)&sizeOfText, 4);
      auto text = new char[sizeOfText];
      stream.read(text, sizeOfText);
      file.textSection.push_back(new TextSection{ std::string(text, sizeOfText) });
      delete text;
    }

    stream.seekg(file.ptrToTypeTableHeader, std::ios::beg);
    uint32_t numOfTypes;
    stream.read((char*)&numOfTypes, 4);
    for(uint32_t i = 0; i < numOfTypes; ++i) {
      uint32_t flag, sizeOfType;
      stream.read((char*)&flag, 4);
      stream.read((char*)&sizeOfType, 4);
      if(flag == TYPE_SIGNED_INTEGER) {
        Types type;
        switch(sizeOfType) {
          case 8: type = Types::I8; break;
          case 16: type = Types::I16; break;
          case 32: type = Types::I32; break;
          case 64: type = Types::I64; break;
          default: return error(std::vector{ path + "は適切なPCIRファイルではありません。整数型は8bit, 16bit, 32bit, 64bitでなければなりません。" });
        }
        file.typeSection.push_back(new TypeSection{ type, Type(type) });
      }
      else if(flag == TYPE_UNSIGNED_INTEGER) {
        Types type;
        switch(sizeOfType) {
          case 8: type = Types::U8; break;
          case 16: type = Types::U16; break;
          case 32: type = Types::U32; break;
          case 64: type = Types::U64; break;
          default: return error(std::vector{ path + "は適切なPCIRファイルではありません。整数型は8bit, 16bit, 32bit, 64bitでなければなりません。" });
        }
        file.typeSection.push_back(new TypeSection{ type, Type(type) });
      }
      else if(flag == TYPE_FLOAT) {
        Types type;
        switch(sizeOfType) {
          case 32: type = Types::F32; break;
          case 64: type = Types::F64; break;
          default: return error(std::vector{ path + "は適切PCIRファイルではありません。浮動小数型は32bitまたは64bitでなければなりません。" });
        }
        file.typeSection.push_back(new TypeSection{ type, Type(type) });
      }
      else if(flag == TYPE_VOID) {
        file.typeSection.push_back(new TypeSection{ Types::Void, Type(Types::Void) });
      }
      else if(flag == TYPE_BOOL) {
        file.typeSection.push_back(new TypeSection{ Types::Bool, Type(Types::Bool) });
      }
      else if(flag == TYPE_CHAR) {
        file.typeSection.push_back(new TypeSection{ Types::Char, Type(Types::Char) });
      }
      else if(flag == TYPE_ARRAY) {
        auto array = new TypeSection{ Types::Array };
        stream.read((char*)&array->indexOfElem, 4);
        stream.read((char*)&array->elemLength, 4);
        file.typeSection.push_back(array);
      }
      else if(flag == TYPE_PTR) {
        auto ptr = new TypeSection{ Types::Ptr };
        stream.read((char*)&ptr->indexOfElem, 4);
        file.typeSection.push_back(ptr);
      }
      else if(flag == TYPE_FUNCTION) {
        auto fn = new TypeSection{ Types::Function };
        stream.read((char*)&fn->indexOfRet, 4);
        stream.read((char*)&fn->numOfArgs, 4);
        for(uint32_t j = 0; j < fn->numOfArgs; ++j) {
          uint32_t indexOfArgType;
          stream.read((char*)&indexOfArgType, 4);
          fn->indexOfArgs.push_back(indexOfArgType);
        }
        file.typeSection.push_back(fn);
      }
      else {
        assert(false);
      }
    }
    for(auto& type : file.typeSection) {
      switch(type->types) {
        case Types::Array: {
          TypeArray array;
          array.elem = &file.typeSection[type->indexOfElem]->type;
          array.length = type->elemLength;
          type->type = Type(array);
          break;
        }
        case Types::Ptr: {
          TypePtr ptr;
          ptr.elem = &file.typeSection[type->indexOfElem]->type;
          type->type = Type(ptr);
          break;
        }
        case Types::Function: {
          TypeFunction fn;
          fn.retType = &file.typeSection[type->indexOfRet]->type;
          for(auto arg : type->indexOfArgs) {
            fn.args.push_back(&file.typeSection[arg]->type);
          }
          type->type = Type(fn);
          break;
        }
      }
    }

    stream.seekg(file.ptrToFunctionTableHeader, std::ios::beg);
    uint32_t numOfFunctions;
    stream.read((char*)&numOfFunctions, 4);
    for(uint32_t i = 0; i < numOfFunctions; ++i) {
      auto fn = new FunctionSection();
      uint32_t type;
      stream.read((char*)&type, 4);
      fn->type = file.typeSection[type];
      uint32_t numOfRegs;
      stream.read((char*)&numOfRegs, 4);
      fn->regs.reserve(numOfRegs);
      for(uint32_t j = 0; j < numOfRegs; ++j) {
        uint32_t reg;
        stream.read((char*)&reg, 4);
        fn->regs.push_back(new RegisterStruct{ file.typeSection[reg] });
      }
      uint32_t numFlows;
      stream.read((char*)&numFlows, 4);
      uint32_t entryFlow;
      stream.read((char*)&entryFlow, 4);
      fn->flows.reserve(numFlows);
      for(uint32_t j = 0; j < numFlows; ++j) {
        fn->flows.push_back(new FlowStruct());
      }
      for(auto& flow : fn->flows) {
        stream.read((char*)&flow->flowType, 4);
        uint32_t indexOfParentFlow;
        stream.read((char*)&indexOfParentFlow, 4);
        if(indexOfParentFlow == -1) {
          flow->parent = nullptr;
        }
        else {
          flow->parent = fn->flows[indexOfParentFlow];
        }
        switch(flow->flowType) {
          case FLOW_TYPE_NORMAL: {
            new (&flow->normal) NormalFlowStruct();
            uint32_t indexOfNextFlow;
            stream.read((char*)&indexOfNextFlow, 4);
            if(indexOfNextFlow == -1) {
              flow->normal.next = nullptr;
            }
            else {
              flow->normal.next = fn->flows[indexOfNextFlow];
            }
            uint32_t sizeOfCodes;
            stream.read((char*)&sizeOfCodes, 4);
            auto code = new uint8_t[sizeOfCodes];
            stream.read((char*)code, sizeOfCodes);
            flow->normal.code.reserve(sizeOfCodes);
            flow->normal.code.insert(flow->normal.code.end(), code, code + sizeOfCodes);
            delete[] code;
            break;
          }
          default:
            return error(std::vector{ path + "は適切PCIRファイルではありません。フロータイプが予期しない値でした。" });
        }
      }
      fn->entryFlow = fn->flows[entryFlow];
      file.fnSection.push_back(fn);
    }

    stream.seekg(file.ptrToSymbolTableHeader, std::ios::beg);
    uint32_t numOfSymbols;
    stream.read((char*)&numOfSymbols, 4);
    for(uint32_t i = 0; i < numOfSymbols; ++i) {
      PCIRSymbol symbol;
      stream.read((char*)&symbol, sizeof(PCIRSymbol));
      file.symbolSection.push_back(new SymbolSection{
        file.textSection[symbol.name],
        (symbol.access & ACCESS_PUBLIC) ? Scope::Public : Scope::Private,
        (symbol.access & ACCESS_MUTABLE) ? Mutability::Mutable : Mutability::Immutable,
        file.typeSection[symbol.type],
        file.fnSection[symbol.init],
      });
    }

    stream.seekg(file.ptrToModuleHeader, std::ios::beg);
    uint32_t numOfModules;
    stream.read((char*)&numOfModules, 4);
    for(uint32_t i = 0; i < numOfModules; ++i) {
      auto module = new ModuleSection();
      uint32_t indexOfName, numOfModuleSymbols;
      stream.read((char*)&indexOfName, 4);
      stream.read((char*)&numOfModuleSymbols, 4);
      module->name = file.textSection[indexOfName];
      for(uint32_t j = 0; j < numOfModuleSymbols; ++j) {
        uint32_t indexOfSymbol;
        stream.read((char*)&indexOfSymbol, 4);
        module->symbols.push_back(file.symbolSection[indexOfSymbol]);
      }
      file.moduleSection.push_back(module);
    }

    return ok(file);
  }
}