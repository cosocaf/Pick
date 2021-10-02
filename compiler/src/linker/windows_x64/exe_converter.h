/**
 * @file exe_converter.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-10-01
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_LINKER_WINDOWS_X64_EXE_CONVERTER_H_
#define PICKC_LINKER_WINDOWS_X64_EXE_CONVERTER_H_

#include <map>
#include <array>
#include <vector>

#include "../routine.h"
#include "exe_format.h"

#include "utils/binary_vec.h"

namespace pickc::linker::windows_x64 {
  class EXEConverter {
    std::map<std::array<uint8_t, 16>, std::vector<Routine>> routines;
    DOSHeader dosHeader;
    uint8_t stub[8];
    NTHeader ntHeader;
    SectionHeader textSection;
    SectionHeader rdataSection;
    SectionHeader dataSection;
    SectionHeader relocSection;
  public:
    EXEConverter(const std::map<std::array<uint8_t, 16>, std::vector<Routine>>& routines);
    BinaryVec convert();
  private:
    void init();
    void placeTextSection();
    void placeRDataSection();
    void placeDataSection();
    void placeRelocSection();
  };
}

#endif // PICKC_LINKER_WINDOWS_X64_EXE_CONVERTER_H_