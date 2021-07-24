/**
 * @file compiler_option.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief compiler_option.hの実装
 * @version 0.1
 * @date 2021-07-22
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "compiler_option.h"

#include <vector>
#include <string>
#include <optional>

#include "config.h"
#include "output_buffer.h"
#include "utils/string_utils.h"

namespace pickc
{
  namespace {
    enum struct CompOptionKind {
      Help,
      Version,
      Target,
      Debug,
      ProjectName,
      MainModule,
      SrcDir,
      OutName,
      OutDir,
      Library,
    };
    std::optional<CompOptionKind> findOptionKind(const std::string& s) {
      if(s == "--help" || s == "-h" || s == "-?") {
        return CompOptionKind::Help;
      }
      else if(s == "--version" || s == "-v") {
        return CompOptionKind::Version;
      }
      else if(s == "--target") {
        return CompOptionKind::Target;
      }
      else if(s == "-g") {
        return CompOptionKind::Debug;
      }
      else if(s == "--project" || s == "-p") {
        return CompOptionKind::ProjectName;
      }
      else if(s == "--main" || s == "-m") {
        return CompOptionKind::MainModule;
      }
      else if(s == "--src" || s == "-s") {
        return CompOptionKind::SrcDir;
      }
      else if(s == "--out" || s == "-o") {
        return CompOptionKind::OutName;
      }
      else if(s == "--out-dir" || s == "-d") {
        return CompOptionKind::OutDir;
      }
      else if(s == "--library" || s == "-l") {
        return CompOptionKind::Library;
      }
      else {
        return std::nullopt;
      }
    }
    std::optional<TargetPlatforms> findTarget(const std::string& s) {
      if(s == "windows_x64") {
        return TargetPlatforms::WindowsX64;
      }
      else if(s == "unknown") {
        return TargetPlatforms::Unknown;
      }
      else {
        return std::nullopt;
      }
    }
    struct CompilerOptionErrorMessage : public OutputMessage {
      CompilerOptionErrorMessage(const std::string& message) : OutputMessage(OutputMessageKind::Error, message) {}
      virtual std::string toString() const override {
        return message + "\n使用方法: pickc [OPTIONS]\n詳しくはpickc --helpを使用してください。";
      }
    };
    bool grouping(int argc, char* argv[]) {
      for(int i = 1; i < argc; ++i) {
        std::string s = argv[i];
        if(startsWith(s, "-")) {
          auto kind = findOptionKind(s);
          if(!kind) {
            outputBuffer.emplace<CompilerOptionErrorMessage>(s + "は無効なフラグです。");
            return false;
          }
          else switch(kind.value()) {
            case CompOptionKind::Help:
              compilerOption.help = true;
              break;
            case CompOptionKind::Version:
              compilerOption.version = true;
              break;
            case CompOptionKind::Target:
              if(++i < argc && !startsWith(argv[i], "-")) {
                s = argv[i];
                auto target = findTarget(s);
                if(!target) {
                  outputBuffer.emplace<CompilerOptionErrorMessage>(s + "は無効なターゲットです。");
                  return false;
                }
                else {
                  compilerOption.target = target.value();
                }
              }
              else {
                outputBuffer.emplace<CompilerOptionErrorMessage>("ターゲット名が必要です。");
                return false;
              }
              break;
            case CompOptionKind::Debug:
              compilerOption.debug = true;
              break;
            case CompOptionKind::ProjectName:
              if(++i < argc && !startsWith(argv[i], "-")) {
                compilerOption.projectName = argv[i];
              }
              else {
                outputBuffer.emplace<CompilerOptionErrorMessage>("プロジェクト名が必要です。");
                return false;
              }
              break;
            case CompOptionKind::MainModule:
              if(++i < argc && !startsWith(argv[i], "-")) {
                compilerOption.mainModule = argv[i];
              }
              else {
                outputBuffer.emplace<CompilerOptionErrorMessage>("メインモジュール名が必要です。");
                return false;
              }
              break;
            case CompOptionKind::SrcDir:
              if(++i < argc && !startsWith(argv[i], "-")) {
                compilerOption.srcDir = argv[i];
              }
              else {
                outputBuffer.emplace<CompilerOptionErrorMessage>("ソースディレクトリ名が必要です。");
                return false;
              }
              break;
            case CompOptionKind::OutName:
              if(++i < argc && !startsWith(argv[i], "-")) {
                compilerOption.outName = argv[i];
              }
              else {
                outputBuffer.emplace<CompilerOptionErrorMessage>("出力ファイル名が必要です。");
                return false;
              }
              break;
            case CompOptionKind::OutDir:
              if(++i < argc && !startsWith(argv[i], "-")) {
                compilerOption.outDir = argv[i];
              }
              else {
                outputBuffer.emplace<CompilerOptionErrorMessage>("出力ディレクトリ名が必要です。");
                return false;
              }
              break;
            case CompOptionKind::Library:
              if(++i < argc && !startsWith(argv[i], "-")) {
                compilerOption.libraries.emplace_back(argv[i]);
                while(++i < argc && !startsWith(argv[i], "-")) {
                  compilerOption.libraries.emplace_back(argv[i]);
                }
                --i;
              }
              else {
                outputBuffer.emplace<CompilerOptionErrorMessage>("ライブラリ名が必要です。");
                return false;
              }
              break;
          }
        }
        else {
          outputBuffer.emplace<CompilerOptionErrorMessage>(s + "は無効な入力です。");
        }
      }
      return true;
    }
  }

  CompilerOption compilerOption;
  void initCompilerOption(int argc, char* argv[]) {
    static bool inited = false;
    assert(inited == false);
    inited = true;

    compilerOption.help = false;
    compilerOption.version = false;

#if defined(_WIN64) || defined(WIN64)
    compilerOption.target = TargetPlatforms::WindowsX64;
#else
    compilerOption.target = TargetPlatforms::Unknown;
#endif

    compilerOption.debug = false;
    compilerOption.projectName = "";
    compilerOption.mainModule = "";
    compilerOption.srcDir = "./";
    compilerOption.outDir = "./";
    compilerOption.outName = "";
    compilerOption.libraries = {};

    if(!grouping(argc, argv)) return;

    if(!compilerOption.help && !compilerOption.version && compilerOption.projectName.empty()) {
      outputBuffer.emplace<CompilerOptionErrorMessage>("プロジェクト名は必須オプションです。");
      return;
    }

    if(compilerOption.mainModule.empty()) compilerOption.mainModule = compilerOption.projectName;
    if(compilerOption.outName.empty()) compilerOption.outName = compilerOption.projectName;
  }
}