#include "lib_loader.h"

namespace pickc::windows::x64
{
  namespace
  {
    inline size_t sstrlen(char* str, size_t max)
    {
      for (size_t i = 0; i < max; ++i) {
        if (str[i] == ' ') return i;
      }
      return max;
    }
  }
  void LibLoader::loadArchiveMemberHeader(ArchiveMemberHeader& header)
  {
    assert(stream);
    char n;
    stream.read(&n, 1);
    if (n != '\n') stream.seekg(-1, std::ios::cur);
    char name[16];
    char date[12];
    char userID[6];
    char groupID[6];
    char mode[8];
    char size[10];
    char end[2];
    stream.read(name, 16);
    stream.read(date, 12);
    stream.read(userID, 6);
    stream.read(groupID, 6);
    stream.read(mode, 8);
    stream.read(size, 10);
    stream.read(end, 2);
    if (!strcmp(end, "`\n")) {
      assert(false);
    }
    header.name = std::string(name).substr(0, sstrlen(name, 16));
    if (header.name[0] != '/' && header.name.back() == '/') header.name.resize(header.name.size() - 1);
    header.date = std::stoull(std::string(date).substr(0, sstrlen(date, 12)));
    header.size = std::stoull(std::string(size).substr(0, sstrlen(size, 10)));
  }
  void LibLoader::loadImportHeader(ImportHeader& header)
  {
    assert(stream);
    stream.read((char*)&header.sig1, 2);
    stream.read((char*)&header.sig2, 2);
    stream.read((char*)&header.version, 2);
    stream.read((char*)&header.machine, 2);
    stream.read((char*)&header.timestamp, 4);
    stream.read((char*)&header.len, 4);
    stream.read((char*)&header.hint, 2);
    stream.read((char*)&header.type, 2);
  }
  LibLoader::LibLoader(const std::string& path) : path(path) {}
  Result<std::map<std::string, LibSymbol>, std::vector<std::string>> LibLoader::load()
  {
    stream.open(path, std::ios::binary);
    if (!stream) {
      return error(std::vector({ "エラー: ファイルを開けませんでした。ファイル名: " + path }));
    }
    std::string magic;
    std::getline(stream, magic);
    if (magic == "!<arch>") {
      ArchiveMemberHeader first;
      loadArchiveMemberHeader(first);
      if (first.name != "/") {
        return error(std::vector({ "エラー: 正しいlibファイルではありません。第一リンカメンバがありませんでした。ファイル名: " + path }));
      }
      stream.seekg(first.size, std::ios::cur);
      ArchiveMemberHeader second;
      loadArchiveMemberHeader(second);
      if (second.name != "/") {
        return error(std::vector({ "エラー: 正しいlibファイルではありません。第二リンカメンバがありませんでした。ファイル名: " + path }));
      }
      uint32_t numMember;
      stream.read((char*)&numMember, 4);
      std::vector<uint32_t> offsets(numMember);
      for (uint32_t i = 0; i < numMember; ++i) {
        uint32_t offset;
        stream.read((char*)&offset, 4);
        offsets[i] = offset;
      }
      uint32_t numSymbols;
      stream.read((char*)&numSymbols, 4);
      std::vector<uint16_t> indices(numSymbols);
      for (uint32_t i = 0; i < numSymbols; ++i) {
        uint16_t index;
        stream.read((char*)&index, 2);
        indices[i] = index;
      }
      std::vector<std::string> strTable(numSymbols);
      for (uint32_t i = 0; i < numSymbols; ++i) {
        std::string str;
        while (!stream.eof()) {
          char c;
          stream.read(&c, 1);
          if (c == '\0') break;
          str += c;
        }
        strTable[i] = str;
      }
      std::vector<LibSymbol> members(numMember);
      for (uint32_t i = 0; i < numMember; ++i) {
        LibSymbol symbol;
        loadArchiveMemberHeader(symbol.header);
        auto pos = stream.tellg();
        loadImportHeader(symbol.member.header);
        symbol.member.data = new uint8_t[symbol.member.header.len];
        stream.read((char*)symbol.member.data, symbol.member.header.len);
        stream.seekg(pos);
        stream.seekg(symbol.header.size, std::ios::cur);
        members[i] = symbol;
      }
      for (uint32_t i = 0; i < numSymbols; ++i) {
        symbols[strTable[i]] = members[indices[i] - 1];
      }
      return ok(symbols);
    }
    return error(std::vector({ "エラー: 正しいlibファイルではありません。libファイルは!<arch>から始まる形式である必要があります。ファイル名: " + path }));
  }
}