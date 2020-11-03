#include "routine.h"

namespace pickc::windows::x64
{
  JmpOperation::JmpOperation(pcir::FlowStruct* to) : Operation(OperationSize::QWord), to(to) {}
  BinaryVec JmpOperation::bin(WindowsX64& x64, Routine* routine)
  {
    return {};
  }
}