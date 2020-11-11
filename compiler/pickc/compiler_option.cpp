#include "compiler_option.h"

#include <iostream>

#include "config.h"
#include "utils/string_utils.h"

namespace pickc
{
  namespace
  {
    void version()
    {
      std::cout << VERSION << std::endl;
    }
    void usage()
    {
      std::cout <<
        "使用方法: pickc [OPTIONS] <INPUT_DIR>\n"
        "使用可能なオプションは以下の通りです。\n"
        "    --help, -h, -?        このメッセージを出力します。\n"
        "    --version, -v         バージョン情報を出力します。\n"
        "    --target <TARGET>     ビルドターゲットを指定します。使用可能なターゲット: [windows_x64]\n"
        "    -g                    コンパイラのデバッグ情報を出力します。\n"
        "    --project, -p <NAME>  プロジェクトの名前を指定します。ルートモジュール名はこの名前になります。このオプションは必須です。\n"
        "    --main -m <NAME>      メイン関数の存在するモジュールを指定します。指定しない場合はプロジェクトの名前と同じになります。\n"
        "    --out, -o <NAME>      出力ファイルの名前を指定します。指定しない場合はプロジェクトの名前と同じになります。拡張子は自動で付与されます。\n"
        "    --out-dir, -d <PATH>  出力先のディレクトリを指定します。指定しない場合は現在の位置に出力します。\n"
        "    --library, -l <PATH>  リンクするライブラリを指定します。"
        << std::endl;
    }
  }
  CompilerOption::CompilerOption() :
    target(
      #ifdef _WIN64
        TargetPlatforms::WindowsX64
      #else
        #error "サポートされていないプラットフォームです。"
      #endif
    ),
    compilerDebug(false),
    projectName(""),
    mainModule(""),
    out(""),
    outDir("./"),
    libraries(),
    srcDir("./")
  {}
  Result<CompilerOption, std::string> CompilerOption::create(int argc, char* argv[])
  {
    if(argc == 1) {
      usage();
      std::exit(STATUS_SUCCESS);
    }
    CompilerOption option;
    // argv[0]は実行パスなので、スキップする。
    for(int i = 1; i < argc; ++i) {
      std::string str(argv[i]);
      if (str == "--help" || str == "-h" || str == "-?") {
        usage();
        std::exit(STATUS_SUCCESS);
      }
      else if(str == "--version" || str == "-v") {
        version();
        std::exit(STATUS_SUCCESS);
      }
      else if(str == "--target") {
        if(++i < argc && !startsWith(argv[i], "-")) {
          std::string target(argv[i]);
          if(target == "windows_x64") {
            option.target = TargetPlatforms::WindowsX64;
          }
          else {
            return error("サポートされていないターゲットが指定されました。サポートされるターゲットは[windows_x64]です。");
          }
        }
        else {
          return error("--targetには引数が必要です。");
        }
      }
      else if(str == "-g") {
        option.compilerDebug = true;
      }
      else if(str == "--project" || str == "-p") {
        if(++i < argc && !startsWith(argv[i], "-")) {
          option.projectName = argv[i];
        }
        else {
          return error("--project, -pには引数が必要です。");
        }
      }
      else if(str == "--main" || str == "-m") {
        if(++i < argc && !startsWith(argv[i], "-")) {
          option.mainModule = argv[i];
        }
        else {
          return error("--main, -mには引数が必要です。");
        }
      }
      else if(str == "--out" || str == "-o") {
        if(++i < argc && !startsWith(argv[i], "-")) {
          option.out = argv[i];
        }
        else {
          return error("--out, -oには引数が必要です。");
        }
      }
      else if(str == "--out-dir" || str == "-d") {
        if(++i < argc && !startsWith(argv[i], "-")) {
          option.outDir = argv[i];
        }
        else {
          return error("--out-dirには引数が必要です。");
        }
      }
      else if(str == "--library" || str == "-l") {
        if(++i < argc && !startsWith(argv[i], "-")) {
          option.libraries.push_back(argv[i]);
        }
        else {
          return error("--library, -lには引数が必要です。");
        }
      }
      else if(!startsWith(argv[i], "-")) {
        option.srcDir = argv[i];
      }
      else {
        return error(std::string(argv[i]) + "は無効なオプションです。");
      }
    }
    if(option.projectName.empty()) return error("プロジェクト名が指定されていません。--projectオプションは必須です。");
    if(option.out.empty()) option.out = option.projectName;
    if(option.mainModule.empty()) option.mainModule = option.projectName;
    return ok(option);
  }
  void CompilerOption::dump() const
  {
    std::string targetString;
    switch(target) {
      case TargetPlatforms::WindowsX64: targetString = "Windows x64"; break;
      default: assert(false);
    }
    std::cout << "Platform Target: " << targetString << std::endl;
    std::cout << "Project Name:    " << projectName << std::endl;
    std::cout << "Main Module:     " << mainModule << std::endl;
    std::cout << "Output Name:     " << out << std::endl;
    std::cout << "Output Dir:      " << outDir << std::endl;
    std::cout << "Source Dir:      " << srcDir << std::endl;
    std::cout << "Libraries:       [";
    for(const auto& lib : libraries) {
      std::cout << "\n    " << lib;
    }
    if(!libraries.empty()) std::cout << '\n';
    std::cout << ']' << std::endl;
  }
}