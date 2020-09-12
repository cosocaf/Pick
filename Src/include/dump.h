#pragma once

#include "module.h"
#include "token.h"
#include "syntax.h"
#include "ir.h"

namespace pick::dump
{
    void moduleDump(const ModuleTree& mod);
    void tokenDump(const token::TokenSequence& sequence);
    void syntaxDump(const syntax::SyntaxTree& tree);
    void irDump(const ir::IntermediateRepresentation& ir);
}