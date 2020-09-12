#include <iostream>
#include <ctime>

#include "compiler_option.h"
#include "compiler.h"

// std::localtimeを::localtime_sにするのは互換性的にあれなので。
#pragma warning(disable: 4996)
int main(int argc, char* argv[]) {
    using namespace pick;
    if (argc == 1) {
        usage();
        return 0;
    }
    CompilerOption option;
    if (!readCommandLine(option, argc, argv)) {
        std::cerr << "エラー: 不正なコマンドライン引数です。" << std::endl;
        return -1;
    }
    if (option.compilerDebugMode) {
        time_t now;
        std::time(&now);
        auto timeInfo = std::localtime(&now);
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", timeInfo);
        std::cout << "Pick Compiler C++ Implementation" << std::endl;
        std::cout << "\ndebug mode..." << std::endl;
        std::cout << buffer << std::endl;
        std::cout << "name: " << option.name << std::endl;
        std::cout << "src: " << option.src << std::endl;
        std::cout << "out: " << option.out << std::endl;
        for (const auto& lib : option.lib) {
            std::cout << "lib: " << lib << std::endl;
        }
    }
    Compiler compiler(option);
    return compiler.compile();
}