// ============================================================================
//  ASMLEX - src/lexer/SourceManager.cpp
//  Document registry plus offset <-> (line, column) translation.
// ============================================================================
#include <asmlex/lexer/SourceManager.hpp>

namespace asmlex::lexer
{

  void SourceManager::rebuild_lines(SourceFile &file)
  {
    file.line_start.clear();
    file.line_start.push_back(0);
    for (usize i = 0; i < file.text.size(); ++i)
    {
      if (file.text[i] == '\n')
        file.line_start.push_back(i + 1);
    }
    // An empty document still has one (empty) line.
    if (file.line_start.empty())
      file.line_start.push_back(0);
  }

  SourceManager::Handle SourceManager::add(String path, String text)
  {
    SourceFile file;
    file.path = std::move(path);
    file.text = std::move(text);
    rebuild_lines(file);

    // Reuse a hole left by remove() so handles stay dense.
    for (usize i = 0; i < files_.size(); ++i)
    {
      if (!files_[i].has_value())
      {
        files_[i] = std::move(file);
        return i;
      }
    }
    files_.push_back(std::move(file));
    return files_.size() - 1;
  }

  bool SourceManager::update(Handle handle, String text)
  {
    SourceFile *file = find(handle);
    if (!file)
      return false;
    file->text = std::move(text);
    rebuild_lines(*file);
    return true;
  }

  bool SourceManager::remove(Handle handle)
  {
    if (!contains(handle))
      return false;
    files_[handle].reset();
    return true;
  }

  SourceManager::Handle SourceManager::find_by_path(StringView path) const noexcept
  {
    const String wanted = core::normalize_slashes(path);
    for (usize i = 0; i < files_.size(); ++i)
    {
      if (!files_[i].has_value())
        continue;
      if (core::normalize_slashes(files_[i]->path) == wanted)
        return i;
    }
    return kInvalid;
  }

  SourcePos SourceManager::position_of(Handle handle, usize offset) const noexcept
  {
    SourcePos pos;
    const SourceFile *file = find(handle);
    if (!file)
    {
      pos.offset = offset;
      return pos;
    }

    if (offset > file->text.size())
      offset = file->text.size();

    // Binary search for the last line whose start is <= offset.
    usize lo = 0;
    usize hi = file->line_start.size();
    while (lo + 1 < hi)
    {
      const usize mid = (lo + hi) / 2;
      if (file->line_start[mid] <= offset)
        lo = mid;
      else
        hi = mid;
    }

    pos.line = static_cast<u32>(lo + 1);
    pos.column = static_cast<u32>(offset - file->line_start[lo] + 1);
    pos.offset = offset;
    return pos;
  }

  usize SourceManager::offset_of_line(Handle handle, u32 line) const noexcept
  {
    const SourceFile *file = find(handle);
    if (!file || line == 0 || line > file->line_start.size())
      return 0;
    return file->line_start[line - 1];
  }

  StringView SourceManager::line_text(Handle handle, u32 line) const noexcept
  {
    const SourceFile *file = find(handle);
    if (!file || line == 0 || line > file->line_start.size())
      return {};

    const usize begin = file->line_start[line - 1];
    usize end = (line < file->line_start.size()) ? file->line_start[line] : file->text.size();
    while (end > begin && (file->text[end - 1] == '\n' || file->text[end - 1] == '\r'))
      --end;
    return StringView(file->text).substr(begin, end - begin);
  }

  SourceSpan SourceManager::whole_span(Handle handle) const
  {
    SourceSpan span;
    const SourceFile *file = find(handle);
    if (!file)
      return span;

    span.file = file->path;
    span.begin = SourcePos{1, 1, 0};
    span.end = position_of(handle, file->text.size());
    return span;
  }

} // namespace asmlex::lexer
