#include "symbol.h"

#include <cassert>

namespace pickc::bundler
{
  Symbol::Symbol(pcir::FunctionSection* pcirFn) : initType(SymbolInitType::PCIR), pcirFn(pcirFn) {}
  Symbol::Symbol(const BinaryVec& imm) : initType(SymbolInitType::Immediate), imm(imm) {}
  Symbol::Symbol(SymbolInitType type, Function* fn) : initType(type), fn(fn)
  {
    assert(type == SymbolInitType::Function || type == SymbolInitType::Runtime);
  }
  Symbol::Symbol(const Symbol& symbol) : initType(symbol.initType)
  {
    switch(initType) {
      case SymbolInitType::_NONE:
        break;
      case SymbolInitType::PCIR:
        pcirFn = symbol.pcirFn;
        break;
      case SymbolInitType::Immediate:
        new (&imm) BinaryVec(symbol.imm);
        break;
      case SymbolInitType::Function:
      case SymbolInitType::Runtime:
        fn = symbol.fn;
        break;
      default:
        assert(false);
    }
  }
  Symbol::~Symbol()
  {
    clear();
  }
  Symbol& Symbol::operator=(const Symbol& symbol)
  {
    clear();
    initType = symbol.initType;
    switch(initType) {
      case SymbolInitType::_NONE:
        break;
      case SymbolInitType::PCIR:
        pcirFn = symbol.pcirFn;
        break;
      case SymbolInitType::Immediate:
        new (&imm) BinaryVec(symbol.imm);
        break;
      case SymbolInitType::Function:
      case SymbolInitType::Runtime:
        fn = symbol.fn;
        break;
      default:
        assert(false);
    }
    return *this;
  }
  void Symbol::clear()
  {
    switch(initType) {
      case SymbolInitType::_NONE:
        break;
      case SymbolInitType::PCIR:
        break;
      case SymbolInitType::Immediate:
        imm.~vector();
        break;
      case SymbolInitType::Function:
      case SymbolInitType::Runtime:
        break;
    }
    initType = SymbolInitType::_NONE;
  }
}