/**
 * @file pcir_decoder.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief pcir_decoder.hの実装
 * @version 0.1
 * @date 2021-09-06
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "pcir_decoder.h"

#include <cassert>
#include <variant>

#include "utils/fstream_utils.h"
#include "utils/binary_vec.h"

namespace pickc::bundler {
  PCIRDecoder::PCIRDecoder(const std::string& filename) : filename(filename) {}
  std::optional<PCIR> PCIRDecoder::decode() {
    stream.open(filename, std::ios::binary);
    if(!stream) {
      return std::nullopt;
    }

    stream.seekg(0, std::ios::beg);
    loadHeader(pcir.header);

    stream.seekg(pcir.header.ptrTextSection, std::ios::beg);
    loadTextSection(pcir.textSection);

    stream.seekg(pcir.header.ptrModuleSection, std::ios::beg);
    loadModuleSection(pcir.moduleSection);

    stream.seekg(pcir.header.ptrTypeSection, std::ios::beg);
    loadTypeSection(pcir.typeSection);

    stream.seekg(pcir.header.ptrSymbolSection, std::ios::beg);
    loadSymbolSection(pcir.symbolSection);

    stream.seekg(pcir.header.ptrFnSection, std::ios::beg);
    loadFunctionSection(pcir.functionSection);

    return pcir;

    // // TODO: C++20以降はstd::to_arrayで置き換え可能
    // for(int i = 0; i < 16; ++i) {
    //   package->fileID[i] = fileHeader.fileID[i];
    // }
    // package->texts = std::move(textSection.texts);
    // package->modules.resize(moduleSection.numModules);
    // for(auto& module : package->modules) {
    //   module = std::make_shared<_Module>();
    // }
    // package->types.resize(typeSection.numTypes);
    // for(auto& type : package->types) {
    //   type = std::make_shared<_Type>();
    // }
    // package->symbols.resize(symbolSection.numSymbols);
    // for(auto& symbol : package->symbols) {
    //   symbol = std::make_shared<_Symbol>();
    // }
    // package->functions.resize(fnSection.numFunctions);
    // for(auto& function : package->functions) {
    //   function = std::make_shared<_Function>();
    // }

    // for(size_t i = 0; i < moduleSection.numModules; ++i) {
    //   auto& pcirModule = moduleSection.modules[i];
    //   auto& module = package->modules[i];
    //   module->name = package->texts[pcirModule.moduleName];
    //   module->parentName = package->texts[pcirModule.parentName];
    //   module->accessibility = static_cast<Accessibility>(pcirModule.accessibility);
    //   module->types.resize(pcirModule.numTypes);
    //   for(size_t j = 0; j < pcirModule.numTypes; ++j) {
    //     auto& pcirType = pcirModule.types[j];
    //     auto& type = module->types[j];
    //     type.type = getType(pcirType.typeIndex);
    //     type.accessibility = static_cast<Accessibility>(pcirType.accessibility);
    //   }
    //   module->symbols.resize(pcirModule.numSymbols);
    //   for(size_t j = 0; j < pcirModule.numSymbols; ++j) {
    //     auto& pcirSymbol = pcirModule.symbols[j];
    //     auto& symbol = module->symbols[j];
    //     symbol.symbol = package->symbols[pcirSymbol.symbolIndex];
    //     symbol.accessibility = static_cast<Accessibility>(pcirSymbol.accessibility);
    //   }
    //   module->functions.resize(pcirModule.numFunctions);
    //   for(size_t j = 0; j < pcirModule.numFunctions; ++j) {
    //     auto& pcirFunction = pcirModule.functions[j];
    //     auto& function = module->functions[j];
    //     function.function = package->functions[pcirFunction.functionIndex];
    //   }
    // }

    // for(size_t i = 0; i < typeSection.numTypes; ++i) {
    //   auto& pcirType = typeSection.types[i];
    //   auto& type = package->types[i];
    //   switch(pcirType.kind) {
    //     // Array
    //     case 0x0001: {
    //       auto& pcirArray = std::get<PCIRTypeArray>(pcirType.variant);
    //       TypeArray array;
    //       array.elementType = getType(pcirArray.elemType);
    //       array.arrayLength = pcirArray.arraySize;
    //       type->variant = array;
    //       break;
    //     }
    //     // Tuple
    //     case 0x0002: {
    //       auto& pcirTuple = std::get<PCIRTypeTuple>(pcirType.variant);
    //       TypeTuple tuple;
    //       tuple.elementTypes.resize(pcirTuple.numElems);
    //       for(size_t j = 0; j < pcirTuple.numElems; ++j) {
    //         tuple.elementTypes[j] = getType(pcirTuple.elements[j]);
    //       }
    //       type->variant = tuple;
    //       break;
    //     }
    //     // Pointer
    //     case 0x0003: {
    //       auto& pcirPointer = std::get<PCIRTypePointer>(pcirType.variant);
    //       TypePointer pointer;
    //       pointer.entityType = getType(pcirPointer.entityType);
    //       type->variant = pointer;
    //       break;
    //     }
    //     // Function
    //     case 0x0004: {
    //       auto& pcirFunction = std::get<PCIRTypeFunction>(pcirType.variant);
    //       TypeFunction function;
    //       function.returnType = getType(pcirFunction.returnType);
    //       function.argumentTypes.resize(pcirFunction.numArgs);
    //       for(size_t j = 0; j < pcirFunction.numArgs; ++j) {
    //         function.argumentTypes[j] = getType(pcirFunction.args[j]);
    //       }
    //       type->variant = function;
    //       break;
    //     }
    //     // External
    //     case 0x0007: {
    //       auto& pcirExternal = std::get<PCIRTypeExternal>(pcirType.variant);
    //       TypeExternal external;
    //       external.typeName = package->texts[pcirExternal.name];
    //       external.moduleName = package->texts[pcirExternal.moduleName];
    //       for(int j = 0; j < 16; ++j) {
    //         external.fileID[j] = pcirExternal.fileID[j];
    //       }
    //       type->variant = external;
    //       break;
    //     }
    //     default:
    //       // TODO: return error.
    //       assert(false);
    //   }
    // }

    // for(size_t i = 0; i < symbolSection.numSymbols; ++i) {
    //   auto& pcirSymbol = symbolSection.symbols[i];
    //   auto& symbol = package->symbols[i];
    //   symbol->name = package->texts[pcirSymbol.name];
    //   switch(pcirSymbol.kind) {
    //     // Internal
    //     case 0x0001: {
    //       auto& pcirInternal = std::get<PCIRInternalSymbol>(pcirSymbol.variant);
    //       InternalSymbol internal;
    //       internal.type = getType(pcirInternal.type);
    //       internal.mutability = static_cast<Mutability>(pcirInternal.mutability);
    //       internal.initFunction = package->functions[pcirInternal.initFn];
    //       symbol->variant = internal;
    //       break;
    //     }
    //     // External
    //     case 0x0002: {
    //       auto& pcirExternal = std::get<PCIRExternalSymbol>(pcirSymbol.variant);
    //       ExternalSymbol external;
    //       external.moduleName = package->texts[pcirExternal.moduleName];
    //       for(int j = 0; j < 16; ++j) {
    //         external.fileID[j] = pcirExternal.fileID[j];
    //       }
    //       symbol->variant = external;
    //       break;
    //     }
    //     default:
    //       // TODO: return error.
    //       assert(false);
    //   }
    // }

    // for(size_t i = 0; i < fnSection.numFunctions; ++i) {
    //   auto& pcirFn = fnSection.functions[i];
    //   auto& fn = package->functions[i];
    //   fn->type = getType(pcirFn.type);
    //   switch(pcirFn.kind) {
    //     // Internal
    //     case 0x0001: {
    //       auto& pcirInternal = std::get<PCIRInternalFunction>(pcirFn.variant);
    //       InternalFunction internal;
    //       internal.variables.resize(pcirInternal.numVars);
    //       for(size_t j = 0; j < pcirInternal.numVars; ++j) {
    //         internal.variables[j].type = getType(pcirInternal.vars[j].type);
    //       }
    //       internal.blocks.resize(pcirInternal.numBlocks);
    //       for(size_t j = 0; j < pcirInternal.numBlocks; ++j) {
    //         auto& pcirBlock = pcirInternal.blocks[j];
    //         Block block;
    //         switch(pcirBlock.kind) {
    //           // NonTerminalBlock
    //           case 0x0001: {
    //             auto& pcirNonTerminal = std::get<PCIRNonTerminalBlock>(pcirBlock.variant);
    //             NonTerminalBlock nonTerminal;
    //             nonTerminal.nextBlock = pcirNonTerminal.nextBlock;
    //             block.variant = nonTerminal;
    //             break;
    //           }
    //           // TerminalBlock
    //           case 0x0002: {
    //             auto& pcirTerminal = std::get<PCIRTerminalBlock>(pcirBlock.variant);
    //             TerminalBlock terminal;
    //             terminal.returnValue = pcirTerminal.returnValue;
    //             block.variant = terminal;
    //             break;
    //           }
    //           // ConditionalBranchBlock
    //           case 0x0003: {
    //             auto& pcirCond = std::get<PCIRConditionalBranchBlock>(pcirBlock.variant);
    //             ConditionalBranchBlock cond;
    //             cond.conditionalValue = pcirCond.conditionalValue;
    //             cond.thenBlock = pcirCond.thenBlock;
    //             cond.elseBlock = pcirCond.elseBlock;
    //             block.variant = cond;
    //             break;
    //           }
    //           default:
    //             // TODO: return error.
    //             assert(false);
    //         }
    //       }
    //       fn->variant = internal;
    //       break;
    //     }
    //     // External
    //     case 0x0002: {
    //       auto& pcirExternal = std::get<PCIRExternalFunction>(pcirFn.variant);
    //       ExternalFunction external;
    //       external.name = package->texts[pcirExternal.name];
    //       fn->variant = external;
    //       break;
    //     }
    //     default:
    //       // TODO: return error.
    //       assert(false);
    //   }
    // }

    // return package;
  }

  void PCIRDecoder::loadHeader(PCIRFileHeader& header) {
    assert(stream.is_open());
    stream >> header.signature
           >> header.majorVersion
           >> header.minorVersion
           >> header.timestamp
           >> header.fileID
           >> header.ptrTextSection
           >> header.ptrModuleSection
           >> header.ptrTypeSection
           >> header.ptrSymbolSection
           >> header.ptrFnSection;
    if(strncmp(header.signature, "PCIR", 4) != 0) {
      // TODO: return error.
      assert(false);
    }
  }
  void PCIRDecoder::loadTextSection(PCIRTextSection& section) {
    assert(stream.is_open());
    stream >> section.sizeOfTextSection
           >> section.numTexts;
    section.texts.resize(section.numTexts);
    for(auto& text : section.texts) {
      uint32_t sizeOfText;
      stream >> sizeOfText;
      text.resize(sizeOfText);
      stream.read(text.data(), sizeOfText);
    }
  }
  void PCIRDecoder::loadModuleSection(PCIRModuleSection& section) {
    assert(stream.is_open());
    stream >> section.sizeOfModuleSection
           >> section.numModules;
    section.modules.resize(section.numModules);
    for(auto& module : section.modules) {
      stream >> module.moduleName
             >> module.parentName
             >> module.accessibility
             >> module.numTypes
             >> module.numSymbols
             >> module.numFunctions;
      module.types.resize(module.numTypes);
      module.symbols.resize(module.numSymbols);
      module.functions.resize(module.numFunctions);
      for(auto& type : module.types) {
        stream >> type.typeIndex
               >> type.accessibility;
      }
      for(auto& symbol : module.symbols) {
        stream >> symbol.symbolIndex
               >> symbol.accessibility;
      }
      for(auto& fn : module.functions) {
        stream >> fn.functionIndex;
      }
    }
  }
  void PCIRDecoder::loadTypeSection(PCIRTypeSection& section) {
    assert(stream.is_open());

    stream >> section.sizeOfTypeSection
           >> section.numTypes;
    section.types.resize(section.numTypes);
    for(auto& type : section.types) {
      stream >> type.kind;
      switch(type.kind) {
        // Array
        case 0x0001: {
          PCIRTypeArray array;
          stream >> array.elemType
                 >> array.arraySize;
          type.variant = array;
          break;
        }
        // Tuple
        case 0x0002: {
          PCIRTypeTuple tuple;
          stream >> tuple.numElems;
          tuple.elements.resize(tuple.numElems);
          for(auto& elem : tuple.elements) {
            stream >> elem;
          }
          type.variant = tuple;
          break;
        }
        // Pointer
        case 0x0003: {
          PCIRTypePointer pointer;
          stream >> pointer.entityType;
          type.variant = pointer;
          break;
        }
        // Function
        case 0x0004: {
          PCIRTypeFunction fn;
          stream >> fn.returnType
                 >> fn.numArgs;
          fn.args.resize(fn.numArgs);
          for(auto& arg : fn.args) {
            stream >> arg;
          }
          type.variant = fn;
          break;
        }
        // External
        case 0x0007: {
          PCIRTypeExternal ext;
          stream >> ext.name
                 >> ext.moduleName
                 >> ext.fileID;
          type.variant = ext;
          break;
        }
        default:
          // TODO: return error.
          assert(false);
      }
    }
  }
  void PCIRDecoder::loadSymbolSection(PCIRSymbolSection& section) {
    assert(stream.is_open());

    stream >> section.sizeOfSymbolSection
           >> section.numSymbols;
    section.symbols.resize(section.numSymbols);
    for(auto& symbol : section.symbols) {
      stream >> symbol.name
             >> symbol.kind;
      switch(symbol.kind) {
        // Internal
        case 0x0001: {
          PCIRInternalSymbol s;
          stream >> s.type
                 >> s.mutability
                 >> s.initFn;
          symbol.variant = s;
          break;
        }
        // External
        case 0x0002: {
          PCIRExternalSymbol s;
          stream >> s.moduleName
                 >> s.fileID;
          symbol.variant = s;
          break;
        }
        default:
          // TODO: return error.
          assert(false);
      }
    }
  }
  void PCIRDecoder::loadFunctionSection(PCIRFunctionSection& section) {
    assert(stream.is_open());

    stream >> section.sizeOfFunctionSection
           >> section.numFunctions;
    section.functions.resize(section.numFunctions);
    for(auto& fn : section.functions) {
      stream >> fn.type
             >> fn.kind;
      switch(fn.kind) {
        // Internal
        case 0x0001: {
          PCIRInternalFunction f;
          stream >> f.numVars
                 >> f.numBlocks;
          f.vars.resize(f.numVars);
          for(auto& var : f.vars) {
            stream >> var.type;
          }
          f.blocks.resize(f.numBlocks);
          for(auto& block : f.blocks) {
            stream >> block.kind
                   >> block.sizeOfByteCode;
            switch(block.kind) {
              // NonTerminalBlock
              case 0x0001: {
                PCIRNonTerminalBlock b;
                stream >> b.nextBlock;
                block.variant = b;
                break;
              }
              // TerminalBlock
              case 0x0002: {
                PCIRTerminalBlock b;
                stream >> b.returnValue;
                block.variant = b;
                break;
              }
              // ConditionalBranchBlock
              case 0x0003: {
                PCIRConditionalBranchBlock b;
                stream >> b.conditionalValue
                       >> b.thenBlock
                       >> b.elseBlock;
                block.variant = b;
                break;
              }
              default:
                // TODO: return error.
                assert(false);
                break;
            }
            block.byteCode.resize(block.sizeOfByteCode);
            stream.read((char*)block.byteCode.data(), block.sizeOfByteCode);
          }
          fn.variant = f;
          break;
        }
        // External
        case 0x0002: {
          PCIRExternalFunction f;
          stream >> f.name;
          fn.variant = f;
          break;
        }
        default:
          // TODO: return error.
          assert(false);
      }  
    }
  }
}