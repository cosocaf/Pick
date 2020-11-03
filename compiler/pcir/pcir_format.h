#ifndef PICKC_PCIR_PCIR_FORMAT_H_
#define PICKC_PCIR_PCIR_FORMAT_H_

#include <cstdint>

namespace pickc::pcir
{
  constexpr uint32_t ACCESS_PUBLIC    = 0x00000001;
  constexpr uint32_t ACCESS_MUTABLE   = 0x00000002;
  // 符号付き整数型を表す。i8, i16, i32, i64
  constexpr uint32_t TYPE_SIGNED_INTEGER = 0x00000000;
  // 符号なし整数型を表す。u8, u16, u32, u64
  constexpr uint32_t TYPE_UNSIGNED_INTEGER = 0x00000001;
  // 浮動小数型を表す。f32, f64
  constexpr uint32_t TYPE_FLOAT = 0x00000002;
  // 配列型を表す。
  constexpr uint32_t TYPE_ARRAY = 0x00000004;
  // void型を表す。
  constexpr uint32_t TYPE_VOID = 0x00000008;
  // ポインタ型を表す。
  constexpr uint32_t TYPE_PTR = 0x00000010;
  // ブール型を表す。
  constexpr uint32_t TYPE_BOOL = 0x00000020;
  // 文字型を表す。
  constexpr uint32_t TYPE_CHAR = 0x00000040;
  // 関数型を表す。
  constexpr uint32_t TYPE_FUNCTION = 0x00000080;
  // ユーザー定義型を表す。
  constexpr uint32_t TYPE_USER_DEFINE = 0x00000100;
  // ジェネリクス型を表す。
  constexpr uint32_t TYPE_GENERICS = 0x00000200;

  // 通常のフローであることを表す。
  constexpr uint32_t FLOW_TYPE_NORMAL = 0x00000000;


  struct PCIRFileHeader
  {
    char magic[4];                        // "PCIR"
    uint64_t timeStamp;                   // UNIX time stamp.
    uint16_t majorVersion;                // Major PCIR version.
    uint16_t minorVersion;                // Minor PCIR version.
    uint32_t ptrToTextHeader;             // Pointer to text section header.
    uint32_t ptrToModuleHeader;           // Pointer to module section header.
    uint32_t ptrToTypeTableHeader;        // Pointer to type table header.
    uint32_t ptrToSymbolTableHeader;      // Pointer to symbol table header.
    uint32_t ptrToFunctionTableHeader;    // Pointer to function table header.
  };
  struct PCIRTextHeader
  {
    uint32_t numOfTexts;                  // Number of texts;
    // PCIRTextSection sections[numOfTexts]; // Pointer to sections.
  };
  struct PCIRTextSection
  {
    uint32_t sizeOfText;                  // Size of text.
    // char text[sizeOfText];             // text.
  };
  struct PCIRModuleHeader
  {
    uint32_t numOfModules;                // Number of modules.
    // PCIRModuleSection sections[numOfModules]; // Pointer to sections.
  };
  struct PCIRModuleSection
  {
    uint32_t name;                        // Index of module name in text section.
    uint32_t numOfSymbols;                // Number of symbols in module.
    // uint32_t indexOfSymbols[numOfSymbols] // Index of symbols in symbol section.
  };
  struct PCIRTypeTableHeader
  {
    uint32_t numOfTypes;                  // Number of types.
    // uint32_t ptrToTypes[numOfTypes]    // Pointer to types.
  };
  struct PCIRType
  {
    uint32_t flags;                       // flags. TYPE_*
    // PCIRTypeSignedInteger;             // if flags == TYPE_SIGNED_INTEGER
    // PCIRTypeUnsignedInteger;           // if flags == TYPE_UNSIGNED_INTEGER
    // PCIRTypeFloat;                     // if flags == TYPE_FLOAT
    // PCIRTypeArray;                     // if flags == TYPE_ARRAY
    // PCIRTypeVoid;                      // if flags == TYPE_VOID
    // PCIRTypePtr;                       // if flags == TYPE_PTR
    // PCIRTypeBool;                      // if flags == TYPE_BOOL
    // PCIRTypeChar;                      // if flags == TYPE_CHAR
    // PCIRTypeFunction;                  // if flags == TYPE_FUNCTION
    // PCIRTypeUserDefine;                // if flags == TYPE_USER_DEFINE
    // PCIRTypeGenerics;                  // if flags == TYPE_GENERICS
  };
  struct PCIRTypeSignedInteger
  {
    // i8 = 8, i16 = 16...
    uint32_t sizeOfType;
  };
  struct PCIRTypeUnsignedInteger
  {
    // u8 = 8, u16 = 16...
    uint32_t sizeOfType;
  };
  struct PCIRTypeFloat
  {
    // f32 = 32, f64 = 64
    uint32_t sizeOfType;
  };
  struct PCIRTypeArray
  {
    // [indexOfElemType].sizeOfType * elemLength
    uint32_t sizeOfType;
    uint32_t indexOfElemType;             // Index of element type in type table section.
    uint32_t elemLength;                  // array length
  };
  struct PCIRTypeVoid
  {
    // Always 0.
    uint32_t sizeOfType;
  };
  struct PCIRTypePtr
  {
    // Always 0. Cause the size of the pointer varies depending on the environment.
    uint32_t sizeOfType;
    uint32_t indexOfElemType;             // Index of element type in type table section.
  };
  struct PCIRTypeBool
  {
    // Always 1.
    uint32_t sizeOfType;
  };
  struct PCIRTypeChar
  {
    // Always 8.
    uint32_t sizeOfType;
  };
  struct PCIRTypeFunction
  {
    // Always 0.
    uint32_t sizeOfType;
    uint32_t indexOfReturnType;             // Index of return type in type table section.
    uint32_t numOfArgs;                     // Number of arguments
    // uint32_t args[numOfArgs];            // arguments.
  };
  struct PCIRTypeUserDefine
  {
    uint32_t sizeOfType;
    // TODO
  };
  struct PCIRTypeGenerics
  {
    // Always 0.
    uint32_t sizeOfType;
  };
  struct PCIRSymbolTableHeader
  {
    uint32_t numOfSymbols;                // Number of symbols.
    // PCIRSymbol symbols[numOfSymbols];  // Pointer to symbols,
  };
  struct PCIRSymbol
  {
    uint32_t name;                        // Index of symbol name in text section.
    uint32_t access;                      // symbol access flags. ACCESS_*
    uint32_t type;                        // Index of type name in text section.
    uint32_t init;                        // Index of init function in function table section.
  };
  struct PCIRFunctionTableHeader
  {
    uint32_t numOfFunctions;              // Number of functions.
    // PCIRFunction fns[numOfFunctions];  // Pointer to functions.
  };
  struct PCIRRegister
  {
    uint32_t type;                        // Index of type in type section.
  };
  struct PCIRFlow
  {
    uint32_t flowType;                    // FLOW_TYPE_*
    uint32_t indexOfParentFlow;           // Index of parent flow. If not, set it to -1(0xFFFFFFFF).
    // PCIRNormalBlock normal;            // if flowType == FLOW_TYPE_NORMAL
  };
  struct PCIRNormalFlow
  {
    uint32_t indexOfNextFlow;             // Index of next flow. If not, set it to -1(0xFFFFFFFF).
    uint32_t sizeOfCodes;                 // Size of codes.
    // uint8_t[sizeOfCodes];              // codes. 
  };
  struct PCIRFunction
  {
    uint32_t type;                        // Index of type in type section.
    uint32_t numOfRegisters;              // Number of virtual registers.
    // PCIRRegister[numOfRegisters];      // regs.
    uint32_t numOfFlows;                  // Number of flows.
    uint32_t indexOfEntryFlow;            // Index of entry flow.
    // PCIRFlow flows[numOfFlows];        // blocks
  };
}

#endif // PICKC_PCIR_PCIR_FORMAT_H_