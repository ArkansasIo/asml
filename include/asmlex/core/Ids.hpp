// ============================================================================
//  ASMLEX - include/asmlex/core/Ids.hpp
//  Stable string identifiers for every named entity and generic id helpers.
// ============================================================================
#pragma once

#include <asmlex/core/Types.hpp>

#include <functional>
#include <string_view>
#include <utility>

namespace asmlex::core
{

  /// A stable, string based identifier.  Identifiers are intentionally strings
  /// rather than integers so they survive serialisation to JSON and can be
  /// surfaced verbatim in the UI (symbols, labels, nodes, panes, ...).
  class Identifier
  {
  public:
    Identifier() = default;
    Identifier(String text) : value_(std::move(text)) {}
    Identifier(const char *text) : value_(text ? text : "") {}

    [[nodiscard]] const String &value() const noexcept { return value_; }
    [[nodiscard]] bool empty() const noexcept { return value_.empty(); }
    [[nodiscard]] StringView view() const noexcept { return value_; }

    [[nodiscard]] bool operator==(const Identifier &other) const noexcept
    {
      return value_ == other.value_;
    }
    [[nodiscard]] bool operator!=(const Identifier &other) const noexcept
    {
      return value_ != other.value_;
    }
    [[nodiscard]] bool operator<(const Identifier &other) const noexcept
    {
      return value_ < other.value_;
    }
    [[nodiscard]] operator const String &() const noexcept { return value_; }

    /// Case-insensitive identity, used for assembly labels and mnemonics.
    [[nodiscard]] bool equals_ci(StringView other) const noexcept
    {
      return upper(value_) == upper(other);
    }

  private:
    String value_{};
  };

  [[nodiscard]] inline Identifier make_id(StringView text)
  {
    return Identifier(String(text));
  }

  /// Namespaced identity, e.g. ("asmlex.lexer", "Opcode").
  [[nodiscard]] inline Identifier qualify_id(StringView ns, StringView name)
  {
    String out(ns);
    out += '.';
    out += name;
    return Identifier(std::move(out));
  }

  /// Hash functor so identifiers can be used in unordered containers.
  struct IdentifierHash
  {
    [[nodiscard]] std::size_t operator()(const Identifier &id) const noexcept
    {
      return std::hash<String>{}(id.value());
    }
    [[nodiscard]] std::size_t operator()(StringView id) const noexcept
    {
      return std::hash<StringView>{}(id);
    }
  };

  /// Monotonic counter used to mint unique node / document ids.
  class IdAllocator
  {
  public:
    [[nodiscard]] u64 next() noexcept { return ++counter_; }
    [[nodiscard]] u64 peek() const noexcept { return counter_; }
    void reset() noexcept { counter_ = 0; }

  private:
    u64 counter_ = 0;
  };

} // namespace asmlex::core
