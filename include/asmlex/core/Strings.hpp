// ============================================================================
//  ASMLEX - include/asmlex/core/Strings.hpp
//  Extra string helpers layered on top of core/Types.hpp: tokenisation, joins,
//  quoting, number formatting and small parse helpers used by the CLI and the
//  JSON project reader.
// ============================================================================
#pragma once

#include <asmlex/core/Types.hpp>

#include <vector>

namespace asmlex::core
{

  /// Split `text` on any character contained in `delims` (consecutive
  /// delimiters produce empty fields, mirroring std::getline behaviour).
  [[nodiscard]] inline std::vector<String> split(StringView text, StringView delims)
  {
    std::vector<String> parts;
    usize start = 0;
    for (usize i = 0; i <= text.size(); ++i)
    {
      const bool boundary = (i == text.size()) || delims.find(text[i]) != StringView::npos;
      if (!boundary)
        continue;
      parts.emplace_back(text.substr(start, i - start));
      start = i + 1;
    }
    return parts;
  }

  /// Split `text` on ASCII whitespace, dropping empty fields.
  [[nodiscard]] inline std::vector<String> split_ws(StringView text)
  {
    std::vector<String> parts;
    usize i = 0;
    while (i < text.size())
    {
      while (i < text.size() && (text[i] == ' ' || text[i] == '\t' || text[i] == '\r' ||
                                 text[i] == '\n' || text[i] == ','))
        ++i;
      const usize start = i;
      while (i < text.size() && !(text[i] == ' ' || text[i] == '\t' || text[i] == '\r' ||
                                  text[i] == '\n' || text[i] == ','))
        ++i;
      if (i > start)
        parts.emplace_back(text.substr(start, i - start));
    }
    return parts;
  }

  /// Join `parts` with `sep` between each element.
  [[nodiscard]] inline String join(const std::vector<String> &parts, StringView sep)
  {
    String out;
    for (usize i = 0; i < parts.size(); ++i)
    {
      if (i)
        out.append(sep);
      out += parts[i];
    }
    return out;
  }

  /// Wrap `text` in double quotes, escaping control characters and backslashes.
  [[nodiscard]] inline String quote(StringView text)
  {
    String out;
    out.reserve(text.size() + 2);
    out.push_back('"');
    for (char c : text)
    {
      switch (c)
      {
      case '"':
        out += "\\\"";
        break;
      case '\\':
        out += "\\\\";
        break;
      case '\n':
        out += "\\n";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\t':
        out += "\\t";
        break;
      default:
        out.push_back(c);
        break;
      }
    }
    out.push_back('"');
    return out;
  }

  /// Render `value` as an unsigned hexadecimal string, optionally 0x-prefixed.
  [[nodiscard]] inline String to_hex(u64 value, bool prefix = true, int min_digits = 0)
  {
    static constexpr char kDigits[] = "0123456789ABCDEF";
    char buffer[24];
    int n = 0;
    if (value == 0)
    {
      buffer[n++] = '0';
    }
    while (value != 0)
    {
      buffer[n++] = kDigits[value & 0xF];
      value >>= 4;
    }
    while (n < min_digits)
      buffer[n++] = '0';

    String out;
    if (prefix)
      out += "0x";
    for (int i = n - 1; i >= 0; --i)
      out.push_back(buffer[i]);
    return out;
  }

  /// Render a byte count as "0x....." address with fixed width (default 16).
  [[nodiscard]] inline String to_address(u64 value, int width = 16)
  {
    String digits = to_hex(value, false);
    if (static_cast<int>(digits.size()) < width)
      digits = String(static_cast<usize>(width) - digits.size(), '0') + digits;
    return "0x" + digits;
  }

  /// Parse a decimal / 0x / 0b integer.  Returns false on malformed input.
  [[nodiscard]] inline bool parse_i64(StringView text, i64 &out)
  {
    const String trimmed = trim(text);
    if (trimmed.empty())
      return false;

    bool negative = false;
    usize i = 0;
    if (trimmed[i] == '+' || trimmed[i] == '-')
    {
      negative = trimmed[i] == '-';
      ++i;
    }

    int base = 10;
    if (i + 1 < trimmed.size() && trimmed[i] == '0')
    {
      const char tag = trimmed[i + 1];
      if (tag == 'x' || tag == 'X')
      {
        base = 16;
        i += 2;
      }
      else if (tag == 'b' || tag == 'B')
      {
        base = 2;
        i += 2;
      }
      else if (tag == 'o' || tag == 'O')
      {
        base = 8;
        i += 2;
      }
    }

    u64 acc = 0;
    bool any = false;
    for (; i < trimmed.size(); ++i)
    {
      const char c = trimmed[i];
      if (c == '_')
        continue;
      int d = -1;
      if (c >= '0' && c <= '9')
        d = c - '0';
      else if (c >= 'a' && c <= 'f')
        d = c - 'a' + 10;
      else if (c >= 'A' && c <= 'F')
        d = c - 'A' + 10;
      else
        return false;
      if (d >= base)
        return false;
      acc = acc * static_cast<u64>(base) + static_cast<u64>(d);
      any = true;
    }
    if (!any)
      return false;

    out = negative ? -static_cast<i64>(acc) : static_cast<i64>(acc);
    return true;
  }

  /// True when `c` is an ASCII letter or digit.
  [[nodiscard]] constexpr bool is_alnum(char c) noexcept
  {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
  }

  /// True when `text` is a valid C-style identifier.
  [[nodiscard]] inline bool is_identifier(StringView text)
  {
    if (text.empty())
      return false;
    const char first = text[0];
    if (!((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z') || first == '_'))
      return false;
    for (char c : text)
    {
      if (!(is_alnum(c) || c == '_' || c == '.' || c == '@' || c == '?'))
        return false;
    }
    return true;
  }

} // namespace asmlex::core
