#include "exe.h"

#include <ctime>
#include <sstream>

namespace pick::exe
{
    uint32_t alignment(uint32_t value, uint32_t alignment)
    {
        return (value + alignment - 1) / alignment * alignment;
    }
    Result<_, std::vector<std::string>> Linker::resolveRelocations()
    {
        std::vector<std::string> errors;

        const uint64_t rdataBase =
            ntHeader.optionalHeader.imageBase
            + ntHeader.optionalHeader.addressOfEntryPoint
            + alignment(ntHeader.optionalHeader.sizeOfCode, ntHeader.optionalHeader.sectionAlignment);

        struct DLL
        {
            uint32_t index;
            uint32_t nameIndex;
            uint32_t thunkIndex;
            std::map<std::string, uint64_t> symbols;
        };
        std::map<std::string, DLL> dlls;
        for (const auto& ext : x86_64.exts) {
            if (libSymbols.find(ext) == libSymbols.end()) {
                std::stringstream ss;
                ss << "リンクエラー: シンボル " << ext << " が見つかりませんでした。";
                errors += ss.str();
            }
            dlls[libSymbols[ext].header.name].symbols.insert({ ext, 0 });
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

        for (auto& symbol : x86_64.constSymbols) {
            symbol.address = address;
            switch (symbol.type) {
            case x86_64::ConstantSymbolType::Immediate:
                rdataSectionRawData += symbol.imm;
                address += symbol.imm.size();
                break;
            case x86_64::ConstantSymbolType::Relocation:
                switch (symbol.reloc.type) {
                case x86_64::RelocationType::Function:
                    rdataSectionRawData << x86_64.routines[symbol.reloc.indexOfData].address;
                    address += sizeof(x86_64.routines[symbol.reloc.indexOfData].address);
                    break;
                case x86_64::RelocationType::ConstantSymbol:
                    assert(false);
                    break;
                case x86_64::RelocationType::RuntimeSymbol:
                    assert(false);
                    break;
                case x86_64::RelocationType::Extern:
                    // この段階では何もしない。
                    break;
                default:
                    assert(false);
                }
                break;
            default:
                assert(false);
            }
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
                        rdataSectionRawData += c;
                    }
                    rdataSectionRawData += '\0';
                    if (symbol.first.size() % 2 == 0) rdataSectionRawData += '\0';
                }
                for (auto c : dll.first) {
                    rdataSectionRawData += c;
                }
                rdataSectionRawData += '\0';
                if (dll.first.size() % 2 == 0) rdataSectionRawData += '\0';
            }
        }

        ntHeader.optionalHeader.sizeOfInitData = rdataSectionRawData.size();
        rdataSection.misc.virtualSize = ntHeader.optionalHeader.sizeOfInitData;
        rdataSection.sizeOfRawData = alignment(rdataSection.misc.virtualSize, ntHeader.optionalHeader.fileAlignment);
        rdataSection.ptrToRawData = textSection.ptrToRawData + textSection.sizeOfRawData;
        rdataSection.virtualAddress = textSection.virtualAddress + alignment(textSection.sizeOfRawData, ntHeader.optionalHeader.sectionAlignment);
        ntHeader.optionalHeader.sizeOfInitData = rdataSection.sizeOfRawData;

        if (x86_64.runtimeSymbols.empty()) {
            ntHeader.optionalHeader.sizeOfUninitData = 0;
        }
        else {
            address = ntHeader.optionalHeader.imageBase + ntHeader.optionalHeader.addressOfEntryPoint + alignment(ntHeader.optionalHeader.sizeOfCode, ntHeader.optionalHeader.sectionAlignment) + alignment(rdataSection.sizeOfRawData, ntHeader.optionalHeader.sectionAlignment);
            for (auto& symbol : x86_64.runtimeSymbols) {
                symbol.address = address;
                address += symbol.sizeOfSymbol;
                ntHeader.optionalHeader.sizeOfUninitData += static_cast<uint32_t>(symbol.sizeOfSymbol);
            }
            dataSection.misc.virtualSize = ntHeader.optionalHeader.sizeOfUninitData;
            dataSection.sizeOfRawData = alignment(dataSection.misc.virtualSize, ntHeader.optionalHeader.fileAlignment);
            dataSection.ptrToRawData = rdataSection.ptrToRawData + rdataSection.sizeOfRawData;
            dataSection.virtualAddress = rdataSection.virtualAddress + alignment(rdataSection.sizeOfRawData, ntHeader.optionalHeader.sectionAlignment);
            ntHeader.optionalHeader.sizeOfUninitData = dataSection.sizeOfRawData;
        }

        if (errors.empty()) return ok();
        else return error(errors);
    }
    Linker::Linker(const x86_64::X86_64& x86_64) :
        x86_64(x86_64),
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
            0x00004550,                                 // Signature
            FileHeader{
                0x8664,                                 // Machine (x86)
                0x0002,                                 // Number of sections
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
        },
        relocTextSection{
            ImageBaseRelocation{
                0x00001000,
                sizeof(ImageBaseRelocation)
            }
        }
    {}
    Result<_, std::vector<std::string>> Linker::link(const std::string& mainModuleName, const std::vector<std::string>& libs)
    {
        std::vector<std::string> errors;

        for (const auto& lib : libs) {
            LibLoader loader(lib);
            auto res = loader.load();
            if (!res) errors += res.err();
            else libSymbols.merge(res.get());
        }

        ntHeader.fileHeader.numSections = 3;
        if (!x86_64.runtimeSymbols.empty()) ++ntHeader.fileHeader.numSections;
        ntHeader.optionalHeader.sizeOfHeaders = alignment(sizeof(DOSHeader) + sizeof(stub) + sizeof(NTHeader) + sizeof(SectionHeader) * ntHeader.fileHeader.numSections, ntHeader.optionalHeader.fileAlignment);

        uint64_t address = ntHeader.optionalHeader.imageBase + ntHeader.optionalHeader.addressOfEntryPoint;
        x86_64.invokeMain.address = address;
        address += static_cast<uint32_t>(x86_64.invokeMain.code.size());
        ntHeader.optionalHeader.sizeOfCode = static_cast<uint32_t>(x86_64.invokeMain.code.size());
        for (auto& routine : x86_64.routines) {
            routine.address = address;
            address += routine.code.size();
            ntHeader.optionalHeader.sizeOfCode += static_cast<uint32_t>(routine.code.size());
        }
        textSection.misc.virtualSize = ntHeader.optionalHeader.sizeOfCode;
        textSection.sizeOfRawData = alignment(textSection.misc.virtualSize, ntHeader.optionalHeader.fileAlignment);
        textSection.ptrToRawData = ntHeader.optionalHeader.sizeOfHeaders;
        textSection.virtualAddress = ntHeader.optionalHeader.addressOfEntryPoint;

        ntHeader.optionalHeader.sizeOfCode = textSection.sizeOfRawData;

        auto res = resolveRelocations();
        if (!res) errors += res.err();

        for (auto& reloc : x86_64.invokeMain.relocs) {
            uint64_t address = 0;
            switch (reloc.type) {
            case x86_64::RelocationType::Function:
                address = x86_64.routines[reloc.indexOfData].address - reloc.indexOfRoutine - 4 - (ntHeader.optionalHeader.imageBase + ntHeader.optionalHeader.addressOfEntryPoint);
                break;
            case x86_64::RelocationType::ConstantSymbol:
                address = x86_64.constSymbols[reloc.indexOfData].address;
                relocTextSection.base.sizeOfBlock += 2;
                relocTextSection.rva.push_back(0x3000 | (reloc.indexOfRoutine & 0x0FFF));
                break;
            case x86_64::RelocationType::RuntimeSymbol:
                address = x86_64.runtimeSymbols[reloc.indexOfData].address;
                relocTextSection.base.sizeOfBlock += 2;
                relocTextSection.rva.push_back(0x3000 | (reloc.indexOfRoutine & 0x0FFF));
                break;
            default:
                assert(false);
            }
            x86_64.invokeMain.code[reloc.indexOfRoutine] = static_cast<uint8_t>(address);
            x86_64.invokeMain.code[reloc.indexOfRoutine + 1] = static_cast<uint8_t>(address >> 8);
            x86_64.invokeMain.code[reloc.indexOfRoutine + 2] = static_cast<uint8_t>(address >> 16);
            x86_64.invokeMain.code[reloc.indexOfRoutine + 3] = static_cast<uint8_t>(address >> 24);
        }
        uint64_t base = x86_64.invokeMain.code.size();
        for (auto& routine : x86_64.routines) {
            for (auto& reloc : routine.relocs) {
                uint64_t address = 0;
                switch (reloc.type) {
                case x86_64::RelocationType::Function:
                    address = x86_64.routines[reloc.indexOfData].address - routine.address - reloc.indexOfRoutine - 4;
                    break;
                case x86_64::RelocationType::ConstantSymbol:
                    address = x86_64.constSymbols[reloc.indexOfData].address;
                    relocTextSection.base.sizeOfBlock += 2;
                    relocTextSection.rva.push_back(0x3000 | ((base + reloc.indexOfRoutine) & 0x0FFF));
                    break;
                case x86_64::RelocationType::RuntimeSymbol:
                    address = x86_64.runtimeSymbols[reloc.indexOfData].address;
                    relocTextSection.base.sizeOfBlock += 2;
                    relocTextSection.rva.push_back(0x3000 | ((base + reloc.indexOfRoutine) & 0x0FFF));
                    break;
                case x86_64::RelocationType::Extern:
                    address = libSymbols[reloc.ext].address - routine.address - reloc.indexOfRoutine - 4;
                    break;
                default:
                    assert(false);
                }
                routine.code[reloc.indexOfRoutine] = static_cast<uint8_t>(address);
                routine.code[reloc.indexOfRoutine + 1] = static_cast<uint8_t>(address >> 8);
                routine.code[reloc.indexOfRoutine + 2] = static_cast<uint8_t>(address >> 16);
                routine.code[reloc.indexOfRoutine + 3] = static_cast<uint8_t>(address >> 24);
            }
            base += routine.code.size();
        }

        relocSection.misc.virtualSize = relocTextSection.base.sizeOfBlock;
        relocSection.sizeOfRawData = alignment(relocSection.misc.virtualSize, ntHeader.optionalHeader.fileAlignment);
        if (x86_64.runtimeSymbols.empty()) {
            relocSection.ptrToRawData = rdataSection.ptrToRawData + rdataSection.sizeOfRawData;
            relocSection.virtualAddress = rdataSection.virtualAddress + alignment(relocSection.sizeOfRawData, ntHeader.optionalHeader.sectionAlignment);
        }
        else {
            relocSection.ptrToRawData = dataSection.ptrToRawData + dataSection.sizeOfRawData;
            relocSection.virtualAddress = dataSection.virtualAddress + alignment(relocSection.sizeOfRawData, ntHeader.optionalHeader.sectionAlignment);
        }

        ntHeader.optionalHeader.sizeOfInitData += relocSection.sizeOfRawData;

        ntHeader.optionalHeader.dataDirectory[5].size = relocSection.misc.virtualSize;
        ntHeader.optionalHeader.dataDirectory[5].virtualAddress = relocSection.virtualAddress;

        ntHeader.optionalHeader.baseOfCode = textSection.virtualAddress;

        ntHeader.optionalHeader.sizeOfImage = ntHeader.optionalHeader.addressOfEntryPoint + ntHeader.optionalHeader.sectionAlignment * ntHeader.fileHeader.numSections;
    
        if (errors.empty()) return ok();
        else return error(errors);
    }
    Result<_, std::vector<std::string>> Linker::write(const std::string& outFilePath)
    {
        stream.open(outFilePath, std::ios::binary);
        if (!stream) {
            std::stringstream ss;
            ss << "エラー: ファイルが開けませんでした。ファイル名: " << outFilePath;
            return error(std::vector({ ss.str() }));
        }

        stream.write((char*)&dosHeader, sizeof(dosHeader));
        stream.write((char*)stub, sizeof(stub));
        stream.write((char*)&ntHeader, sizeof(ntHeader));
        stream.write((char*)&textSection, sizeof(textSection));
        stream.write((char*)&rdataSection, sizeof(rdataSection));
        if (!x86_64.runtimeSymbols.empty()) {
            stream.write((char*)&dataSection, sizeof(dataSection));
        }
        stream.write((char*)&relocSection, sizeof(relocSection));

        auto pos = sizeof(DOSHeader) + sizeof(stub) + sizeof(NTHeader) + sizeof(SectionHeader) * ntHeader.fileHeader.numSections;
        if (pos % ntHeader.optionalHeader.fileAlignment) {
            auto s = ntHeader.optionalHeader.fileAlignment - pos % ntHeader.optionalHeader.fileAlignment;
            char* b = new char[s];
            memset(b, 0x00, s);
            stream.write(b, s);
            delete[] b;
        }

        stream.write((char*)x86_64.invokeMain.code.data(), x86_64.invokeMain.code.size());
        for (const auto& routine : x86_64.routines) {
            stream.write((char*)routine.code.data(), routine.code.size());
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

        if (!x86_64.runtimeSymbols.empty()) {
            b = new char[dataSection.sizeOfRawData];
            memset(b, 0x00, dataSection.sizeOfRawData);
            stream.write(b, dataSection.sizeOfRawData);
            delete[] b;
        }

        stream.write((char*)&relocTextSection.base, sizeof(ImageBaseRelocation));
        stream.write((char*)relocTextSection.rva.data(), relocTextSection.rva.size() * 2);

        b = new char[relocSection.sizeOfRawData - relocSection.misc.virtualSize];
        memset(b, 0x00, relocSection.sizeOfRawData - relocSection.misc.virtualSize);
        stream.write(b, relocSection.sizeOfRawData - relocSection.misc.virtualSize);
        delete[] b;

        stream.close();

        return ok();
    }

    inline size_t sstrlen(char* str, size_t max)
    {
        for (size_t i = 0; i < max; ++i) {
            if (str[i] == ' ') return i;
        }
        return max;
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
            std::stringstream ss;
            ss << "エラー: ファイルを開けませんでした。ファイル名: " << path;
            return error(std::vector({ ss.str() }));
        }
        std::string magic;
        std::getline(stream, magic);
        if (magic == "!<arch>") {
            ArchiveMemberHeader first;
            loadArchiveMemberHeader(first);
            if (first.name != "/") {
                std::stringstream ss;
                ss << "エラー: 正しいlibファイルではありません。第一リンカメンバがありませんでした。ファイル名: " << path;
                return error(std::vector({ ss.str() }));
            }
            stream.seekg(first.size, std::ios::cur);
            ArchiveMemberHeader second;
            loadArchiveMemberHeader(second);
            if (second.name != "/") {
                std::stringstream ss;
                ss << "エラー: 正しいlibファイルではありません。第二リンカメンバがありませんでした。ファイル名: " << path;
                return error(std::vector({ ss.str() }));
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
            return symbols;
        }
        std::stringstream ss;
        ss << "エラー: 正しいlibファイルではありません。libファイルは!<arch>から始まる形式である必要があります。ファイル名: " << path;
        return error(std::vector({ ss.str() }));
    }
}