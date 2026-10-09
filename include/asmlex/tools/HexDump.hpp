// ============================================================================
//  ASMLEX - include/asmlex/tools/HexDump.hpp
//  Formatting helpers shared by the hex viewer, the CLI and the tests.
// ============================================================================
#pragma once

#include <asmlex/core/Types.hpp>

#include <vector>

namespace asmlex::tools
{

  struct HexDumpOptions
  {
    u64 base_address = 0;
    usize bytes_per_line = 16;
    bool show_ascii = true;
    bool show_address = true;
    bool upper_case = false;
  };

  /// Classic "ADDR  BYTES  |ascii|" rendering of `bytes`.
  [[nodiscard]] String hex_dump(const String &bytes, const HexDumpOptions &options = {});

  /// Just the hex bytes, space separated, e.g. "48 8B C1".
  [[nodiscard]] String hex_bytes(const String &bytes, bool upper_case = false);

  /// Parse "48 8B C1" (or "48,8b,c1", or "0x48 0x8B") back into raw bytes.
  /// Returns false and fills `error` when a token is not a byte.
  bool parse_hex_bytes(StringView text, String &out, String *error = nullptr);

  /// Parse "0x1000" / "1000h" / "4096" into an address.  Returns false when the
  /// text is not a valid number in any supported form.
  bool parse_address(StringView text, u64 &out);

  /// Render a byte count as "1.5 KiB" / "2.0 MiB".
  [[nodiscard]] String human_size(u64 bytes);

} // namespace asmlex::tools
