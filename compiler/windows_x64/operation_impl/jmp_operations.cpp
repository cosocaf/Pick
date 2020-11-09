#include "routine.h"

namespace pickc::windows::x64
{
  JmpOperation::JmpOperation(size_t to) : Operation(OperationSize::QWord), to(to) {}
  BinaryVec JmpOperation::bin(WindowsX64& x64, Routine* routine)
  {
    BinaryVec code;
    code.push_back(0xE9);
    x64.relocs.insert(new RelocationInfo(routine, Relocation(to), this, code.size(), OperationSize::DWord, RelocationPosition::Relative));
    code << 0;
    return code;
  }
  BinaryVec JeOperation::bin(WindowsX64& x64, Routine* routine)
  {
    BinaryVec code;
    code.push_back(0x0F);
    code.push_back(0x84);
    x64.relocs.insert(new RelocationInfo(routine, Relocation(to), this, code.size(), OperationSize::DWord, RelocationPosition::Relative));
    code << 0;
    return code;
  }
  BinaryVec JneOperation::bin(WindowsX64& x64, Routine* routine)
  {
    BinaryVec code;
    code.push_back(0x0F);
    code.push_back(0x85);
    x64.relocs.insert(new RelocationInfo(routine, Relocation(to), this, code.size(), OperationSize::DWord, RelocationPosition::Relative));
    code << 0;
    return code;
  }
  BinaryVec JgOperation::bin(WindowsX64& x64, Routine* routine)
  {
    BinaryVec code;
    code.push_back(0x0F);
    code.push_back(0x8F);
    x64.relocs.insert(new RelocationInfo(routine, Relocation(to), this, code.size(), OperationSize::DWord, RelocationPosition::Relative));
    code << 0;
    return code;
  }
  BinaryVec JgeOperation::bin(WindowsX64& x64, Routine* routine)
  {
    BinaryVec code;
    code.push_back(0x0F);
    code.push_back(0x8D);
    x64.relocs.insert(new RelocationInfo(routine, Relocation(to), this, code.size(), OperationSize::DWord, RelocationPosition::Relative));
    code << 0;
    return code;
  }
  BinaryVec JlOperation::bin(WindowsX64& x64, Routine* routine)
  {
    BinaryVec code;
    code.push_back(0x0F);
    code.push_back(0x8C);
    x64.relocs.insert(new RelocationInfo(routine, Relocation(to), this, code.size(), OperationSize::DWord, RelocationPosition::Relative));
    code << 0;
    return code;
  }
  BinaryVec JleOperation::bin(WindowsX64& x64, Routine* routine)
  {
    BinaryVec code;
    code.push_back(0x0F);
    code.push_back(0x8E);
    x64.relocs.insert(new RelocationInfo(routine, Relocation(to), this, code.size(), OperationSize::DWord, RelocationPosition::Relative));
    code << 0;
    return code;
  }
}