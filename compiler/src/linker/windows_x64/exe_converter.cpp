/**
 * @file exe_converter.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-10-01
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "exe_converter.h"

namespace pickc::linker::windows_x64 {
  namespace {
    uint32_t alignment(uint32_t value, uint32_t alignment) {
      return (value + alignment - 1) / alignment * alignment;
    }
  }
  EXEConverter::EXEConverter(const std::map<std::array<uint8_t, 16>, std::vector<Routine>>& routines) : routines(routines), dosHeader(), stub(), ntHeader() {
    init();
  }
  void EXEConverter::init() {
    dosHeader.magic = 0x5A4D;
    dosHeader.cblp = 0x0090;
    dosHeader.cp = 0x0003;
    dosHeader.crlc = 0x0000;
    dosHeader.cparhdr = 0x0004;
    dosHeader.minalloc = 0x0000;
    dosHeader.maxalloc = 0xFFFF;
    dosHeader.ss = 0x0000;
    dosHeader.sp = 0x00B8;
    dosHeader.csum = 0x0000;
    dosHeader.ip = 0x0000;
    dosHeader.cs = 0x0000;
    dosHeader.lfarlc = 0x0040;
    dosHeader.ovno = 0x0000;
    dosHeader.oemid = 0x0000;
    dosHeader.oeminfo = 0x0000;
    dosHeader.lfanew = 0x48;

    // mov ax 0x014C [al: return code, ah: syscall return]
    stub[0] = 0xB8;
    stub[1] = 0x01;
    stub[2] = 0x4C; 

    // system call
    stub[3] = 0xCD;
    stub[4] = 0x21;

    // alignment (nop)
    stub[5] = 0x90;
    stub[6] = 0x90;
    stub[7] = 0x90;

    ntHeader.signature = 0x00004550;

    ntHeader.fileHeader.machine = 0x8664;
    ntHeader.fileHeader.numSections = 0x0004;
    ntHeader.fileHeader.timeStamp = (uint32_t)std::time(nullptr);
    ntHeader.fileHeader.ptrToSymbolTable = 0x00000000;
    ntHeader.fileHeader.numSymbols = 0x00000000;
    ntHeader.fileHeader.sizeOfOptionalHeader = static_cast<uint16_t>(sizeof(OptionalHeader));
    // Executable
    // Application can handle large (>2GB) addresses
    ntHeader.fileHeader.characteristics = 0x0022;
    
    // PE32+
    ntHeader.optionalHeader.magic = 0x020B;
    ntHeader.optionalHeader.majorLinkerVersion = 0x00;
    ntHeader.optionalHeader.minorLinkerVersion = 0x00;
    ntHeader.optionalHeader.sizeOfCode = 0x00000000;
    ntHeader.optionalHeader.sizeOfInitData = 0x00000000;
    ntHeader.optionalHeader.sizeOfUninitData = 0x00000000;
    ntHeader.optionalHeader.addressOfEntryPoint = 0x00001000;
    ntHeader.optionalHeader.baseOfCode = 0x00000000;
    ntHeader.optionalHeader.imageBase = 0x00400000;
    ntHeader.optionalHeader.sectionAlignment = 0x00001000;
    ntHeader.optionalHeader.fileAlignment = 0x00000200;
    ntHeader.optionalHeader.majorOSVersion = 0x0006;
    ntHeader.optionalHeader.minorOSVersion = 0x0000;
    ntHeader.optionalHeader.majorImageVersion = 0x0000;
    ntHeader.optionalHeader.minorImageVersion = 0x0000;
    ntHeader.optionalHeader.majorSubsystemVersion = 0x0006;
    ntHeader.optionalHeader.minorSubsystemVersion = 0x0000;
    ntHeader.optionalHeader.win32VersionValue = 0x00000000;
    ntHeader.optionalHeader.sizeOfImage = 0x00000000;
    ntHeader.optionalHeader.sizeOfHeaders = 0x00000000;
    ntHeader.optionalHeader.checksum = 0x00000000;
    // /SUBSYSTEM:CONSOLE
    ntHeader.optionalHeader.subsystem = 0x0003;
    // Dynamic base
    // NX compatible
    // No structured exception handler
    // Terminal Server Aware
    ntHeader.optionalHeader.dllCharacteristics = 0x8540;
    ntHeader.optionalHeader.sizeOfStackReserve = 0x00100000;
    ntHeader.optionalHeader.sizeOfStackCommit = 0x00001000;
    ntHeader.optionalHeader.sizeOfHeapReserve = 0x00100000;
    ntHeader.optionalHeader.sizeOfHeapCommit = 0x00001000;
    ntHeader.optionalHeader.loaderFlags = 0x00000000;
    ntHeader.optionalHeader.numRvaAndSizes = 0x00000010;

    textSection = {
      ".text\0\0",
      0,
      0,
      0,
      0,
      0, 0, 0, 0,
      // Code
      // Execute Read
      0x60000020
    },
    rdataSection = {
      ".rdata\0",
      0,
      0,
      0,
      0,
      0, 0, 0, 0,
      // Initialized Data
      // Read Only
      0x40000040
    },
    dataSection = {
      ".data\0\0",
      0,
      0,
      0,
      0,
      0, 0, 0, 0,
      // Uninitialized Data
      // Read Write
      0xC0000080
    },
    relocSection = {
      ".reloc\0",
      0,
      0,
      0,
      0,
      0, 0, 0, 0,
      // Initialized Data
      // Discardable
      // Read Only
      0x42000040
    };
  }
  BinaryVec EXEConverter::convert() {
    ntHeader.fileHeader.numSections = 1;
    ntHeader.optionalHeader.sizeOfHeaders = alignment(sizeof(DOSHeader) + sizeof(stub) + sizeof(NTHeader) + sizeof(SectionHeader) * ntHeader.fileHeader.numSections, ntHeader.optionalHeader.fileAlignment);

    placeTextSection();
    placeRDataSection();
    placeDataSection();
    placeRelocSection();

    ntHeader.optionalHeader.baseOfCode = textSection.virtualAddress;

    ntHeader.optionalHeader.sizeOfImage = ntHeader.optionalHeader.addressOfEntryPoint + ntHeader.optionalHeader.sectionAlignment * ntHeader.fileHeader.numSections;

    BinaryVec bin;

    bin << dosHeader
        << stub
        << ntHeader
        << textSection;
        // << rdataSection
        // << dataSection
        // << relocSection;
    
    align(bin, ntHeader.optionalHeader.fileAlignment);

    for(const auto& [fileID, routine] : routines) {
      for(const auto& r : routine) {
        bin << r.bin;
      }
    }
    align(bin, ntHeader.optionalHeader.fileAlignment);

    return bin;
  }
  void EXEConverter::placeTextSection() {
    uint64_t address = ntHeader.optionalHeader.imageBase + ntHeader.optionalHeader.addressOfEntryPoint;
    uint32_t sizeOfCode = 0;
    for(auto& [fileID, routine] : routines) {
      for(auto& r : routine) {
        r.address = address;
        address += r.bin.size();
        sizeOfCode += static_cast<uint32_t>(r.bin.size());
      }
    }

    textSection.misc.virtualSize = sizeOfCode;
    textSection.sizeOfRawData = alignment(sizeOfCode, ntHeader.optionalHeader.fileAlignment);
    textSection.ptrToRawData = ntHeader.optionalHeader.sizeOfHeaders;
    textSection.virtualAddress = ntHeader.optionalHeader.addressOfEntryPoint;

    ntHeader.optionalHeader.sizeOfCode = textSection.sizeOfRawData;
  }
  void EXEConverter::placeRDataSection() {
    uint64_t address = ntHeader.optionalHeader.imageBase;
  }
  void EXEConverter::placeDataSection() {

  }
  void EXEConverter::placeRelocSection() {

  }
}