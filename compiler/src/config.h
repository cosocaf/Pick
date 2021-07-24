/**
 * @file config.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief コンパイラ全体で使用する定数群を定義する。
 * @version 0.1
 * @date 2021-07-22
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_PICKC_CONFIG_H_
#define PICKC_PICKC_CONFIG_H_

#include <cstdint>

namespace pickc
{
  /**
   * @brief コンパイラのバージョンの文字列表現。
   * 
   */
  constexpr auto VERSION = "pickc 0.0.0 alpha";
  /**
   * @brief コンパイラのメジャーバージョン
   * 
   */
  constexpr uint16_t MAJOR_VERSION = 0;
  /**
   * @brief コンパイラのマイナーバージョン
   * 
   */
  constexpr uint16_t MINOR_VERSION = 0;

  /**
   * @brief コンパイラオプションにヘルプを指定したときに出力するメッセージ。
   * 
   */
  constexpr auto USAGE =
    "使用方法: pickc [OPTIONS]\n"
    "使用可能なオプションは以下の通りです。\n"
    "    --help, -h, -?           このメッセージを出力します。\n"
    "    --version, -v            バージョン情報を出力します。\n"
    "    --target <TARGET>        ビルドターゲットを指定します。使用可能なターゲット: [windows_x64]\n"
    "    -g                       コンパイラのデバッグ情報を出力します。\n"
    "    --project, -p <NAME>     プロジェクトの名前を指定します。ルートモジュール名はこの名前になります。このオプションは必須です。\n"
    "    --main -m <NAME>         main関数の存在するモジュールを指定します。指定しない場合はルートモジュールになります。\n"
    "    --src -s <NAME>          ルートモジュールの存在するディレクトリを指定します。指定しない場合は現在の位置になります。\n"
    "    --out, -o <NAME>         出力ファイルの名前を指定します。指定しない場合はルートモジュール名になります。拡張子は自動で付与されます。\n"
    "    --out-dir, -d <PATH>     出力先のディレクトリを指定します。指定しない場合は現在の位置に出力します。\n"
    "    --library, -l <PATH>...  リンクするライブラリを指定します。そのまま並べることで複数指定できます。";

  /**
   * @brief コンパイルが正常に完了した場合にmain関数から返される。
   * 
   */
  constexpr auto STATUS_SUCCESS                   = 0x00000000;
  /**
   * @brief コンパイラに異常なオプションを指定した場合にmain関数から返される。
   * 
   */
  constexpr auto STATUS_INVALID_OPTION            = 0x00000001;
  /**
   * @brief パース作業中に失敗した場合にmain関数から返される。
   * 
   */
  constexpr auto STATUS_PARSER_ERROR              = 0x00000002;
  constexpr auto STATUS_PCIR_ERROR                = 0x00000004;
  constexpr auto STATUS_BUNDLER_ERROR             = 0x00000008;
  constexpr auto STATUS_UNKNOWN_PLATFORM          = 0x00000010;
  constexpr auto STATUS_WINDOWS_X64_ERROR         = 0x00000020;
  constexpr auto STATUS_WINDOWS_X64_LINKER_ERROR  = 0x00000040;
}

#endif // PICKC_PICKC_CONFIG_H_