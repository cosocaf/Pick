/**
 * @file linker.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-08-06
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "linker.h"

#include <fstream>

#include "routine_converter.h"
#include "windows_x64/routine_compiler.h"
#include "windows_x64/exe_converter.h"

#include "bundler/bundler.h"

#include "compiler_option.h"

namespace pickc::linker {
  Linker::Linker(const bundler::Bundle& bundle) : bundle(bundle) {}
  bool Linker::link() {
    for(const auto& [fileID, functions] : bundle->getFunctions()) {
      for(const auto& function : functions) {
        if(std::holds_alternative<bundler::InternalFunction>(function.variant)) {
          auto fn = std::get<bundler::InternalFunction>(function.variant);
          RoutineConverter routineConverter(bundle, fn);
          auto routine = routineConverter.convert();
          switch(compilerOption.target) {
            case TargetPlatforms::WindowsX64: {
              windows_x64::RoutineCompiler routineCompiler(routine);
              routine.bin = routineCompiler.compile();
            }
          }
          routines[fileID].push_back(std::move(routine));
        }
        else {
          auto fn = std::get<bundler::ExternalFunction>(function.variant);
        }
      }
    }

    return true;
  }
  bool Linker::writeToFile(const std::string& filename) {
    std::ofstream stream(filename);
    if(!stream) {
      // TODO: return error.
      assert(false);
      return false;
    }

    BinaryVec bin;
    switch(compilerOption.target) {
      case TargetPlatforms::WindowsX64: {
        windows_x64::EXEConverter converter(routines);
        bin = converter.convert();
        break;
      }
      default:
        // TODO
        assert(false);
    }

    stream.write((char*)bin.data(), bin.size());

    return true;
  }
}