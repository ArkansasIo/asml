// ============================================================================
//  ASMLEX - include/asmlex/core/FileSystem.hpp
//  Tiny platform layer for reading / writing source files and enumerating
//  directories.  Kept free of <filesystem> so the UI can drive it from any
//  thread without exceptions leaking into the message loop.
// ============================================================================
#pragma once

#include <asmlex/core/Types.hpp>

#include <vector>

namespace asmlex::core
{

  /// Result of a file read: `ok` is false when the path does not exist or is
  /// not readable.  `error` carries a human readable reason.
  struct FileReadResult
  {
    bool ok = false;
    String contents;
    String error;
  };

  /// Read an entire file into memory (binary safe).
  [[nodiscard]] FileReadResult read_file(StringView path);

  /// Write `contents` to `path`, replacing any existing file.  Returns false
  /// and fills `error` when the file cannot be created.
  bool write_file(StringView path, StringView contents, String *error = nullptr);

  /// True when `path` exists and is a regular file.
  [[nodiscard]] bool file_exists(StringView path);

  /// True when `path` exists and is a directory.
  [[nodiscard]] bool directory_exists(StringView path);

  /// File size in bytes, or 0 when the path cannot be stat-ed.
  [[nodiscard]] u64 file_size(StringView path);

  /// Last write time as unix milliseconds, or 0 when unavailable.
  [[nodiscard]] u64 file_modified_ms(StringView path);

  /// List the entries of `directory`; when `want_directories` is true only
  /// sub-directories are returned, otherwise only regular files.
  [[nodiscard]] std::vector<String> list_directory(StringView directory, bool want_directories);

  /// Filename including extension, e.g. "hello.asm".
  [[nodiscard]] String file_name(StringView path);

  /// File extension including the dot, lower-cased, e.g. ".asm".
  [[nodiscard]] String file_extension(StringView path);

  /// Strip the final extension, e.g. "C:/x/hello.asm" -> "C:/x/hello".
  [[nodiscard]] String strip_extension(StringView path);

  /// Directory portion of `path`, or "." when there is none.
  [[nodiscard]] String parent_directory(StringView path);

  /// Join two path fragments with the platform separator, collapsing a
  /// duplicate separator at the seam.
  [[nodiscard]] String path_join(StringView a, StringView b);

  /// Replace every '\' with '/' so paths compare identically on all hosts.
  [[nodiscard]] String normalize_slashes(StringView path);

} // namespace asmlex::core
