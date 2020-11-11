#ifndef PICKC_WINDOWS_X64_LINKER_H_
#define PICKC_WINDOWS_X64_LINKER_H_

#include <vector>
#include <string>
#include <map>

#include "pickc/compiler_option.h"
#include "utils/result.h"
#include "utils/binary_vec.h"
#include "routine.h"
#include "exe_format.h"
#include "lib_loader.h"

namespace pickc::windows::x64
{
  struct DLL
  {
    uint32_t index;
    uint32_t nameIndex;
    uint32_t thunkIndex;
    std::map<std::string, uint64_t> symbols;
  };
  class Linker
  {
    WindowsX64 x64;
    DOSHeader dosHeader;
    uint8_t stub[8];
    NTHeader ntHeader;
    SectionHeader textSection;
    SectionHeader rdataSection;
    SectionHeader dataSection;
    SectionHeader relocSection;

    std::map<std::string, LibSymbol> libSymbols;
    std::map<uint32_t, RelocationBase> relocs;

    BinaryVec textSectionRawData;
    BinaryVec rdataSectionRawData;
    BinaryVec dataSectionRawData;

    static uint32_t alignment(uint32_t value, uint32_t alignment);
    
    void placeRoutines();
    void placeTextSection();
    void placeRDataSection();
    void placeDataSection();
    void placeRelocation();
    Result<_, std::vector<std::string>> loadLibs(const CompilerOption& option);
    Result<_, std::vector<std::string>> write(const CompilerOption& option);
  public:
    Linker(const WindowsX64& x64);
    Result<_, std::vector<std::string>> link(const CompilerOption& option);
  };
}

#endif // PICKC_WINDOWS_X64_LINKER_H_