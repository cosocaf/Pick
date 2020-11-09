#include "bundler.h"

#include <filesystem>

#include "utils/vector_utils.h"

#include "fn_compiler.h"

namespace pickc::bundler
{
  Bundler::Bundler() {}
  Result<Bundle, std::vector<std::string>> Bundler::bundle(const CompilerOption& option)
  {
    std::vector<std::string> errors;

    auto path = std::filesystem::path(option.outDir) / (option.out + ".pcir");
    auto res = pcir::PCIRLoader().load(path.string());
    if(!res) errors += res.err();
    pcirs.push_back(res.get());
    for(const auto& lib : option.libraries) {
      // TODO: Load PCIRs
    }

    if(!errors.empty()) return error(errors);

    for(const auto& pcir : pcirs) {
      for(const auto& module : pcir.moduleSection) {
        for(const auto& symbol : module->symbols) {
          b.symbols[symbol] = new Symbol(symbol->init);
          b.modules[module->name->text].insert(symbol);
          b.symbolNames[module->name->text + "::" + symbol->name->text] = symbol;
        }
      }
    }

    for(auto& pcir : pcirs) {
      for(auto& fn : pcir.fnSection) {
        if(auto res = FnCompiler(&b, &pcir, fn).compile()) {
          b.fns[fn] = res.get();
        }
        else {
          errors += res.err();
        }
      }
    }

    return ok(b);
  }
}