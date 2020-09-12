#pragma once

#include "vector_utils.h"

namespace pick
{
    struct CompilerOption
    {
        std::string name;
        std::string src;
        std::string out;
        std::vector<std::string> lib;
        bool compilerDebugMode = false;
    };
    bool readCommandLine(CompilerOption& option, int argc, char* argv[]);
    void usage();
}