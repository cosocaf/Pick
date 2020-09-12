#pragma once

#include <unordered_map>

#include "compiler_option.h"
#include "result.h"
#include "module.h"
#include "ir.h"
#include "x86_64.h"
#include "exe.h"

namespace pick
{
    class Compiler
    {
        CompilerOption option;
        ModuleTree tree;
        ir::IntermediateRepresentation ir;
        x86_64::X86_64 x86_64;
        Result<_, std::vector<std::string>> createModuleTree();
        Result<_, std::vector<std::string>> tokenize();
        Result<_, std::vector<std::string>> parse();
        Result<_, std::vector<std::string>> irGenerate();
        Result<_, std::vector<std::string>> x86_64Compile();
        Result<_, std::vector<std::string>> exeLink();
    public:
        Compiler(const CompilerOption& option);
        int compile();
    };
}