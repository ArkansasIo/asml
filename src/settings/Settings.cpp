// ============================================================================
//  ASMLEX - src/settings/Settings.cpp
//  JSON round-trip for the user preferences blob.
// ============================================================================
#include <asmlex/settings/Settings.hpp>

#include <asmlex/core/FileSystem.hpp>

#include <json/json.h>

#include <algorithm>

namespace asmlex::settings
{

  bool theme_from_string(StringView text, Theme &out) noexcept
  {
    const String key = lower(text);
    if (key == "dark")
    {
      out = Theme::Dark;
      return true;
    }
    if (key == "light")
    {
      out = Theme::Light;
      return true;
    }
    if (key == "high_contrast" || key == "highcontrast")
    {
      out = Theme::HighContrast;
      return true;
    }
    return false;
  }

  String default_settings_path()
  {
    const char *profile = std::getenv("APPDATA");
    if (profile == nullptr || *profile == '\0')
      return "asmlex-settings.json";
    return core::path_join(core::path_join(profile, "ASMLEX"), "settings.json");
  }

  Settings load(StringView path, String *error)
  {
    Settings settings;
    const core::FileReadResult read = core::read_file(path);
    if (!read.ok)
    {
      if (error)
        *error = read.error;
      return settings;
    }

    Json::CharReaderBuilder builder;
    Json::Value root;
    String parse_errors;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(read.contents.data(), read.contents.data() + read.contents.size(), &root,
                       &parse_errors))
    {
      if (error)
        *error = parse_errors;
      return settings;
    }

    if (root.isMember("theme"))
    {
      Theme parsed = settings.theme;
      if (theme_from_string(root["theme"].asString(), parsed))
        settings.theme = parsed;
    }
    settings.editor_font = root.get("editor_font", settings.editor_font).asString();
    settings.editor_font_size = root.get("editor_font_size", settings.editor_font_size).asUInt();
    settings.word_wrap = root.get("word_wrap", settings.word_wrap).asBool();
    settings.show_line_numbers = root.get("show_line_numbers", settings.show_line_numbers).asBool();
    settings.show_hex_column = root.get("show_hex_column", settings.show_hex_column).asBool();
    settings.show_address_column =
        root.get("show_address_column", settings.show_address_column).asBool();
    settings.auto_save = root.get("auto_save", settings.auto_save).asBool();
    settings.auto_save_seconds = root.get("auto_save_seconds", settings.auto_save_seconds).asUInt();
    settings.recent_limit = root.get("recent_limit", settings.recent_limit).asUInt();

    const Json::Value &recent = root["recent_projects"];
    if (recent.isArray())
    {
      for (const Json::Value &entry : recent)
      {
        if (entry.isString())
          settings.recent_projects.push_back(entry.asString());
      }
    }
    return settings;
  }

  bool save(const Settings &settings, StringView path, String *error)
  {
    Json::Value root(Json::objectValue);
    root["theme"] = String(to_string(settings.theme));
    root["editor_font"] = settings.editor_font;
    root["editor_font_size"] = settings.editor_font_size;
    root["word_wrap"] = settings.word_wrap;
    root["show_line_numbers"] = settings.show_line_numbers;
    root["show_hex_column"] = settings.show_hex_column;
    root["show_address_column"] = settings.show_address_column;
    root["auto_save"] = settings.auto_save;
    root["auto_save_seconds"] = settings.auto_save_seconds;
    root["recent_limit"] = settings.recent_limit;

    Json::Value recent(Json::arrayValue);
    for (const String &entry : settings.recent_projects)
      recent.append(entry);
    root["recent_projects"] = recent;

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "  ";
    const String text = Json::writeString(builder, root);

    String write_error;
    if (!core::write_file(path, text, &write_error))
    {
      if (error)
        *error = write_error;
      return false;
    }
    return true;
  }

  void push_recent(Settings &settings, StringView project)
  {
    const String key = core::normalize_slashes(project);
    auto &recent = settings.recent_projects;
    recent.erase(std::remove_if(recent.begin(), recent.end(),
                                [&](const String &e)
                                { return core::normalize_slashes(e) == key; }),
                 recent.end());
    recent.insert(recent.begin(), String(project));
    if (settings.recent_limit > 0 && recent.size() > settings.recent_limit)
      recent.resize(settings.recent_limit);
  }

} // namespace asmlex::settings
