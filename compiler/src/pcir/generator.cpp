/**
 * @file generator.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief generator.hの実装
 * @version 0.1
 * @date 2021-07-24
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "generator.h"

#include <fstream>
#include <filesystem>

#include "fn_generator.h"

#include "config.h"
#include "compiler_option.h"
#include "output_buffer.h"
#include "utils/file_id.h"
#include "utils/binary_vec.h"

namespace pickc::pcir {
  PCIRGenerator::PCIRGenerator(const parser::ModuleTree& rootModule) :
    rootModule(rootModule),
    textSection(_TextSection::create()),
    moduleSection(_ModuleSection::create()),
    typeSection(_TypeSection::create()),
    symbolSection(_SymbolSection::create()),
    fnSection(_FnSection::create()) {}
  bool PCIRGenerator::generate() {
    if(compilerOption.debug) {
      outputBuffer.emplace<OutputMessage>(OutputMessageKind::Debug, "Generate PCIR... ");
    }

    // TODO: read pcir-libs from compilerOption::libraries

    if(!declare("", rootModule)) {
      return false;
    }

    if(!generate(rootModule)) {
      return false;
    }

    if(compilerOption.debug) {
      outputBuffer.emplace<OutputMessage>(OutputMessageKind::Debug, "PCIR generation has been completed.");
    }

    return true;
  }
  bool PCIRGenerator::writeToFile(const std::string& filename) {
    BinaryVec bin;
    bin << "PCIR";
    bin << MAJOR_VERSION << MINOR_VERSION;
    bin << static_cast<uint64_t>(std::time(nullptr));
    for(auto byte : genFileID()) {
      bin << byte;
    }
    auto pointerToBase = bin.size();
    // PointerToXXXSection用の領域
    bin.resize(pointerToBase + 4 * 5);

    auto textBin = textSection->toBinaryVec();
    auto moduleBin = moduleSection->toBinaryVec();
    auto typeBin = typeSection->toBinaryVec();
    auto symbolBin = symbolSection->toBinaryVec();
    auto fnBin = fnSection->toBinaryVec();

    write(bin, pointerToBase, static_cast<uint32_t>(bin.size()));
    bin << textBin;

    write(bin, pointerToBase + 4, static_cast<uint32_t>(bin.size()));
    bin << moduleBin;

    write(bin, pointerToBase + 8, static_cast<uint32_t>(bin.size()));
    bin << typeBin;

    write(bin, pointerToBase + 12, static_cast<uint32_t>(bin.size()));
    bin << symbolBin;

    write(bin, pointerToBase + 16, static_cast<uint32_t>(bin.size()));
    bin << fnBin;

    std::ofstream stream(filename, std::ios::binary);
    if(!stream) {
      outputBuffer.emplace<OutputMessage>(OutputMessageKind::Error, "ファイルを開けません: " + filename);
      return false;
    }

    stream.write((char*)bin.data(), bin.size());

    if(compilerOption.debug) {
      outputBuffer.emplace<OutputMessage>(OutputMessageKind::Debug, "Writing PCIR to " + std::filesystem::absolute(filename).string() + " is complete.");
    }

    return true;
  }
  bool PCIRGenerator::declare(const std::string& parentName, const parser::ModuleTree& moduleTree) {
    bool suc = true;

    auto module = moduleSection->createModule(
      textSection->createText(moduleTree->getName()),
      textSection->createText(parentName)
    );
    if(!moduleTree->isDirectoryModule()) {
      for(const auto& fn : moduleTree->getRootNode()->functions) {
        FnGenerator fnGen(this, fn);
        auto name = textSection->createText(fn->name);
        auto symbol = symbolSection->createSymbol(
          name,
          fnGen.declare(),
          std::nullopt
        );
        module->addSymbol(name, symbol);
      }
    }

    for(const auto& sub : *moduleTree) {
      if(!declare(moduleTree->getFullyQualifiedName(), sub)) suc = false;
    }

    return suc;
  }
  bool PCIRGenerator::generate(const parser::ModuleTree& moduleTree) {
    bool suc = true;

    auto module = moduleSection->getModule(moduleTree->getFullyQualifiedName());
    if(!moduleTree->isDirectoryModule()) {
      for(const auto& fnNode : moduleTree->getRootNode()->functions) {
        FnGenerator fnGen(this, fnNode);
        auto symbol = module->findSymbol(textSection->createText(fnNode->name));
        auto fn = fnSection->createFn(symbol->getType());
        fnGen.generate(fn);

        auto initFn = fnSection->createFn(
          typeSection->createType(FnType(
            symbol->getType(),
            {}
          ))
        );
        auto entry = initFn->getEntryBlock();
        auto fnVar = initFn->addVariable(symbol->getType());
        entry->emplace<LoadFnOperator>(fnVar, fn);
        entry->toTerminalBlock(TerminalBlock(fnVar));
        symbol->setInitFn(initFn);
      }
    }

    for(const auto& sub : *moduleTree) {
      if(!generate(sub)) suc = false;
    }

    return suc;
  }
}