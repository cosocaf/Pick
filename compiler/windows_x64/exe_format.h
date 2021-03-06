#ifndef PICKC_WINDOWS_X64_EXE_FORMAT_H_
#define PICKC_WINDOWS_X64_EXE_FORMAT_H_

#include <cstdint>
#include <vector>

namespace pickc::windows::x64
{
  struct DOSHeader
  {
    uint16_t magic;
    uint16_t cblp;
    uint16_t cp;
    uint16_t crlc;
    uint16_t cparhdr;
    uint16_t minalloc;
    uint16_t maxalloc;
    uint16_t ss;
    uint16_t sp;
    uint16_t csum;
    uint16_t ip;
    uint16_t cs;
    uint16_t lfarlc;
    uint16_t ovno;
    uint16_t res[4];
    uint16_t oemid;
    uint16_t oeminfo;
    uint16_t res2[10];
    uint32_t lfanew;
  };
  struct DataDirectory
  {
    uint32_t virtualAddress;
    uint32_t size;
  };
  struct OptionalHeader
  {
    uint16_t magic;
    uint8_t majorLinkerVersion;
    uint8_t minorLinkerVersion;
    uint32_t sizeOfCode;
    uint32_t sizeOfInitData;
    uint32_t sizeOfUninitData;
    uint32_t addressOfEntryPoint;
    uint32_t baseOfCode;
    // uint32_t baseOfData;

    uint64_t imageBase;
    uint32_t sectionAlignment;
    uint32_t fileAlignment;
    uint16_t majorOSVersion;
    uint16_t minorOSVersion;
    uint16_t majorImageVersion;
    uint16_t minorImageVersion;
    uint16_t majorSubsystemVersion;
    uint16_t minorSubsystemVersion;
    uint32_t win32VersionValue;
    uint32_t sizeOfImage;
    uint32_t sizeOfHeaders;
    uint32_t checksum;
    uint16_t subsystem;
    uint16_t dllCharacteristics;
    uint64_t sizeOfStackReserve;
    uint64_t sizeOfStackCommit;
    uint64_t sizeOfHeapReserve;
    uint64_t sizeOfHeapCommit;
    uint32_t loaderFlags;
    uint32_t numRvaAndSizes;
    DataDirectory dataDirectory[16];
  };
  struct FileHeader
  {
    uint16_t machine;
    uint16_t numSections;
    uint32_t timeStamp;
    uint32_t ptrToSymbolTable;
    uint32_t numSymbols;
    uint16_t sizeOfOptionalHeader;
    uint16_t characteristics;
  };
  struct NTHeader
  {
    uint32_t Signature;
    FileHeader fileHeader;
    OptionalHeader optionalHeader;
  };
  struct SectionHeader
  {
    uint8_t name[8];
    union {
        uint32_t physicalAddress;
        uint32_t virtualSize;
    } misc;
    uint32_t virtualAddress;
    uint32_t sizeOfRawData;
    uint32_t ptrToRawData;
    uint32_t ptrToRelocations;
    uint32_t ptrToLinenumbers;
    uint16_t numRelocations;
    uint16_t numLinenumbers;
    uint32_t characteristics;
  };
  struct ImageBaseRelocation
  {
    uint32_t virtualAddress;
    uint32_t sizeOfBlock;
  };
  struct RelocationBase
  {
    ImageBaseRelocation base;
    std::vector<uint16_t> rva;
  };

  struct ImportDescriptor
  {
    union
    {
      uint32_t characteristics;
      uint32_t originalFirstThunk;
    };
    uint32_t timestamp;
    uint32_t forwarderChain;
    uint32_t name;
    uint32_t firstThunk;
  };
  struct ThunkData
  {
    union {
      uint64_t forwarderString;
      uint64_t function;
      uint64_t ordinal;
      uint64_t addressOfData;
    };
  };
}

#endif // PICKC_WINDOWS_X64_EXE_FORMAT_H_