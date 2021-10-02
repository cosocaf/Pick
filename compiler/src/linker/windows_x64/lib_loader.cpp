/**
 * @file lib_loader.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-08-06
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "lib_loader.h"

#include <fstream>

#include "output_buffer.h"

namespace pickc::linker::windows_x64 {
  LibLoader::LibLoader(const std::string& filename) : open(false) {
    load(filename);
  }
  bool LibLoader::isOpen() const {
    return open;
  }
  void LibLoader::load(const std::string& filename) {
    std::ifstream stream(filename, std::ios::binary);
    if(!stream) {
      outputBuffer.emplace<OutputMessage>(OutputMessageKind::Error, "ファイルを開けません: " + filename);
      return;
    }

    open = true;

    stream.seekg(0, std::ios::end);
    bin.resize(static_cast<size_t>(stream.tellg()));
    stream.seekg(0, std::ios::beg);
    stream.read((char*)bin.data(), bin.size());
    stream.close();

    loadHeaders();

    analyzeSecondLinkerMember();
  }

  void LibLoader::loadHeaders() {
    size_t offset = 0;
    while(offset < bin.size()) {
      headers.push_back((ArchiveMemberHeader*)(bin.data() + offset));
      offset += sizeof(ArchiveMemberHeader);
      offset += headers.back()->getSize();
    }
  }

  void LibLoader::analyzeSecondLinkerMember() {
    if(headers.size() < 2) return;

    auto secondLinkerMember = headers[1];
    if(secondLinkerMember->getName() != "/") return;

    uint8_t* member = ((uint8_t*)secondLinkerMember) + sizeof(ArchiveMemberHeader);

    uint32_t numMembers = *(uint32_t*)member;
    member += 4;
    memberOffsets.resize(numMembers);
    for(auto& offset : memberOffsets) {
      offset = *(uint32_t*)member;
      member += 4;
    }

    uint32_t numSymbols = *(uint32_t*)member;
    std::vector<uint16_t> symbolOffsets(numSymbols);
    for(auto& offset : symbolOffsets) {
      offset = *(uint16_t*)member;
      member += 2;
    }

    for(uint32_t i = 0; i < numSymbols; ++i) {
      std::string name = (char*)member;
      member += name.size() + 1;
      symbols[name] = symbolOffsets[i] - 1;
    }
  }

  std::string ArchiveMemberHeader::getName() const {
    std::string res;
    res.reserve(16);
    for(const auto c : name) {
      if(c == ' ') break;
      res += c;
    }
    return res;
  }
  size_t ArchiveMemberHeader::getSize() const {
    size_t res = 0;
    for(const auto c : size) {
      if(c == ' ') break;
      res *= 10;
      res += c - '0';
    }
    return res;
  }
  bool ArchiveMemberHeader::isValid() const {
    return memcmp(endHeader, "`\n", 2) && getSize() != 0;
  }
}