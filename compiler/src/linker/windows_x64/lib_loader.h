/**
 * @file lib_loader.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-08-06
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_LINKER_WINDOWS_X64_LIB_LOADER_H_
#define PICKC_LINKER_WINDOWS_X64_LIB_LOADER_H_

#include <string>
#include <vector>
#include <map>

#include "utils/binary_vec.h"

namespace pickc::linker::windows_x64 {
  struct alignas(1) ArchiveMemberHeader {
    char name[16];
    char date[12];
    char userID[6];
    char groupID[6];
    char mode[8];
    char size[10];
    char endHeader[2];
    std::string getName() const;
    size_t getSize() const;
    bool isValid() const;
  };
  static_assert(sizeof(ArchiveMemberHeader) == 60);
  class LibLoader {
    bool open;
    BinaryVec bin;
    std::vector<ArchiveMemberHeader*> headers;
    std::vector<uint32_t> memberOffsets;
    std::map<std::string, uint16_t> symbols;
  public:
    LibLoader(const std::string& filename);
    bool isOpen() const;
  private:
    void load(const std::string& filename);
    void loadHeaders();
    void analyzeSecondLinkerMember();
  };
}

#endif // PICKC_LINKER_WINDOWS_X64_LIB_LOADER_H_