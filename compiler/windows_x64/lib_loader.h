#ifndef PICKC_WINDOWS_X64_LIB_LOADER_H_
#define PICKC_WINDOWS_X64_LIB_LOADER_H_

#include <string>
#include <vector>
#include <fstream>
#include <map>

#include "utils/result.h"

namespace pickc::windows::x64
{
  struct ImportByName
  {
    uint16_t hint;
    std::string name;
  };
  struct ArchiveMemberHeader
  {
    std::string name;
    uint64_t date;
    size_t size;
  };
  struct ImportHeader
  {
    uint16_t sig1;
    uint16_t sig2;
    uint16_t version;
    uint16_t machine;
    uint32_t timestamp;
    uint32_t len;
    uint16_t hint;
    uint16_t type;
  };
  struct ArchiveMember
  {
    ImportHeader header;
    uint8_t* data;
  };
  struct LibSymbol
  {
    uint64_t address;
    ArchiveMemberHeader header;
    ArchiveMember member;
  };
  class LibLoader
  {
    std::string path;
    std::ifstream stream;
    std::map<std::string, LibSymbol> symbols;
    void loadArchiveMemberHeader(ArchiveMemberHeader& header);
    void loadImportHeader(ImportHeader& header);
  public:
    LibLoader(const std::string& path);
    Result<std::map<std::string, LibSymbol>, std::vector<std::string>> load();
  };
}

#endif // PICKC_WINDOWS_X64_LIB_LOADER_H_