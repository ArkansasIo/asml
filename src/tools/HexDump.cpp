// ============================================================================
//  ASMLEX - src/tools/HexDump.cpp
//  Byte rendering and parsing utilities.
// ============================================================================
#include <asmlex/tools/HexDump.hpp>

#include <cstdio>

namespace asmlex::tools
{

  namespace
  {
    constexpr char kLowerDigits[] = "0123456789abcdef";
    constexpr char kUpperDigits[] = "0123456789ABCDEF";

    [[nodiscard]] char hex_digit(u8 value, bool upper)
    {
      return (upper ? kUpperDigits : kLowerDigits)[value & 0x0Fu];
    }

    [[nodiscard]] int digit_value(char c) noexcept
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

  String hex_bytes(const String &bytes, bool upper_case)
  {
    String out;
    out.reserve(bytes.size() * 3);
    for (usize i = 0; i < bytes.size(); ++i)
    {
      if (i != 0)
        out.push_back(' ');
      const u8 value = static_cast<u8>(bytes[i]);
      out.push_back(hex_digit(static_cast<u8>(value >> 4), upper_case));
      out.push_back(hex_digit(value, upper_case));
    }
    return out;
  }

  String hex_dump(const String &bytes, const HexDumpOptions &options)
  {
    const usize width = options.bytes_per_line == 0 ? 16 : options.bytes_per_line;
    String out;

    for (usize offset = 0; offset < bytes.size(); offset += width)
    {
      if (options.show_address)
      {
        char address[32]{};
        std::snprintf(address, sizeof(address), "%016llX",
                      static_cast<unsigned long long>(options.base_address + offset));
        out += address;
        out += "  ";
      }

      String ascii;
      for (usize i = 0; i < width; ++i)
      {
        const usize index = offset + i;
        if (index < bytes.size())
        {
          const u8 value = static_cast<u8>(bytes[index]);
          out.push_back(hex_digit(static_cast<u8>(value >> 4), options.upper_case));
          out.push_back(hex_digit(value, options.upper_case));
          out.push_back(' ');
          ascii.push_back((value >= 0x20 && value < 0x7F) ? static_cast<char>(value) : '.');
        }
        else
        {
          out += "   ";
        }
      }

      if (options.show_ascii)
      {
        out.push_back('|');
        out += ascii;
        out.push_back('|');
      }
      out.push_back('\n');
    }
    return out;
  }

  bool parse_hex_bytes(StringView text, String &out, String *error)
  {
    out.clear();
    int high = -1;

    for (usize i = 0; i < text.size(); ++i)
    {
      const char c = text[i];
      if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == ',' || c == ';')
      {
        if (high >= 0)
        {
          // A lone nibble is accepted as its own byte value.
          out.push_back(static_cast<char>(high));
          high = -1;
        }
        continue;
      }
      // Skip a 0x / 0X prefix.
      if (c == '0' && i + 1 < text.size() && (text[i + 1] == 'x' || text[i + 1] == 'X'))
      {
        ++i;
        continue;
      }

      const int value = digit_value(c);
      if (value < 0)
      {
        if (error)
          *error = String("not a hex digit: '") + c + "'";
        return false;
      }
      if (high < 0)
      {
        high = value;
      }
      else
      {
        out.push_back(static_cast<char>((high << 4) | value));
        high = -1;
      }
    }

    if (high >= 0)
      out.push_back(static_cast<char>(high));
    return true;
  }

  bool parse_address(StringView text, u64 &out)
  {
    const String clean = trim(text);
    if (clean.empty())
      return false;

    String body = clean;
    int base = 10;
    if (starts_with(clean, "0x") || starts_with(clean, "0X"))
    {
      body = clean.substr(2);
      base = 16;
    }
    else if (ends_with(clean, "h") || ends_with(clean, "H"))
    {
      body = clean.substr(0, clean.size() - 1);
      base = 16;
    }

    if (body.empty())
      return false;

    u64 value = 0;
    for (char c : body)
    {
      int digit = -1;
      if (c >= '0' && c <= '9')
        digit = c - '0';
      else if (base == 16 && c >= 'a' && c <= 'f')
        digit = c - 'a' + 10;
      else if (base == 16 && c >= 'A' && c <= 'F')
        digit = c - 'A' + 10;
      if (digit < 0 || digit >= base)
        return false;
      value = value * static_cast<u64>(base) + static_cast<u64>(digit);
    }
    out = value;
    return true;
  }

  String human_size(u64 bytes)
  {
    constexpr double kKiB = 1024.0;
    constexpr double kMiB = kKiB * 1024.0;
    constexpr double kGiB = kMiB * 1024.0;

    char buffer[64]{};
    if (static_cast<double>(bytes) < kKiB)
      std::snprintf(buffer, sizeof(buffer), "%llu B", static_cast<unsigned long long>(bytes));
    else if (static_cast<double>(bytes) < kMiB)
      std::snprintf(buffer, sizeof(buffer), "%.1f KiB", static_cast<double>(bytes) / kKiB);
    else if (static_cast<double>(bytes) < kGiB)
      std::snprintf(buffer, sizeof(buffer), "%.1f MiB", static_cast<double>(bytes) / kMiB);
    else
      std::snprintf(buffer, sizeof(buffer), "%.1f GiB", static_cast<double>(bytes) / kGiB);
    return String(buffer);
  }

} // namespace asmlex::tools
