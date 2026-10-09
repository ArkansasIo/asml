// ============================================================================
//  ASMLEX - include/asmlex/platform/Platform.hpp
//  The very small set of OS services the IDE needs: process launch, clipboard
//  access, message boxes and a monotonic clock.  Everything else goes through
//  core/FileSystem.hpp.
// ============================================================================
#pragma once

#include <asmlex/core/Types.hpp>

namespace asmlex::platform
{

  /// Result of launching a child process.
  struct ProcessResult
  {
    bool started = false;
    int exit_code = -1;
    String stdout_text;
    String error;
  };

  /// Run `command_line` (a full command, not argv) and wait for it to finish.
  /// Captures stdout separately from stderr for the build pane.
  [[nodiscard]] ProcessResult run_process(StringView command_line, StringView working_directory);

  /// Launch `command_line` detached and return immediately.  Used by "Run".
  bool spawn_process(StringView command_line, StringView working_directory, String *error = nullptr);

  /// Put `text` on the system clipboard, replacing its contents.
  bool set_clipboard_text(StringView text);

  /// Read the system clipboard as UTF-8; empty when it holds no text.
  [[nodiscard]] String clipboard_text();

  enum class MessageBoxKind
  {
    Info,
    Warning,
    Error
  };

  /// Modal message box owned by the IDE window.  Returns true when the user
  /// confirmed (OK / Yes).
  bool message_box(StringView title, StringView text, MessageBoxKind kind = MessageBoxKind::Info);

  /// Monotonic milliseconds since an unspecified epoch.  Safe to diff.
  [[nodiscard]] u64 monotonic_ms();

  /// Local wall-clock time formatted as "YYYY-MM-DD HH:MM:SS".
  [[nodiscard]] String local_timestamp();

} // namespace asmlex::platform
