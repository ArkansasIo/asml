// ============================================================================
//  ASMLEX - include/asmlex/lexer/SourceManager.hpp
//  Owns the in-memory text of every open document and hands out stable views.
//  Line index tables let the editor map offsets to (line, column) cheaply.
// ============================================================================
#pragma once

#include <asmlex/core/Diagnostics.hpp>
#include <asmlex/core/Types.hpp>

#include <optional>
#include <vector>

namespace asmlex::lexer
{

  /// A single loaded document.
  struct SourceFile
  {
    String path;                     ///< absolute path, or "<memory>" for scratch text
    String text;                     ///< full contents
    std::vector<usize> line_start{}; ///< byte offset of each line's first char

    [[nodiscard]] usize line_count() const noexcept { return line_start.size(); }
  };

  /// Central registry of open documents.  Documents are addressed by an opaque
  /// handle (their index), so the UI can hold references without owning text.
  class SourceManager
  {
  public:
    using Handle = usize;
    static constexpr Handle kInvalid = static_cast<Handle>(-1);

    /// Register `text` under `path` and return its handle.
    Handle add(String path, String text);

    /// Replace the text of an existing document and rebuild its line index.
    bool update(Handle handle, String text);

    /// Drop a document.  Handles to other documents remain valid.
    bool remove(Handle handle);

    [[nodiscard]] bool contains(Handle handle) const noexcept
    {
      return handle < files_.size() && files_[handle].has_value();
    }

    [[nodiscard]] const SourceFile *find(Handle handle) const noexcept
    {
      return contains(handle) ? &*files_[handle] : nullptr;
    }

    [[nodiscard]] SourceFile *find(Handle handle) noexcept
    {
      return contains(handle) ? &*files_[handle] : nullptr;
    }

    [[nodiscard]] StringView text(Handle handle) const noexcept
    {
      const SourceFile *file = find(handle);
      return file ? StringView(file->text) : StringView{};
    }

    [[nodiscard]] const String &path(Handle handle) const noexcept
    {
      static const String kEmpty;
      const SourceFile *file = find(handle);
      return file ? file->path : kEmpty;
    }

    /// Handle of the document stored at `path`, or kInvalid when not open.
    [[nodiscard]] Handle find_by_path(StringView path) const noexcept;

    [[nodiscard]] usize size() const noexcept { return files_.size(); }

    /// Convert a byte offset into a 1-based (line, column) pair.
    [[nodiscard]] SourcePos position_of(Handle handle, usize offset) const noexcept;

    /// Byte offset of the first character of 1-based `line`.
    [[nodiscard]] usize offset_of_line(Handle handle, u32 line) const noexcept;

    /// The full text of 1-based `line`, without its terminator.
    [[nodiscard]] StringView line_text(Handle handle, u32 line) const noexcept;

    /// Build a span covering the whole document (used for coarse diagnostics).
    [[nodiscard]] SourceSpan whole_span(Handle handle) const;

    /// Recompute the line-start table of `file` from its `text`.
    static void rebuild_lines(SourceFile &file);

  private:
    std::vector<std::optional<SourceFile>> files_{};
  };

} // namespace asmlex::lexer
