#pragma once

#include <unordered_map>

#include "token.h"
#include "syntax.h"

namespace pick
{
    struct Module
    {
        std::string name;
        std::string srcPath;
        token::TokenSequence tokenSequence;
        syntax::SyntaxTree syntaxTree;
        bool irGenerated;
    };
    struct ModuleNode
    {
        std::string name;
        std::unordered_map<std::string, Module> modules;
        std::unordered_map<std::string, ModuleNode> children;
    };
    struct ModuleTree
    {
        std::unordered_map<std::string, ModuleNode> nodes;
    };
}