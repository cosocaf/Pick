#ifndef PICKC_PCIR_STRUCT_H_
#define PICKC_PCIR_STRUCT_H_

/**
 * バイナリフォーマットについてはpcir_format.hを参照。
 * ここでは、PCIRを扱いやすくするための構造体を定義する。
*/

#include <vector>
#include <string>

#include "pcir.h"
#include "utils/result.h"
#include "utils/binary_vec.h"

namespace pickc::pcir
{
  struct TextSection
  {
    std::string text;
  };
  struct TypeSection
  {
    Types types;
    Type type;
    uint32_t indexOfElem;
    uint32_t elemLength;
    uint32_t indexOfRet;
    uint32_t numOfArgs;
    std::vector<uint32_t> indexOfArgs;
  };
  struct RegisterStruct
  {
    TypeSection* type;
  };
  struct FlowStruct
  {
    FlowStruct* parent;
    uint32_t flowType;
    FlowStruct* next;
    RegisterStruct* cond;
    FlowStruct* thenFlow;
    FlowStruct* elseFlow;
    RegisterStruct* retReg;
    BinaryVec code;
  };
  struct FunctionSection
  {
    TypeSection* type;
    std::vector<RegisterStruct*> regs;
    std::vector<FlowStruct*> flows;
    FlowStruct* entryFlow;
  };
  struct SymbolSection
  {
    TextSection* name;
    Scope scope;
    Mutability mut;
    TypeSection* type;
    FunctionSection* init;
  };
  struct ModuleSection
  {
    TextSection* name;
    std::vector<SymbolSection*> symbols;
  };
  struct PCIRFile
  {
    char magic[4];
    uint64_t timeStamp;
    uint16_t majorVersion;
    uint16_t minorVersion;
    uint32_t ptrToTextHeader;
    uint32_t ptrToModuleHeader;
    uint32_t ptrToTypeTableHeader;
    uint32_t ptrToSymbolTableHeader;
    uint32_t ptrToFunctionTableHeader;
    std::vector<TextSection*> textSection;
    std::vector<TypeSection*> typeSection;
    std::vector<SymbolSection*> symbolSection;
    std::vector<FunctionSection*> fnSection;
    std::vector<ModuleSection*> moduleSection;
  };
  class PCIRLoader
  {
    PCIRFile file;
  public:
    PCIRLoader();
    Result<PCIRFile, std::vector<std::string>> load(const std::string& path);
  };
}

#endif // PICKC_PCIR_STRUCT_H_