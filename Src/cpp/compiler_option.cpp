#include "compiler_option.h"

#include <iostream>

namespace pick
{
    bool readCommandLine(CompilerOption& option, int argc, char* argv[])
    {
        option.name = "a";
        option.src = "./";
        option.out = "./";
        option.compilerDebugMode = false;
        for (int i = 1; i < argc; ++i) {
            if (!strcmp(argv[i], "--name")) {
                if (++i >= argc) return false;
                option.name = argv[i];
            }
            else if (!strcmp(argv[i], "--src")) {
                if (++i >= argc) return false;
                option.src = argv[i];
            }
            else if (!strcmp(argv[i], "--out")) {
                if (++i >= argc) return false;
                option.out = argv[i];
            }
            else if (!strcmp(argv[i], "--lib")) {
                if (++i >= argc) return false;
                for (; i < argc; ++i) {
                    if (!strncmp(argv[i], "--", 2)) {
                        --i;
                        break;
                    }
                    option.lib += argv[i];
                }
            }
            else if (!strcmp(argv[i], "--compiler-debug-mode")) {
                option.compilerDebugMode = true;
            }
            else {
                return false;
            }
        }
        return true;
    }
    void usage()
    {
        std::cout << "使用方法: pickc <options>\n";
        std::cout << "使用可能なオプションは以下の通りです。\n";
        std::cout << "    --name                    出力されるファイル名を指定します。デフォルトでaが指定されます。\n";
        std::cout << "    --src                     ソースのルートディレクトリを指定します。\n";
        std::cout << "    --out                     出力先のディレクトリを指定します。\n";
        std::cout << "    --lib                     ライブラリファイルを指定します。\n";
        std::cout << "    --compiler-debug-mode     コンパイルの内容をダンプします。\n";
    }
}