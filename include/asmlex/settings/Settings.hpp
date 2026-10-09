// ============================================================================
//  ASMLEX - include/asmlex/settings/Settings.hpp
//  Persisted user preferences: theme, editor font, disassembly options and
//  the list of recently opened projects.  Stored as JSON in the user profile.
// ============================================================================
#pragma once

#include <asmlex/core/Types.hpp>

#include <vector>

namespace asmlex::settings
{

  enum class Theme
  {
    Dark,
    Light,
    HighContrast
  };

  [[nodiscard]] inline StringView to_string(Theme theme) noexcept
  {
    switch (theme)
    {
    case Theme::Dark:
      return "dark";
    case Theme::Light:
      return "light";
    case Theme::HighContrast:
      return "high_contrast";
    }
    return "dark";
  }

  [[nodiscard]] bool theme_from_string(StringView text, Theme &out) noexcept;

  /// Everything the IDE remembers between runs.
  struct Settings
  {
    Theme theme = Theme::Dark;
    String editor_font = "Consolas";
    u32 editor_font_size = 12;
    bool word_wrap = false;
    bool show_line_numbers = true;
    bool show_hex_column = true;
    bool show_address_column = true;
    bool auto_save = true;
    u32 auto_save_seconds = 120;
    u32 recent_limit = 10;
    std::vector<String> recent_projects;
  };

  /// Absolute path of the settings file inside the user profile.
  [[nodiscard]] String default_settings_path();

  /// Load settings from `path`.  On any error the defaults are returned and
  /// `error` (when given) explains why.
  [[nodiscard]] Settings load(StringView path, String *error = nullptr);

  /// Serialize `settings` to `path`, creating parent directories as needed.
  bool save(const Settings &settings, StringView path, String *error = nullptr);

  /// Insert `project` at the front of the recent list, de-duplicating and
  /// trimming to `recent_limit`.
  void push_recent(Settings &settings, StringView project);

} // namespace asmlex::settings
