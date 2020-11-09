#include "module_analyzer.h"

#include "utils/instanceof.h"
#include "utils/vector_utils.h"

namespace pickc::pcir
{
  Result<Register*, std::vector<std::string>> ModuleAnalyzer::literalAnalyze(const parser::LiteralNode* literal, FlowNode** flow)
  {
    using namespace parser;
    auto reg = new Register();
    (*flow)->addReg(reg);
    if(instanceof<IntegerLiteral>(literal)) {
      reg->type = Type(Types::I32);
      auto inst = new ImmMove();
      inst->dist = reg;
      inst->type = reg->type;
      inst->imm.i32 = dynamic_cast<const IntegerLiteral*>(literal)->value;
      (*flow)->insts.push_back(inst);
    }
    else if(instanceof<I8Literal>(literal)) {
      reg->type = Type(Types::I8);
      auto inst = new ImmMove();
      inst->dist = reg;
      inst->type = reg->type;
      inst->imm.i8 = dynamic_cast<const I8Literal*>(literal)->value;
      (*flow)->insts.push_back(inst);
    }
    else if(instanceof<I16Literal>(literal)) {
      reg->type = Type(Types::I16);
      auto inst = new ImmMove();
      inst->dist = reg;
      inst->type = reg->type;
      inst->imm.i16 = dynamic_cast<const I16Literal*>(literal)->value;
      (*flow)->insts.push_back(inst);
    }
    else if(instanceof<I32Literal>(literal)) {
      reg->type = Type(Types::I32);
      auto inst = new ImmMove();
      inst->dist = reg;
      inst->type = reg->type;
      inst->imm.i32 = dynamic_cast<const I32Literal*>(literal)->value;
      (*flow)->insts.push_back(inst);
    }
    else if(instanceof<I64Literal>(literal)) {
      reg->type = Type(Types::I64);
      auto inst = new ImmMove();
      inst->dist = reg;
      inst->type = reg->type;
      inst->imm.i64 = dynamic_cast<const I64Literal*>(literal)->value;
      (*flow)->insts.push_back(inst);
    }
    else {
      assert(false);
    }
    return ok(reg);
  }
}