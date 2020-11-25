#ifndef PICKC_PICKC_CONFIG_H_
#define PICKC_PICKC_CONFIG_H_

#include <cstdint>

namespace pickc
{
  constexpr auto VERSION = "pickc 0.0.0 alpha";
  constexpr uint16_t MAJOR_VERSION = 0;
  constexpr uint16_t MINOR_VERSION = 0;

  constexpr auto STATUS_SUCCESS                   = 0x00000000;
  constexpr auto STATUS_INVALID_OPTION            = 0x00000001;
  constexpr auto STATUS_PARSER_ERROR              = 0x00000002;
  constexpr auto STATUS_PCIR_ERROR                = 0x00000004;
  constexpr auto STATUS_BUNDLER_ERROR             = 0x00000008;
  constexpr auto STATUS_UNKNOWN_PLATFORM          = 0x00000010;
  constexpr auto STATUS_WINDOWS_X64_ERROR         = 0x00000020;
  constexpr auto STATUS_WINDOWS_X64_LINKER_ERROR  = 0x00000040;

  constexpr auto CONSOLE_BG_BLACK   = "\x1b[40m";
  constexpr auto CONSOLE_BG_RED     = "\x1b[41m";
  constexpr auto CONSOLE_BG_GREEN   = "\x1b[42m";
  constexpr auto CONSOLE_BG_YELLOW  = "\x1b[43m";
  constexpr auto CONSOLE_BG_BLUE    = "\x1b[44m";
  constexpr auto CONSOLE_BG_MAGENTA = "\x1b[45m";
  constexpr auto CONSOLE_BG_CYAN    = "\x1b[46m";
  constexpr auto CONSOLE_BG_GRAY    = "\x1b[47m";
  constexpr auto CONSOLE_BG_DEFAULT = "\x1b[49m";

  constexpr auto CONSOLE_FG_BLACK   = "\x1b[30m";
  constexpr auto CONSOLE_FG_RED     = "\x1b[31m";
  constexpr auto CONSOLE_FG_GREEN   = "\x1b[32m";
  constexpr auto CONSOLE_FG_YELLOW  = "\x1b[33m";
  constexpr auto CONSOLE_FG_BLUE    = "\x1b[34m";
  constexpr auto CONSOLE_FG_MAGENTA = "\x1b[35m";
  constexpr auto CONSOLE_FG_CYAN    = "\x1b[36m";
  constexpr auto CONSOLE_FG_WHITE   = "\x1b[37m";
  constexpr auto CONSOLE_FG_DEFAULT = "\x1b[39m";

  constexpr auto CONSOLE_UNDER_LINE = "\x1b[4m";
  constexpr auto CONSOLE_HIGHLIGHT  = "\x1b[1m";
  constexpr auto CONSOLE_INVERT     = "\x1b[7m";
  constexpr auto CONSOLE_DEFAULT    = "\x1b[0m";
}

#endif // PICKC_PICKC_CONFIG_H_