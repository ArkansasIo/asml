// ============================================================================
//  ASMLEX - src/types/TypeSystem.cpp
//  Storage and lookup for the project type table.
// ============================================================================
#include <asmlex/types/TypeSystem.hpp>

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace asmlex::types
{

  void TypeSystem::load_builtins()
  {
    auto primitive = [this](StringView name, u32 size, bool is_signed)
    {
      TypeDef def;
      def.name = String(name);
      def.kind = TypeKind::Integer;
      def.size = size;
      def.is_signed = is_signed;
      types_.push_back(std::move(def));
    };

    primitive("u8", 1, false);
    primitive("u16", 2, false);
    primitive("u32", 4, false);
    primitive("u64", 8, false);
    primitive("i8", 1, true);
    primitive("i16", 2, true);
    primitive("i32", 4, true);
    primitive("i64", 8, true);

    TypeDef f32;
    f32.name = "f32";
    f32.kind = TypeKind::Float;
    f32.size = 4;
    types_.push_back(std::move(f32));

    TypeDef f64;
    f64.name = "f64";
    f64.kind = TypeKind::Float;
    f64.size = 8;
    types_.push_back(std::move(f64));

    TypeDef ptr;
    ptr.name = "ptr";
    ptr.kind = TypeKind::Pointer;
    ptr.size = 8;
    types_.push_back(std::move(ptr));
  }

  bool TypeSystem::define(TypeDef def)
  {
    if (def.name.empty())
      return false;
    for (const TypeField &field : def.fields)
    {
      if (field.type.empty())
        return false;
    }

    const auto it = std::find_if(types_.begin(), types_.end(),
                                 [&](const TypeDef &t)
                                 { return t.name == def.name; });
    if (it != types_.end())
      *it = std::move(def);
    else
      types_.push_back(std::move(def));
    return true;
  }

  const TypeDef *TypeSystem::find(StringView name) const
  {
    const auto it = std::find_if(types_.begin(), types_.end(),
                                 [&](const TypeDef &t)
                                 { return t.name == name; });
    return it == types_.end() ? nullptr : &*it;
  }

  bool TypeSystem::erase(StringView name)
  {
    const auto it = std::find_if(types_.begin(), types_.end(),
                                 [&](const TypeDef &t)
                                 { return t.name == name; });
    if (it == types_.end())
      return false;
    types_.erase(it);
    return true;
  }

  void TypeSystem::clear() { types_.clear(); }

  String format_value(const TypeSystem &types, StringView type_name, const String &bytes)
  {
    const TypeDef *def = types.find(type_name);
    if (def == nullptr)
      return "unknown type '" + String(type_name) + "'";

    if (bytes.size() < def->size)
      return "short buffer (" + std::to_string(bytes.size()) + " < " +
             std::to_string(def->size) + ")";

    char buffer[64]{};
    switch (def->kind)
    {
    case TypeKind::Integer:
    {
      const u64 raw = asmlex::core::read_le<u64>(bytes, 0);
      if (def->is_signed)
        std::snprintf(buffer, sizeof(buffer), "%lld", static_cast<long long>(raw));
      else
        std::snprintf(buffer, sizeof(buffer), "0x%llX", static_cast<unsigned long long>(raw));
      return String(buffer);
    }
    case TypeKind::Float:
      if (def->size == 4)
      {
        const u32 bits = asmlex::core::read_le<u32>(bytes, 0);
        float value = 0.0f;
        std::memcpy(&value, &bits, sizeof(value));
        std::snprintf(buffer, sizeof(buffer), "%g", static_cast<double>(value));
      }
      else
      {
        const u64 bits = asmlex::core::read_le<u64>(bytes, 0);
        double value = 0.0;
        std::memcpy(&value, &bits, sizeof(value));
        std::snprintf(buffer, sizeof(buffer), "%g", value);
      }
      return String(buffer);
    case TypeKind::Pointer:
    {
      const u64 raw = asmlex::core::read_le<u64>(bytes, 0);
      std::snprintf(buffer, sizeof(buffer), "0x%llX", static_cast<unsigned long long>(raw));
      return String(buffer);
    }
    case TypeKind::Array:
    case TypeKind::Struct:
    case TypeKind::Union:
    case TypeKind::Alias:
    case TypeKind::Void:
      break;
    }
    return "{" + std::to_string(def->size) + " bytes}";
  }

} // namespace asmlex::types
