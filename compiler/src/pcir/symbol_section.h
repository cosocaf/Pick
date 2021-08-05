#ifndef PICKC_PCIR_SYMBOL_SECTION_H_
#define PICKC_PCIR_SYMBOL_SECTION_H_

#include <memory>
#include <vector>
#include <optional>

#include "text_section.h"
#include "type_section.h"
#include "fn_section.h"
#include "mutability.h"

#include "utils/binary_vec.h"

namespace pickc::pcir {
  class _SymbolSection;
  class _Symbol;
  using SymbolSection = std::shared_ptr<_SymbolSection>;
  using WeakSymbolSection = std::weak_ptr<_SymbolSection>;
  using Symbol = std::shared_ptr<_Symbol>;
  using WeakSymbol = std::weak_ptr<_Symbol>;

  class _Symbol {
    WeakSymbolSection symbolSection;
    uint32_t index;
    Text name;
    Type type;
    Mutability mutability;
    std::optional<Fn> initFn;
  public:
    _Symbol(const WeakSymbolSection& symbolSection, uint32_t index, const Text& name, const Type& type, const std::optional<Fn>& initFn);
    Text getName() const;
    Type getType() const;
    Mutability getMutability() const;
    uint32_t getIndex() const;
    std::optional<Fn> getInitFn() const;
    void setInitFn(const Fn& fn);
  };

  class _SymbolSection : public std::enable_shared_from_this<_SymbolSection> {
    std::vector<Symbol> symbols;
  private:
    _SymbolSection() = default;
  public:
    static SymbolSection create();
    Symbol createSymbol(const Text& name, const Type& type, const std::optional<Fn>& initFn);
    BinaryVec toBinaryVec() const;
  };
}

#endif // PICKC_PCIR_SYMBOL_SECTION_H_