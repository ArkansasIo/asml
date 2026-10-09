// ============================================================================
//  ASMLEX - src/core/Types.cpp
//  Free helpers declared in core/Bytes.hpp: hex parsing, formatting, dumps.
// ============================================================================
#include <asmlex/core/Bytes.hpp>
#include <asmlex/core/Diagnostics.hpp>

namespace asmlex::core
{

  namespace
  {
    [[nodiscard]] int hex_digit(char c) noexcept
    {
      if (c >= '0' && c <= '9')
        return c - '0';
      if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
      if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
      return -1;
    }
  } // namespace

  String parse_hex_bytes(StringView text, DiagnosticSink *sink)
  {
    String out;
    int high = -1;
    usize digits = 0;
    for (usize i = 0; i < text.size(); ++i)
    {
      const char c = text[i];
      if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == ',' || c == '_')
        continue;
      if (c == 'x' || c == 'X')
        continue; // tolerate 0x48 prefixes
      const int nibble = hex_digit(c);
      if (nibble < 0)
      {
        if (sink)
        {
          Diagnostic d;
          d.severity = Severity::Error;
          d.code = "B1001";
          d.text = String("invalid hex byte: unexpected character '") + c + "'";
          d.where.begin.offset = i;
          d.where.begin.column = static_cast<u32>(i + 1);
          sink->report(std::move(d));
        }
        return out;
      }
      ++digits;
      if (high < 0)
      {
        high = nibble;
      }
      else
      {
        out.push_back(static_cast<char>((high << 4) | nibble));
        high = -1;
      }
    }
    if (high >= 0)
    {
      if (sink)
        sink->warning("B1002", "odd number of hex digits; trailing nibble ignored");
    }
    (void)digits;
    return out;
  }

  String format_hex_bytes(StringView bytes, bool uppercase)
  {
    const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    String out;
    out.reserve(bytes.size() * 3);
    for (usize i = 0; i < bytes.size(); ++i)
    {
      if (i)
        out.push_back(' ');
      const u8 b = static_cast<u8>(bytes[i]);
      out.push_back(digits[(b >> 4) & 0xF]);
      out.push_back(digits[b & 0xF]);
    }
    return out;
  }

  String hex_dump(StringView bytes, usize base_offset)
  {
    String out;
    for (usize row = 0; row < bytes.size(); row += 16)
    {
      char addr[32];
      std::snprintf(addr, sizeof(addr), "%08zX  ", static_cast<usize>(base_offset + row));
      out += addr;

      String ascii;
      for (usize col = 0; col < 16; ++col)
      {
        const usize i = row + col;
        if (i < bytes.size())
        {
          const u8 b = static_cast<u8>(bytes[i]);
          const char *digits = "0123456789ABCDEF";
          out.push_back(digits[(b >> 4) & 0xF]);
          out.push_back(digits[b & 0xF]);
          out.push_back(' ');
          ascii.push_back((b >= 32 && b < 127) ? static_cast<char>(b) : '.');
        }
        else
        {
          out += "   ";
        }
        if (col == 7)
          out.push_back(' ');
      }
      out += " |";
      out += ascii;
      out += "|\n";
    }
    return out;
  }

} // namespace asmlex::core
