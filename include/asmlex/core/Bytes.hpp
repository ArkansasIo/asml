// ============================================================================
//  ASMLEX - include/asmlex/core/Bytes.hpp
//  Byte buffer helper shared by the assembler, disassembler and the VM.
// ============================================================================
#pragma once

#include <asmlex/core/Diagnostics.hpp>
#include <asmlex/core/Types.hpp>

#include <utility>
#include <vector>

namespace asmlex::core
{

  /// A growable little-endian byte buffer.
  class ByteBuffer
  {
  public:
    ByteBuffer() = default;
    explicit ByteBuffer(String bytes) : data_(std::move(bytes)) {}

    [[nodiscard]] usize size() const noexcept { return data_.size(); }
    [[nodiscard]] bool empty() const noexcept { return data_.empty(); }
    [[nodiscard]] const String &str() const noexcept { return data_; }
    [[nodiscard]] String &str() noexcept { return data_; }

    void clear() noexcept { data_.clear(); }
    void reserve(usize n) { data_.reserve(n); }

    void push_u8(u8 v) { data_.push_back(static_cast<char>(v)); }

    void push_u16(u16 v)
    {
      push_u8(lo8(v));
      push_u8(hi8(v));
    }

    void push_u32(u32 v)
    {
      push_u16(lo16(v));
      push_u16(hi16(v));
    }

    void push_u64(u64 v)
    {
      push_u32(static_cast<u32>(v & 0xFFFFFFFFu));
      push_u32(static_cast<u32>(v >> 32));
    }

    void push_i8(i8 v) { push_u8(static_cast<u8>(v)); }
    void push_i16(i16 v) { push_u16(static_cast<u16>(v)); }
    void push_i32(i32 v) { push_u32(static_cast<u32>(v)); }
    void push_i64(i64 v) { push_u64(static_cast<u64>(v)); }
    void push_f32(f32 v) { push_u32(bit_cast32(v)); }
    void push_f64(f64 v) { push_u64(bit_cast64(v)); }

    void append(StringView bytes) { data_.append(bytes); }

    /// Overwrite `size` bytes at `offset` (used for back-patching jump targets).
    void patch(usize offset, const String &bytes)
    {
      for (usize i = 0; i < bytes.size() && offset + i < data_.size(); ++i)
      {
        data_[offset + i] = bytes[i];
      }
    }

    void patch_u32(usize offset, u32 v)
    {
      String tmp;
      push_le<u32>(tmp, v);
      patch(offset, tmp);
    }

    [[nodiscard]] u8 at(usize i) const noexcept
    {
      return i < data_.size() ? static_cast<u8>(data_[i]) : 0u;
    }

    [[nodiscard]] u16 u16_at(usize i) const noexcept { return read_le<u16>(data_, i); }
    [[nodiscard]] u32 u32_at(usize i) const noexcept { return read_le<u32>(data_, i); }
    [[nodiscard]] u64 u64_at(usize i) const noexcept { return read_le<u64>(data_, i); }
    [[nodiscard]] i8 i8_at(usize i) const noexcept { return static_cast<i8>(at(i)); }
    [[nodiscard]] i16 i16_at(usize i) const noexcept { return static_cast<i16>(u16_at(i)); }
    [[nodiscard]] i32 i32_at(usize i) const noexcept { return static_cast<i32>(u32_at(i)); }
    [[nodiscard]] i64 i64_at(usize i) const noexcept { return static_cast<i64>(u64_at(i)); }
    [[nodiscard]] f32 f32_at(usize i) const noexcept { return from_bits32(u32_at(i)); }
    [[nodiscard]] f64 f64_at(usize i) const noexcept { return from_bits64(u64_at(i)); }

    /// Slice out `count` bytes starting at `offset`.
    [[nodiscard]] String slice(usize offset, usize count) const
    {
      if (offset >= data_.size())
        return {};
      const usize n = (count == npos_slice) ? (data_.size() - offset) : count;
      const usize avail = data_.size() - offset;
      return data_.substr(offset, n < avail ? n : avail);
    }

    static constexpr usize npos_slice = static_cast<usize>(-1);

    [[nodiscard]] const std::vector<u8> to_vector() const
    {
      std::vector<u8> out;
      out.reserve(data_.size());
      for (char c : data_)
        out.push_back(static_cast<u8>(c));
      return out;
    }

  private:
    [[nodiscard]] static u32 bit_cast32(f32 v) noexcept
    {
      u32 out = 0;
      for (usize i = 0; i < sizeof(f32); ++i)
        reinterpret_cast<u8 *>(&out)[i] = reinterpret_cast<const u8 *>(&v)[i];
      return out;
    }
    [[nodiscard]] static u64 bit_cast64(f64 v) noexcept
    {
      u64 out = 0;
      for (usize i = 0; i < sizeof(f64); ++i)
        reinterpret_cast<u8 *>(&out)[i] = reinterpret_cast<const u8 *>(&v)[i];
      return out;
    }
    [[nodiscard]] static f32 from_bits32(u32 v) noexcept
    {
      f32 out = 0.0f;
      for (usize i = 0; i < sizeof(f32); ++i)
        reinterpret_cast<u8 *>(&out)[i] = reinterpret_cast<const u8 *>(&v)[i];
      return out;
    }
    [[nodiscard]] static f64 from_bits64(u64 v) noexcept
    {
      f64 out = 0.0;
      for (usize i = 0; i < sizeof(f64); ++i)
        reinterpret_cast<u8 *>(&out)[i] = reinterpret_cast<const u8 *>(&v)[i];
      return out;
    }

    String data_{};
  };

  /// Parse "48 8B C1" / "0x48,0x8B" / "\x48\x8B" into a byte string.
  [[nodiscard]] String parse_hex_bytes(StringView text, DiagnosticSink *sink = nullptr);

  /// Render a byte string as "48 8B C1".
  [[nodiscard]] String format_hex_bytes(StringView bytes, bool uppercase = true);

  /// Render a byte string as a classic 16-byte-per-line hex dump.
  [[nodiscard]] String hex_dump(StringView bytes, usize base_offset = 0);

} // namespace asmlex::core
