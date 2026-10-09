// ============================================================================
//  ASMLEX - include/asmlex/core/Version.hpp
//  Product identity shared by the window title, the CLI banner and the JSON
//  project format.
// ============================================================================
#pragma once

#include <asmlex/core/Types.hpp>

namespace asmlex::core
{

  inline constexpr StringView kProductName = "ASMLEX";
  inline constexpr StringView kProductTitle = "ASMLEX - Assembler / Disassembler Studio";
  inline constexpr int kVersionMajor = 1;
  inline constexpr int kVersionMinor = 0;
  inline constexpr int kVersionPatch = 0;
  inline constexpr StringView kVersionString = "1.0.0";

  /// Bump whenever the on-disk project / session JSON layout changes.
  inline constexpr int kProjectFormatVersion = 1;

  [[nodiscard]] inline String version_banner()
  {
    String out(kProductName);
    out += ' ';
    out += kVersionString;
    return out;
  }

} // namespace asmlex::core
