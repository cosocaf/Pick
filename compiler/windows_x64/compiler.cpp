#include "compiler.h"

#include <filesystem>

#include "utils/vector_utils.h"

#include "routine_compiler.h"

namespace pickc::windows::x64
{
  Compiler::Compiler(const ssa::SSABundle& bundle) : bundle(bundle) {}
  Result<WindowsX64, std::vector<std::string>> Compiler::compile(const CompilerOption& option)
  {
    std::vector<std::string> errors;
    
    for(auto& fn : bundle.fns) {
      if(auto res = RoutineCompiler(fn.second).compile()) {
        x64.routines[fn.first] = res.get();
      }
      else {
        errors += res.err();
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
      x64.symbols[symbol] = 0;
      x64.invokeMain->code.push_back(new CallOperation(x64.invokeMain, Operand(Relocation(symbol->init))));
      x64.invokeMain->code.push_back(new MovOperation(OperationSize::QWord, Operand(Register::RCX), Operand(Relocation(symbol))));
      auto size = x64.typeTable[symbol->type].getSize();
      switch(size) {
        case 8:
          x64.invokeMain->code.push_back(new MovOperation(OperationSize::Byte, Operand(Memory(Register::RCX)), Operand(Register::RAX)));
          break;
        case 16:
          x64.invokeMain->code.push_back(new MovOperation(OperationSize::Word, Operand(Memory(Register::RCX)), Operand(Register::RAX)));
          break;
        case 32:
          x64.invokeMain->code.push_back(new MovOperation(OperationSize::DWord, Operand(Memory(Register::RCX)), Operand(Register::RAX)));
          break;
        case 64:
          x64.invokeMain->code.push_back(new MovOperation(OperationSize::QWord, Operand(Memory(Register::RCX)), Operand(Register::RAX)));
          break;
        default:
          assert(false);
      }
    }
    x64.invokeMain->code.push_back(new MovOperation(OperationSize::QWord, Operand(Register::RAX), Operand(Relocation(mainSymbol))));
    x64.invokeMain->code.push_back(new MovOperation(OperationSize::QWord, Operand(Register::RAX), Operand(Memory(Register::RAX))));
    x64.invokeMain->code.push_back(new CallOperation(x64.invokeMain, Operand(Register::RAX)));
    x64.invokeMain->code.push_back(new LeaveOperation());
    x64.invokeMain->code.push_back(new RetOperation());
    x64.routines[nullptr] = x64.invokeMain;
    if(errors.empty()) return ok(x64);
    return error(errors);
  }
}