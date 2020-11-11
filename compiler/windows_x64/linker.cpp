#include "linker.h"

#include <filesystem>
#include <fstream>

#include "utils/vector_utils.h"
#include "utils/map_utils.h"
#include "pcir/pcir_format.h"

namespace pickc::windows::x64
{
  uint32_t Linker::alignment(uint32_t value, uint32_t alignment)
  {
    return (value + alignment - 1) / alignment * alignment;
  }
  Linker::Linker(const WindowsX64& x64) :
    x64(x64),
    dosHeader{
      0x5A4D,                                     // Magic number
      0x0090,                                     // Bytes on last page of file
      0x0003,                                     // Pages in file
      0x0000,                                     // Relocations
      0x0004,                                     // Size of header in paragraphs
      0x0000,                                     // Minimum extra paragraphs needed
      0xFFFF,                                     // Maximum extra paragraphs needed
      0x0000,                                     // Initial (relative) SS value
      0x00B8,                                     // Initial SP value
      0x0000,                                     // Checksum
      0x0000,                                     // Initial IP value
      0x0000,                                     // Initial (relative) CS value
      0x0040,                                     // File address of relocation table
      0x0000,                                     // Overlay number
      {},                                         // Reserved words
      0x0000,                                     // OEM identifier (for e_oeminfo)
      0x0000,                                     // OEM information; e_oemid specific
      {},                                         // Reserved words
      0x48                                        // File address of new exe header
    },
    stub{
      0xB8, 0x01, 0x4C,                           // mov ax 0x014C [al: return code, ah: syscall return]
      0xCD, 0x21,                                 // system call
      0x90, 0x90, 0x90,                           // alignment (nop)
    },
    ntHeader{
      0x00004550,                               // Signature
      FileHeader{
        0x8664,                                 // Machine (x86)
        0x0003,                                 // Number of sections
        (uint32_t)std::time(nullptr),           // Timestamp
        0x00000000,                             // Pointer to symbol table
        0x00000000,                             // Number of symbols
        sizeof(OptionalHeader),                 // Size of optional header
        0x0022,                                 // Characteristics
      },
      OptionalHeader{
        0x020B,                                 // Magic number
        0x00,                                   // Major linker version
        0x00,                                   // Minor linker version
        0x00000000,                             // Size of code
        0x00000000,                             // Size of init data
        0x00000000,                             // Size of uninit data
        0x00001000,                             // Address of entry point
        0x00000000,                             // Base of code
        // 0x00000000,                             // Base of data
        0x00400000,                             // Image base
        0x00001000,                             // Section alignment
        0x00000200,                             // File alignment
        0x0006,                                 // Major operating system version
        0x0000,                                 // Minor operation system version
        0x0000,                                 // Major image version
        0x0000,                                 // Minor image version
        0x0006,                                 // Major subsystem version
        0x0000,                                 // Minor subsystem version
        0x00000000,                             // Win32 version value (Reserved)
        0x00000000,                             // Size of image
        0x00000000,                             // Size of header
        0x00000000,                             // Checksum
        0x0003,                                 // subsystem (/SUBSYSTEM:CONSOLE)
        0x8540,                                 // DLL characteristics
        0x00100000,                             // Size of stack reserve
        0x00001000,                             // Size of stack commit
        0x00100000,                             // Size of heap reserve
        0x00001000,                             // Size of heap commit
        0x00000000,                             // Loader flags (unused)
        0x00000010,                             // Number of RVA and sizes
        {}                                      // Data directory
      }
    },
    textSection{
      ".text\0\0",
      0,
      0,
      0,
      0,
      0, 0, 0, 0,
      0x60000020
    },
    rdataSection{
      ".rdata\0",
      0,
      0,
      0,
      0,
      0, 0, 0, 0,
      0x40000040
    },
    dataSection{
      ".data\0\0",
      0,
      0,
      0,
      0,
      0, 0, 0, 0,
      0xC0000080
    },
    relocSection{
      ".reloc\0",
      0,
      0,
      0,
      0,
      0, 0, 0, 0,
      0x42000040
    }
  {}
  Result<_, std::vector<std::string>> Linker::link(const CompilerOption& option)
  {
    std::vector<std::string> errors;

    placeRoutines();

    ntHeader.fileHeader.numSections = 4;
    ntHeader.optionalHeader.sizeOfHeaders = alignment(sizeof(DOSHeader) + sizeof(stub) + sizeof(NTHeader) + sizeof(SectionHeader) * ntHeader.fileHeader.numSections, ntHeader.optionalHeader.fileAlignment);

    placeTextSection();
    auto libRes = loadLibs(option);
    if(!libRes) errors += libRes.err();
    placeRDataSection();
    placeDataSection();
    placeRelocation();

    ntHeader.optionalHeader.baseOfCode = textSection.virtualAddress;

    ntHeader.optionalHeader.sizeOfImage = ntHeader.optionalHeader.addressOfEntryPoint + ntHeader.optionalHeader.sectionAlignment * ntHeader.fileHeader.numSections;

    auto writeRes = write(option);
    if(!writeRes) errors += writeRes.err();

    if(errors.empty()) return ok();
    return error(errors);
  }
  void Linker::placeRoutines()
  {
    uint64_t routineAddress = 0;
    for(auto& routine : x64.routines) {
      routine.second->address = routineAddress;
      for(auto& op : routine.second->code) {
        routine.second->codeIndexes[op] = routine.second->nativeCode.size();
        routine.second->nativeCode << op->bin(x64, routine.second);
      }
      routine.second->codeIndexes[nullptr] = routine.second->nativeCode.size();
      routineAddress += routine.second->nativeCode.size();
    }
  }
  void Linker::placeTextSection()
  {
    uint64_t address = ntHeader.optionalHeader.imageBase + ntHeader.optionalHeader.addressOfEntryPoint;
    for (auto& routine : x64.routines) {
      routine.second->address = address;
      address += routine.second->nativeCode.size();
      ntHeader.optionalHeader.sizeOfCode += static_cast<uint32_t>(routine.second->nativeCode.size());
    }
    textSection.misc.virtualSize = ntHeader.optionalHeader.sizeOfCode;
    textSection.sizeOfRawData = alignment(textSection.misc.virtualSize, ntHeader.optionalHeader.fileAlignment);
    textSection.ptrToRawData = ntHeader.optionalHeader.sizeOfHeaders;
    textSection.virtualAddress = ntHeader.optionalHeader.addressOfEntryPoint;

    ntHeader.optionalHeader.sizeOfCode = textSection.sizeOfRawData;
  }
  void Linker::placeRDataSection()
  {
    uint64_t address = ntHeader.optionalHeader.imageBase
                     + ntHeader.optionalHeader.addressOfEntryPoint
                     + alignment(ntHeader.optionalHeader.sizeOfCode, ntHeader.optionalHeader.sectionAlignment)
                     + rdataSectionRawData.size();

    for(auto& text : x64.texts) {
      text.second = address;
      address += text.first.size() + 1;
      rdataSectionRawData.reserve(rdataSectionRawData.size() + text.first.size() + 1);
      for(auto c : text.first) {
        rdataSectionRawData.push_back(c);
      }
      rdataSectionRawData.push_back('\0');
    }
    if(ntHeader.optionalHeader.sizeOfInitData == 0) {
      rdataSectionRawData.resize(8);
    }
    ntHeader.optionalHeader.sizeOfInitData = rdataSectionRawData.size();
    rdataSection.misc.virtualSize = ntHeader.optionalHeader.sizeOfInitData;
    rdataSection.sizeOfRawData = alignment(rdataSection.misc.virtualSize, ntHeader.optionalHeader.fileAlignment);
    rdataSection.ptrToRawData = textSection.ptrToRawData + textSection.sizeOfRawData;
    rdataSection.virtualAddress = textSection.virtualAddress + alignment(textSection.sizeOfRawData, ntHeader.optionalHeader.sectionAlignment);
    ntHeader.optionalHeader.sizeOfInitData = rdataSection.sizeOfRawData;
  }
  void Linker::placeDataSection()
  {
    uint64_t address = ntHeader.optionalHeader.imageBase
                     + ntHeader.optionalHeader.addressOfEntryPoint
                     + alignment(ntHeader.optionalHeader.sizeOfCode, ntHeader.optionalHeader.sectionAlignment)
                     + alignment(rdataSection.sizeOfRawData, ntHeader.optionalHeader.sectionAlignment);
    for(auto& symbol : x64.symbols) {
      symbol.second = address + ntHeader.optionalHeader.sizeOfUninitData;
      ntHeader.optionalHeader.sizeOfUninitData += x64.typeTable[symbol.first->type].getSize();
    }
    dataSection.misc.virtualSize = ntHeader.optionalHeader.sizeOfUninitData;
    dataSection.sizeOfRawData = alignment(dataSection.misc.virtualSize, ntHeader.optionalHeader.fileAlignment);
    dataSection.ptrToRawData = rdataSection.ptrToRawData + rdataSection.sizeOfRawData;
    dataSection.virtualAddress = rdataSection.virtualAddress + alignment(rdataSection.sizeOfRawData, ntHeader.optionalHeader.sectionAlignment);
    ntHeader.optionalHeader.sizeOfUninitData = dataSection.sizeOfRawData;
  }
  void Linker::placeRelocation()
  {
    for(auto& reloc : x64.relocs) {
      size_t value;
      auto index = reloc->routine->codeIndexes[reloc->op] + reloc->index;
      size_t vaddress = reloc->routine->address + index - ntHeader.optionalHeader.imageBase;
      uint32_t rva = alignment(vaddress - 0x1000, 0x1000);
      switch(reloc->reloc.type) {
        case RelocationType::Function:
          switch(reloc->pos) {
            case RelocationPosition::Relative: {
              size_t base;
              auto itr = std::find(reloc->routine->code.begin(), reloc->routine->code.end(), reloc->op) + 1;
              if(itr != reloc->routine->code.end()) {
                base = reloc->routine->codeIndexes[*(std::find(reloc->routine->code.begin(), reloc->routine->code.end(), reloc->op) + 1)];
              }
              else {
                base = reloc->routine->codeIndexes[nullptr];
              }
              value = x64.routines[reloc->reloc.fn]->address - (reloc->routine->address + base);
              break;
            }
            case RelocationPosition::Absolute:
              value = x64.routines[reloc->reloc.fn]->address;
              relocs[rva].base.sizeOfBlock += 2;
              relocs[rva].rva.push_back(0x3000 | ((vaddress) & 0x0FFF));
              break;
            default:
              assert(false);
          }
          break;
        case RelocationType::Symbol:
          value = x64.symbols[reloc->reloc.symbol];
          relocs[rva].base.sizeOfBlock += 2;
          relocs[rva].rva.push_back(0x3000 | ((vaddress) & 0x0FFF));
          break;
        case RelocationType::Text:
          value = x64.texts[reloc->reloc.text->text];
          relocs[rva].base.sizeOfBlock += 2;
          relocs[rva].rva.push_back(0x3000 | ((vaddress) & 0x0FFF));
          break;
        case RelocationType::JmpTo:
          value = reloc->routine->codeIndexes[reloc->routine->code[reloc->routine->bundleIndexes[reloc->reloc.jmpTo]]] - (reloc->routine->codeIndexes[reloc->op] + reloc->index + 4);
          break;
        case RelocationType::Extern: {
          size_t base;
          auto itr = std::find(reloc->routine->code.begin(), reloc->routine->code.end(), reloc->op) + 1;
          if(itr != reloc->routine->code.end()) {
            base = reloc->routine->codeIndexes[*(std::find(reloc->routine->code.begin(), reloc->routine->code.end(), reloc->op) + 1)];
          }
          else {
            base = reloc->routine->codeIndexes[nullptr];
          }
          value = libSymbols[reloc->reloc.ext].address - (reloc->routine->address + base);
          break;
        }
        default:
          assert(false);
      }
      switch(reloc->size) {
        case OperationSize::Byte:
          reloc->routine->nativeCode[index] = static_cast<uint8_t>(value >> 0);
          break;
        case OperationSize::Word:
          reloc->routine->nativeCode[index] = static_cast<uint8_t>(value >> 0);
          reloc->routine->nativeCode[index + 1] = static_cast<uint8_t>(value >> 8);
          break;
        case OperationSize::DWord:
          reloc->routine->nativeCode[index] = static_cast<uint8_t>(value >> 0);
          reloc->routine->nativeCode[index + 1] = static_cast<uint8_t>(value >> 8);
          reloc->routine->nativeCode[index + 2] = static_cast<uint8_t>(value >> 16);
          reloc->routine->nativeCode[index + 3] = static_cast<uint8_t>(value >> 24);
          break;
        case OperationSize::QWord:
          reloc->routine->nativeCode[index] = static_cast<uint8_t>(value >> 0);
          reloc->routine->nativeCode[index + 1] = static_cast<uint8_t>(value >> 8);
          reloc->routine->nativeCode[index + 2] = static_cast<uint8_t>(value >> 16);
          reloc->routine->nativeCode[index + 3] = static_cast<uint8_t>(value >> 24);
          reloc->routine->nativeCode[index + 4] = static_cast<uint8_t>(value >> 32);
          reloc->routine->nativeCode[index + 5] = static_cast<uint8_t>(value >> 40);
          reloc->routine->nativeCode[index + 6] = static_cast<uint8_t>(value >> 48);
          reloc->routine->nativeCode[index + 7] = static_cast<uint8_t>(value >> 56);
          break;
        default:
          assert(false);
      }
    }

    for(auto& reloc : relocs) {
      reloc.second.base.virtualAddress = reloc.first;
      reloc.second.base.sizeOfBlock += sizeof(ImageBaseRelocation);
      relocSection.misc.virtualSize += reloc.second.base.sizeOfBlock;
    }
    relocSection.sizeOfRawData = alignment(relocSection.misc.virtualSize, ntHeader.optionalHeader.fileAlignment);

    relocSection.ptrToRawData = dataSection.ptrToRawData + dataSection.sizeOfRawData;
    relocSection.virtualAddress = dataSection.virtualAddress + alignment(relocSection.sizeOfRawData, ntHeader.optionalHeader.sectionAlignment);

    relocSection.ptrToRawData = dataSection.ptrToRawData + dataSection.sizeOfRawData;
    relocSection.virtualAddress = dataSection.virtualAddress + alignment(relocSection.sizeOfRawData, ntHeader.optionalHeader.sectionAlignment);

    ntHeader.optionalHeader.sizeOfInitData += relocSection.sizeOfRawData;

    ntHeader.optionalHeader.dataDirectory[5].size = relocSection.misc.virtualSize;
    ntHeader.optionalHeader.dataDirectory[5].virtualAddress = relocSection.virtualAddress;
  }
  Result<_, std::vector<std::string>> Linker::loadLibs(const CompilerOption& option)
  {
    std::vector<std::string> errors;

    for (const auto& lib : option.libraries) {
      auto res = LibLoader(lib).load();
      if (!res) errors += res.err();
      else libSymbols.merge(res.get());
    }

    const uint64_t rdataBase = ntHeader.optionalHeader.imageBase
                             + ntHeader.optionalHeader.addressOfEntryPoint
                             + alignment(ntHeader.optionalHeader.sizeOfCode, ntHeader.optionalHeader.sectionAlignment);

    std::map<std::string, DLL> dlls;
    // 使用するDLLを列挙する。
    for(auto& ext : x64.externs) {
      if(!keyExists(libSymbols, ext.second)) {
        errors.push_back("リンクエラー: シンボル " + ext.second + " が見つかりませんでした。");
      }
      else {
        // マーキング
        dlls[libSymbols[ext.second].header.name].symbols[ext.second] = 0;
      }
    }
    for (auto& dll : dlls) {
      dll.second.thunkIndex = ntHeader.optionalHeader.dataDirectory[12].size + rdataBase - ntHeader.optionalHeader.imageBase;
      ntHeader.optionalHeader.dataDirectory[12].size += 8 * (dll.second.symbols.size() + 1);
    }

    uint64_t address = rdataBase + ntHeader.optionalHeader.dataDirectory[12].size;

    if (ntHeader.optionalHeader.dataDirectory[12].size != 0) {
      ntHeader.optionalHeader.dataDirectory[12].virtualAddress = rdataBase - ntHeader.optionalHeader.imageBase;
      rdataSectionRawData.resize(rdataSectionRawData.size() + ntHeader.optionalHeader.dataDirectory[12].size);
    }

    if (ntHeader.optionalHeader.dataDirectory[12].size != 0) {
      ntHeader.optionalHeader.dataDirectory[1].virtualAddress = address - ntHeader.optionalHeader.imageBase;
      auto thunkBase = address + sizeof(ImportDescriptor) * (dlls.size() + 1) - ntHeader.optionalHeader.imageBase;
      auto iatBase = thunkBase;
      for (auto& dll : dlls) {
        dll.second.index = iatBase;
        iatBase += sizeof(ThunkData) * (dll.second.symbols.size() + 1);
      }
      auto symbolIndex = iatBase;
      for (auto& dll : dlls) {
        for (auto& symbol : dll.second.symbols) {
          symbol.second = symbolIndex;
          symbolIndex += (sizeof(ImportByName::hint) + symbol.first.size() + 2) / 2 * 2;
        }
        dll.second.nameIndex = symbolIndex;
        symbolIndex += alignment(dll.first.size() + 1, 2);
      }

      for (auto& dll : dlls) {
        ImportDescriptor disc{};
        disc.originalFirstThunk = dll.second.index;
        disc.name = dll.second.nameIndex;
        disc.firstThunk = dll.second.thunkIndex;
        rdataSectionRawData << disc;
      }
      rdataSectionRawData.resize(rdataSectionRawData.size() + sizeof(ImportDescriptor));

      size_t index = 0;
      for (auto& dll : dlls) {
        for (auto& symbol : dll.second.symbols) {
          rdataSectionRawData << symbol.second;
          libSymbols[symbol.first].address = index + rdataBase;
          rdataSectionRawData[index++] = ((symbol.second >> 0) & 0xFF);
          rdataSectionRawData[index++] = ((symbol.second >> 8) & 0xFF);
          rdataSectionRawData[index++] = ((symbol.second >> 16) & 0xFF);
          rdataSectionRawData[index++] = ((symbol.second >> 24) & 0xFF);
          rdataSectionRawData[index++] = ((symbol.second >> 32) & 0xFF);
          rdataSectionRawData[index++] = ((symbol.second >> 40) & 0xFF);
          rdataSectionRawData[index++] = ((symbol.second >> 48) & 0xFF);
          rdataSectionRawData[index++] = ((symbol.second >> 56) & 0xFF);
        }
        index += sizeof(uint64_t);
        rdataSectionRawData.resize(rdataSectionRawData.size() + sizeof(uint64_t));
      }
      ntHeader.optionalHeader.dataDirectory[1].size = index;

      for (auto& dll : dlls) {
        for (auto& symbol : dll.second.symbols) {
          rdataSectionRawData << libSymbols[symbol.first].member.header.hint;
          for (auto c : symbol.first) {
            rdataSectionRawData << c;
          }
          rdataSectionRawData << '\0';
          if (symbol.first.size() % 2 == 0) rdataSectionRawData << '\0';
        }
        for (auto c : dll.first) {
          rdataSectionRawData << c;
        }
        rdataSectionRawData << '\0';
        if (dll.first.size() % 2 == 0) rdataSectionRawData << '\0';
      }
    }

    ntHeader.optionalHeader.sizeOfInitData = rdataSectionRawData.size();

    if (errors.empty()) return ok();
    else return error(errors);
  }
  Result<_, std::vector<std::string>> Linker::write(const CompilerOption& option)
  {
    auto path = std::filesystem::path(option.outDir) / (option.out + ".exe");
    std::ofstream stream(path, std::ios::binary);
    if(!stream) {
      return error(std::vector{ "ファイル " +  path.string() + " が開けません。" });
    }

    stream.write((char*)&dosHeader, sizeof(dosHeader));
    stream.write((char*)stub, sizeof(stub));
    stream.write((char*)&ntHeader, sizeof(ntHeader));
    stream.write((char*)&textSection, sizeof(textSection));
    stream.write((char*)&rdataSection, sizeof(rdataSection));
    // if (!x86_64.runtimeSymbols.empty()) {
    stream.write((char*)&dataSection, sizeof(dataSection));
    // }
    stream.write((char*)&relocSection, sizeof(relocSection));

    auto pos = sizeof(DOSHeader) + sizeof(stub) + sizeof(NTHeader) + sizeof(SectionHeader) * ntHeader.fileHeader.numSections;
    if (pos % ntHeader.optionalHeader.fileAlignment) {
      auto s = ntHeader.optionalHeader.fileAlignment - pos % ntHeader.optionalHeader.fileAlignment;
      char* b = new char[s];
      memset(b, 0x00, s);
      stream.write(b, s);
      delete[] b;
    }

    for (const auto& routine : x64.routines) {
      stream.write((char*)routine.second->nativeCode.data(), routine.second->nativeCode.size());
    }

    if (textSection.misc.virtualSize % ntHeader.optionalHeader.fileAlignment) {
      auto s = ntHeader.optionalHeader.fileAlignment - textSection.misc.virtualSize % ntHeader.optionalHeader.fileAlignment;
      char* b = new char[s];
      memset(b, 0x00, s);
      stream.write(b, s);
      delete[] b;
    }

    stream.write((char*)rdataSectionRawData.data(), rdataSectionRawData.size());
    char* b = new char[rdataSection.sizeOfRawData - rdataSectionRawData.size()];
    memset(b, 0x00, rdataSection.sizeOfRawData - rdataSectionRawData.size());
    stream.write(b, rdataSection.sizeOfRawData - rdataSectionRawData.size());
    delete[] b;

    // if (!x64.runtimeSymbols.empty()) {
    b = new char[dataSection.sizeOfRawData];
    memset(b, 0x00, dataSection.sizeOfRawData);
    stream.write(b, dataSection.sizeOfRawData);
    delete[] b;
    // }

    for(auto reloc : relocs) {
      stream.write((char*)&reloc.second.base, sizeof(ImageBaseRelocation));
      stream.write((char*)reloc.second.rva.data(), reloc.second.rva.size() * 2);
    }

    b = new char[relocSection.sizeOfRawData - relocSection.misc.virtualSize];
    memset(b, 0x00, relocSection.sizeOfRawData - relocSection.misc.virtualSize);
    stream.write(b, relocSection.sizeOfRawData - relocSection.misc.virtualSize);
    delete[] b;

    return ok();
  }
}