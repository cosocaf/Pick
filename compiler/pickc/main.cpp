#include <iostream>

#include "config.h"
#include "compiler_option.h"
#include "parser/parser.h"
#include "pcir/semantic_analyzer.h"
#include "bundler/bundler.h"
#include "windows_x64/compiler.h"
#include "windows_x64/linker.h"

int main(char argc, char* argv[])
{
  using namespace pickc;

  #ifndef NDEBUG
  std::cout << CONSOLE_FG_GREEN << VERSION << " Debug Build\n" << std::endl;
  #endif // NDEBUG

  auto option = CompilerOption::create(argc, argv);
  if(!option) {
    std::cout << CONSOLE_FG_RED << option.err() << CONSOLE_DEFAULT << std::endl;
    return STATUS_INVALID_OPTION;
  }
  if(option.get().compilerDebug) {
    std::cout << VERSION << " Debug Mode\n" << std::endl;
    option.get().dump();
    std::cout << std::endl;
  }
  auto moduleTree = parser::Parser().parse(option.get());
  if(!moduleTree) {
    for(const auto& err : moduleTree.err()) {
      std::cout << CONSOLE_FG_RED << err << CONSOLE_DEFAULT << std::endl;
    }
    return STATUS_PARSER_ERROR;
  }
  if(auto errs = pcir::SemanticAnalyzer(moduleTree.get()).write(option.get())) {
    for(const auto& err : errs.get()) {
      std::cout << CONSOLE_FG_RED << err << CONSOLE_DEFAULT << std::endl;
    }
    return STATUS_PCIR_ERROR;
  }
  auto bundle = bundler::Bundler().bundle(option.get());
  if(!bundle) {
    for(const auto& err : bundle.err()) {
      std::cout << CONSOLE_FG_RED << err << CONSOLE_DEFAULT << std::endl;
    }
    return STATUS_BUNDLER_ERROR;
  }
  switch(option.get().target) {
    case TargetPlatforms::WindowsX64: {
      auto x64 = windows::x64::Compiler(bundle.get()).compile(option.get());
      if(!x64) {
        for(const auto& err : x64.err()) {
          std::cout << CONSOLE_FG_RED << err << CONSOLE_DEFAULT << std::endl;
        }
        return STATUS_WINDOWS_X64_ERROR;
      }
      auto res = windows::x64::Linker(x64.get()).link(option.get());
      if(!res) {
        for(const auto& err : res.err()) {
          std::cout << CONSOLE_FG_RED << err << CONSOLE_DEFAULT << std::endl;
        }
        return STATUS_WINDOWS_X64_LINKER_ERROR;
      }
      break;
    }
    default:
      assert(false);
      return STATUS_UNKNOWN_PLATFORM;
  }
  return STATUS_SUCCESS;
}