// ============================================================================
//  ASMLEX - include/asmlex/types/TypeSystem.hpp
//  Describes the data types an assembler project can define: scalar integer
//  widths, floating point widths, pointers, arrays and user aggregates.
//  The disassembler consults this table to name recovered memory regions.
// ============================================================================
#pragma once

#include <asmlex/core/Types.hpp>

#include <vector>

namespace asmlex::types
{

  /// Kind of a declared type.  `Alias` names another type, `Struct`/`Union`
  /// group fields, everything else is a fixed-width primitive.
  enum class TypeKind
  {
    Void,
    Integer,
    Float,
    Pointer,
    Array,
    Struct,
    Union,
    Alias
  };

  [[nodiscard]] inline StringView to_string(TypeKind kind) noexcept
  {
    switch (kind)
    {
    case TypeKind::Void:
      return "void";
    case TypeKind::Integer:
      return "int";
    case TypeKind::Float:
      return "float";
    case TypeKind::Pointer:
      return "ptr";
    case TypeKind::Array:
      return "array";
    case TypeKind::Struct:
      return "struct";
    case TypeKind::Union:
      return "union";
    case TypeKind::Alias:
      return "alias";
    }
    return "void";
  }

  /// One member of a struct/union, or one element of an array.
  struct TypeField
  {
    Id name;
    Id type;
    u32 array_count = 1; ///< > 1 only for inline fixed arrays.
    u32 offset = 0;      ///< Byte offset inside the parent, when known.
  };

  /// A single entry in the type table.
  struct TypeDef
  {
    Id name;
    TypeKind kind = TypeKind::Integer;
    u32 size = 0; ///< Size in bytes; 0 for void and forward declarations.
    bool is_signed = false;
    Id alias_of;     ///< Valid when kind == Alias.
    Id element_type; ///< Valid when kind == Array / Pointer.
    std::vector<TypeField> fields;
  };

  /// Registry of all types known to the current project.
  class TypeSystem
  {
  public:
    /// Install the built-in primitives (u8..u64, i8..i64, f32/f64, ptr).
    void load_builtins();

    /// Add or replace a definition.  Returns false when the definition is
    /// malformed (empty name, unknown field type).
    bool define(TypeDef def);

    /// Look up a definition by name; nullptr when unknown.
    [[nodiscard]] const TypeDef *find(StringView name) const;

    /// True when a definition with this name exists.
    [[nodiscard]] bool contains(StringView name) const { return find(name) != nullptr; }

    /// Remove one definition.  Built-ins cannot be removed.
    bool erase(StringView name);

    [[nodiscard]] const std::vector<TypeDef> &all() const noexcept { return types_; }

    [[nodiscard]] usize count() const noexcept { return types_.size(); }

    void clear();

  private:
    std::vector<TypeDef> types_;
  };

  /// Format a recovered byte sequence as `NAME field = value` lines using the
  /// given type.  Used by the disassembler's data view.
  [[nodiscard]] String format_value(const TypeSystem &types, StringView type_name,
                                    const String &bytes);

} // namespace asmlex::types
