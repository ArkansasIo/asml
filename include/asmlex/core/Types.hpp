// ============================================================================
//  ASMLEX - include/asmlex/core/Types.hpp
//  Fundamental value/binary types shared by every subsystem.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace asmlex::core
{

  // --- scalar aliases ---------------------------------------------------------
  using u8 = std::uint8_t;
  using u16 = std::uint16_t;
  using u32 = std::uint32_t;
  using u64 = std::uint64_t;
  using i8 = std::int8_t;
  using i16 = std::int16_t;
  using i32 = std::int32_t;
  using i64 = std::int64_t;
  using f32 = float;
  using f64 = double;
  using usize = std::size_t;
  using uptr = std::uintptr_t;

  /// Identifier used for every named entity (symbols, labels, types, nodes...).
  using Id = std::string;
  using String = std::string;
  using StringView = std::string_view;

  // --- helpers ----------------------------------------------------------------
  [[nodiscard]] constexpr u16 lo16(u32 v) noexcept
  {
    return static_cast<u16>(v & 0xFFFFu);
  }
  [[nodiscard]] constexpr u16 hi16(u32 v) noexcept
  {
    return static_cast<u16>((v >> 16) & 0xFFFFu);
  }
  [[nodiscard]] constexpr u8 lo8(u16 v) noexcept
  {
    return static_cast<u8>(v & 0xFFu);
  }
  [[nodiscard]] constexpr u8 hi8(u16 v) noexcept
  {
    return static_cast<u8>((v >> 8) & 0xFFu);
  }

  /// Push a little-endian integer into a byte buffer.
  template <typename T>
  void push_le(std::string &out, T value)
  {
    for (usize i = 0; i < sizeof(T); ++i)
    {
      out.push_back(static_cast<char>((static_cast<u64>(value) >> (i * 8)) & 0xFFu));
    }
  }

  /// Read a little-endian integer out of a byte buffer.
  template <typename T>
  [[nodiscard]] T read_le(StringView data, usize offset) noexcept
  {
    T result{};
    for (usize i = 0; i < sizeof(T) && offset + i < data.size(); ++i)
    {
      result |= static_cast<T>(static_cast<u8>(data[offset + i])) << (i * 8);
    }
    return result;
  }

  [[nodiscard]] inline String upper(StringView text)
  {
    String out(text);
    for (char &c : out)
    {
      if (c >= 'a' && c <= 'z')
        c = static_cast<char>(c - 'a' + 'A');
    }
    return out;
  }

  [[nodiscard]] inline String lower(StringView text)
  {
    String out(text);
    for (char &c : out)
    {
      if (c >= 'A' && c <= 'Z')
        c = static_cast<char>(c - 'A' + 'a');
    }
    return out;
  }

  [[nodiscard]] inline String trim(StringView text)
  {
    usize begin = 0;
    usize end = text.size();
    auto space = [](char c)
    {
      return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    };
    while (begin < end && space(text[begin]))
      ++begin;
    while (end > begin && space(text[end - 1]))
      --end;
    return String(text.substr(begin, end - begin));
  }
  [[nodiscard]] inline bool starts_with(StringView text, StringView prefix)
  {
    return text.size() >= prefix.size() &&
           text.compare(0, prefix.size(), prefix) == 0;
  }

  [[nodiscard]] inline bool ends_with(StringView text, StringView suffix)
  {
    return text.size() >= suffix.size() &&
           text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
  }

  [[nodiscard]] inline String replace_all(StringView text, StringView from, StringView to)
  {
    String out;
    out.reserve(text.size());
    usize cursor = 0;
    while (cursor < text.size())
    {
      const usize hit = from.empty() ? StringView::npos : text.find(from, cursor);
      if (hit == StringView::npos)
      {
        out.append(text.substr(cursor));
        break;
      }
      out.append(text.substr(cursor, hit - cursor));
      out.append(to);
      cursor = hit + from.size();
    }
    return out;
  }

  [[nodiscard]] inline String pad_right(StringView text, usize width)
  {
    if (text.size() >= width)
      return String(text);
    return String(width - text.size(), ' ') + String(text);
  }

  [[nodiscard]] inline String column(StringView text, usize width)
  {
    if (text.size() >= width)
      return String(text);
    return String(text) + String(width - text.size(), ' ');
  }

} // namespace asmlex::core

namespace asmlex
{

  // Re-export the core aliases one level up so the sibling namespaces
  // (asmlex::lexer, asmlex::asm_, ...) can use them unqualified.  Unqualified
  // lookup from asmlex::lexer never searches the sibling asmlex::core, so
  // without this bridge every bare "usize" / "String" in those headers is an
  // unresolved identifier.
  using core::u8;
  using core::u16;
  using core::u32;
  using core::u64;
  using core::i8;
  using core::i16;
  using core::i32;
  using core::i64;
  using core::f32;
  using core::f64;
  using core::usize;
  using core::uptr;
  using core::Id;
  using core::String;
  using core::StringView;

  // Free helpers from core/Types.hpp.
  using core::lo16;
  using core::hi16;
  using core::lo8;
  using core::hi8;
  using core::push_le;
  using core::read_le;
  using core::upper;
  using core::lower;
  using core::trim;
  using core::starts_with;
  using core::ends_with;
  using core::replace_all;
  using core::pad_right;
  using core::column;

} // namespace asmlex
