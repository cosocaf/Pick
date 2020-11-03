#ifndef PICKC_WINDOWS_X64_LINKER_H_
#define PICKC_WINDOWS_X64_LINKER_H_

#include <vector>
#include <string>

#include "pickc/compiler_option.h"
#include "utils/option.h"
#include "utils/binary_vec.h"
#include "routine.h"
#include "exe_format.h"

namespace pickc::windows::x64
{
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

    std::map<uint32_t, RelocationBase> relocs;

    BinaryVec textSectionRawData;
    BinaryVec dataSectionRawData;
    BinaryVec rdataSectionRawData;

    static uint32_t alignment(uint32_t value, uint32_t alignment);
    
    void placeSymbols();
    void placeRoutines();
  public:
    Linker(const WindowsX64& x64);
    Option<std::vector<std::string>> link(const CompilerOption& option);
  };
}

#endif // PICKC_WINDOWS_X64_LINKER_H_