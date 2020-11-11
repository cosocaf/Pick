#include "compiler.h"

#include <filesystem>

#include "utils/vector_utils.h"
#include "pcir/pcir_format.h"

#include "routine_compiler.h"

namespace pickc::windows::x64
{
  Compiler::Compiler(const bundler::Bundle& bundle) : bundle(bundle) {}
  Result<WindowsX64, std::vector<std::string>> Compiler::compile(const CompilerOption& option)
  {
    std::vector<std::string> errors;
    
    for(auto& fn : bundle.fns) {
      if(fn.second->fnType == pcir::FN_TYPE_FUNCTION) {
        if(auto res = RoutineCompiler(fn.second, &x64).compile()) {
          x64.routines[fn.first] = res.get();
        }
        else {
          errors += res.err();
        }
      }
      else {
        x64.externs[fn.first] = fn.second->externName->text;
        auto routine = new Routine();
        routine->code.push_back(new ExtJmpOperation(x64.externs[fn.first]));
        x64.routines[fn.first] = routine;
      }
    }

    pcir::SymbolSection* mainSymbol = nullptr;
    for(const auto& symbol : bundle.modules[option.mainModule]) {
      if(symbol->name->text == "main") {
        mainSymbol = symbol;
        break;
      }
    }

    if(mainSymbol == nullptr) {
      errors.push_back("エラー: mainシンボルが見つかりません。正しくメインモジュールが指定されていない可能性があります。");
    }

    x64.invokeMain = new Routine();
    x64.invokeMain->code.push_back(new PushOperation(Operand(Register::RBP)));
    x64.invokeMain->code.push_back(new MovOperation(OperationSize::QWord, Operand(Register::RBP), Operand(Register::RSP)));
    for(auto& symbol : bundle.symbols) {
      x64.symbols[symbol.first] = 0;
      x64.invokeMain->code.push_back(new CallOperation(x64.invokeMain, Operand(Relocation(symbol.first->init))));
      x64.invokeMain->code.push_back(new MovOperation(OperationSize::QWord, Operand(Register::RCX), Operand(Relocation(symbol.first))));
      auto size = x64.typeTable[symbol.first->type].getSize();
      switch(size) {
        case 8:
          x64.invokeMain->code.push_back(new MovOperation(OperationSize::Byte, Operand(Memory(Register::RCX, 1, true)), Operand(Register::RAX)));
          break;
        case 16:
          x64.invokeMain->code.push_back(new MovOperation(OperationSize::Word, Operand(Memory(Register::RCX, 2, true)), Operand(Register::RAX)));
          break;
        case 32:
          x64.invokeMain->code.push_back(new MovOperation(OperationSize::DWord, Operand(Memory(Register::RCX, 4, true)), Operand(Register::RAX)));
          break;
        case 64:
          x64.invokeMain->code.push_back(new MovOperation(OperationSize::QWord, Operand(Memory(Register::RCX, 8, true)), Operand(Register::RAX)));
          break;
        default:
          assert(false);
      }
    }
    x64.invokeMain->code.push_back(new MovOperation(OperationSize::QWord, Operand(Register::RAX), Operand(Relocation(mainSymbol))));
    x64.invokeMain->code.push_back(new MovOperation(OperationSize::QWord, Operand(Register::RAX), Operand(Memory(Register::RAX, 8, true))));
    x64.invokeMain->code.push_back(new CallOperation(x64.invokeMain, Operand(Register::RAX)));
    x64.invokeMain->code.push_back(new LeaveOperation());
    x64.invokeMain->code.push_back(new RetOperation());
    x64.routines[nullptr] = x64.invokeMain;
    if(errors.empty()) return ok(x64);
    return error(errors);
  }
}