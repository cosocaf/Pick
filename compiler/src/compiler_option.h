/**
 * @file compiler_option.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief コンパイラオプションについて定義する。
 * @version 0.1
 * @date 2021-07-22
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_COMPILER_OPTION_H_
#define PICKC_COMPILER_OPTION_H_

#include <string>
#include <vector>

namespace pickc
{
  /**
   * @brief pickがターゲットとするプラットフォームの列挙。
   * 
   */
  enum struct TargetPlatforms
  {
    /**
     * @brief ターゲットがWindow x64であることを表す。
     * 
     */
    WindowsX64,
    Unknown,
  };

  /**
   * @brief コンパイラオプションを保持するための構造体。
   * 
   */
  struct CompilerOption {
    /**
     * @brief --helpオプションが指定されている場合にtrue。
     * 
     */
    bool help;
    /**
     * @brief --versionオプションが指定されている場合にtrue。
     * 
     */
    bool version;
    /**
     * @brief --targetオプションで指定された値。
     * 
     */
    TargetPlatforms target;
    /**
     * @brief -gオプションが指定された場合にtrue。
     * 
     */
    bool debug;
    /**
     * @brief --projectオプションで指定された値。
     * 
     */
    std::string projectName;
    /**
     * @brief --mainオプションで指定された値。
     * 
     */
    std::string mainModule;
    /**
     * @brief --srcオプションで指定された値。
     * 
     */
    std::string srcDir;
    /**
     * @brief --outオプションで指定された値。
     * 
     */
    std::string outName;
    /**
     * @brief --out-dirオプションで指定された値。
     * 
     */
    std::string outDir;
    /**
     * @brief --libraryオプションで指定された値。
     * 
     */
    std::vector<std::string> libraries;
  };
  /**
   * @brief コンパイラオプション。initCompilerOption関数により初期化される。
   * 
   */
  extern CompilerOption compilerOption;
  
  /**
   * @brief pickc::compilerOptionを初期化する。main関数から呼び出される。
   * 
   * @param argc main関数の第一引数
   * @param argv main関数の第二引数
   */
  void initCompilerOption(int argc, char* argv[]);
}

#endif // PICKC_COMPILER_OPTION_H_