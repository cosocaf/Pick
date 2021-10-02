/**
 * @file pcir_decoder.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-09-06
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_BUNDLER_PACKAGER_H_
#define PICKC_BUNDLER_PACKAGER_H_

#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <fstream>

#include "utils/binary_vec.h"
#include "utils/file_id.h"

namespace pickc::bundler {
  struct PCIRFileHeader {
    char signature[4];
    uint16_t majorVersion, minorVersion;
    uint64_t timestamp;
    uint8_t fileID[16];
    uint32_t ptrTextSection, ptrModuleSection, ptrTypeSection, ptrSymbolSection, ptrFnSection;
  };
  struct PCIRTextSection {
    uint32_t sizeOfTextSection;
    uint32_t numTexts;
    std::vector<std::string> texts;
  };
  struct PCIRModuleType {
    uint32_t typeIndex;
    uint16_t accessibility;
  };
  struct PCIRModuleSymbol {
    uint32_t symbolIndex;
    uint16_t accessibility;
  };
  struct PCIRModuleFunction {
    uint32_t functionIndex;
  };
  struct PCIRModule {
    uint32_t moduleName;
    uint32_t parentName;
    uint16_t accessibility;
    uint32_t numTypes;
    uint32_t numSymbols;
    uint32_t numFunctions;
    std::vector<PCIRModuleType> types;
    std::vector<PCIRModuleSymbol> symbols;
    std::vector<PCIRModuleFunction> functions;
  };
  struct PCIRModuleSection {
    uint32_t sizeOfModuleSection;
    uint32_t numModules;
    std::vector<PCIRModule> modules;
  };
  struct PCIRTypeArray {
    uint32_t elemType;
    uint32_t arraySize;
  };
  struct PCIRTypeTuple {
    uint32_t numElems;
    std::vector<uint32_t> elements;
  };
  struct PCIRTypePointer {
    uint32_t entityType;
  };
  struct PCIRTypeFunction {
    uint32_t returnType;
    uint32_t numArgs;
    std::vector<uint32_t> args;
  };
  struct PCIRTypeStruct {
    // TODO
  };
  struct PCIRTypeClass {
    // TODO
  };
  struct PCIRTypeExternal {
    uint32_t name;
    uint32_t moduleName;
    uint8_t fileID[16];
  };
  struct PCIRType {
    uint16_t kind;
    std::variant<PCIRTypeArray, PCIRTypeTuple, PCIRTypePointer, PCIRTypeFunction, PCIRTypeExternal> variant;
  };
  struct PCIRTypeSection {
    uint32_t sizeOfTypeSection;
    uint32_t numTypes;
    std::vector<PCIRType> types;
  };
  struct PCIRInternalSymbol {
    uint32_t type;
    uint16_t mutability;
    uint32_t initFn;
  };
  struct PCIRExternalSymbol {
    uint32_t moduleName;
    uint8_t fileID[16];
  };
  struct PCIRSymbol {
    uint32_t name;
    uint16_t kind;
    std::variant<PCIRInternalSymbol, PCIRExternalSymbol> variant;
    static constexpr uint16_t Internal = 0x0001;
    static constexpr uint16_t External = 0x0002;
  };
  struct PCIRSymbolSection {
    uint32_t sizeOfSymbolSection;
    uint32_t numSymbols;
    std::vector<PCIRSymbol> symbols;
  };
  struct PCIRVariable {
    uint32_t type;
  };
  struct PCIRNonTerminalBlock {
    uint32_t nextBlock;
  };
  struct PCIRTerminalBlock {
    uint32_t returnValue;
  };
  struct PCIRConditionalBranchBlock {
    uint32_t conditionalValue;
    uint32_t thenBlock;
    uint32_t elseBlock;
  };
  struct PCIRBlock {
    uint16_t kind;
    uint32_t sizeOfByteCode;
    std::variant<PCIRNonTerminalBlock, PCIRTerminalBlock, PCIRConditionalBranchBlock> variant;
    BinaryVec byteCode;
  };
  struct PCIRInternalFunction {
    uint32_t numVars;
    uint32_t numBlocks;
    std::vector<PCIRVariable> vars;
    std::vector<PCIRBlock> blocks;
  };
  struct PCIRExternalFunction {
    uint32_t name;
  };
  struct PCIRFunction {
    uint32_t type;
    uint16_t kind;
    std::variant<PCIRInternalFunction, PCIRExternalFunction> variant;
  };
  struct PCIRFunctionSection {
    uint32_t sizeOfFunctionSection;
    uint32_t numFunctions;
    std::vector<PCIRFunction> functions;
  };
  struct PCIR {
    PCIRFileHeader header;
    PCIRTextSection textSection;
    PCIRModuleSection moduleSection;
    PCIRTypeSection typeSection;
    PCIRSymbolSection symbolSection;
    PCIRFunctionSection functionSection;
  };
  class PCIRDecoder {
    PCIR pcir;
    std::string filename;
    std::ifstream stream;
  public:
    PCIRDecoder(const std::string& filename);
    std::optional<PCIR> decode();
  private:
    void loadHeader(PCIRFileHeader& header);
    void loadTextSection(PCIRTextSection& section);
    void loadModuleSection(PCIRModuleSection& section);
    void loadTypeSection(PCIRTypeSection& section);
    void loadSymbolSection(PCIRSymbolSection& section);
    void loadFunctionSection(PCIRFunctionSection& section);
  };
}

#endif // PICKC_BUNDLER_PACKAGER_H_