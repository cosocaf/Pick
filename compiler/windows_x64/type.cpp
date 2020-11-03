#include "type.h"

#include "utils/vector_utils.h"

namespace pickc::windows::x64
{
  TypeTable::TypeTable() {}
  const Type& TypeTable::operator[](pcir::TypeSection* pcirType)
  {
    if(types.find(pcirType) != types.end()) return types[pcirType];
    
    if(includes({
      pcir::Types::I8, pcir::Types::U8,
      pcir::Types::I16, pcir::Types::U16,
      pcir::Types::I32, pcir::Types::U32,
      pcir::Types::I64, pcir::Types::U64,
      pcir::Types::Ptr, pcir::Types::Function,
    }, pcirType->types)) {
      types[pcirType] = Type(pcirType->types);
    }
    else {
      assert(false);
    }

    return types[pcirType];
  }
  Type::Type() : type(pcir::Types::_UNDEFINED) {}
  Type::Type(pcir::Types langDefType) : type(langDefType)
  {
    assert(includes({
      pcir::Types::I8, pcir::Types::U8,
      pcir::Types::I16, pcir::Types::U16,
      pcir::Types::I32, pcir::Types::U32,
      pcir::Types::I64, pcir::Types::U64,
      pcir::Types::Ptr, pcir::Types::Function,
    }, langDefType));
  }
  size_t Type::getSize() const
  {
    switch(type) {
      case pcir::Types::I8:
      case pcir::Types::U8:
        return 8;
      case pcir::Types::I16:
      case pcir::Types::U16:
        return 16;
      case pcir::Types::I32:
      case pcir::Types::U32:
        return 32;
      case pcir::Types::I64:
      case pcir::Types::U64:
        return 64;
      case pcir::Types::Ptr:
      case pcir::Types::Function:
        return 64;
      default:
        assert(false);
        return 0;
    }
  }
}