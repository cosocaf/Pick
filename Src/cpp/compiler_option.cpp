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
        std::cout << "�g�p���@: pickc <options>\n";
        std::cout << "�g�p�\�ȃI�v�V�����͈ȉ��̒ʂ�ł��B\n";
        std::cout << "    --name                    �o�͂����t�@�C�������w�肵�܂��B�f�t�H���g��a���w�肳��܂��B\n";
        std::cout << "    --src                     �\�[�X�̃��[�g�f�B���N�g�����w�肵�܂��B\n";
        std::cout << "    --out                     �o�͐�̃f�B���N�g�����w�肵�܂��B\n";
        std::cout << "    --lib                     ���C�u�����t�@�C�����w�肵�܂��B\n";
        std::cout << "    --compiler-debug-mode     �R���p�C���̓��e���_���v���܂��B\n";
    }
}