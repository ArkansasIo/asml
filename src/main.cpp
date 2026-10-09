// ============================================================================
//  ASMLEX - src/main.cpp
//  Win32 entry point.  Keeps the window setup here and delegates every real
//  task to asmlex_core, which is what the test suite links against.
// ============================================================================
#include <asmlex/core/Log.hpp>
#include <asmlex/core/Version.hpp>
#include <asmlex/platform/Platform.hpp>
#include <asmlex/plugins/PluginHost.hpp>
#include <asmlex/settings/Settings.hpp>
#include <asmlex/ui/Command.hpp>

#include <cstdio>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace
{

  /// Build the command table for this session.  The window layer wires each
  /// action to a widget; here they only report that they fired.
  void register_commands(asmlex::ui::CommandRegistry &registry)
  {
    using namespace asmlex::ui;

    auto add = [&](asmlex::StringView id, asmlex::String title, asmlex::String category,
                   asmlex::String shortcut, bool checkable = false)
    {
      Command command;
      command.id = asmlex::String(id);
      command.title = std::move(title);
      command.category = std::move(category);
      command.shortcut = std::move(shortcut);
      command.checkable = checkable;
      command.action = [id]()
      { ASMLEX_LOG_INFO("command fired: ", id); };
      registry.add(std::move(command));
    };

    add(kCommandFileNew, "New Project", "File", "Ctrl+N");
    add(kCommandFileOpen, "Open Project...", "File", "Ctrl+O");
    add(kCommandFileSave, "Save", "File", "Ctrl+S");
    add(kCommandFileSaveAs, "Save As...", "File", "Ctrl+Shift+S");
    add(kCommandFileClose, "Close Project", "File", "Ctrl+W");
    add(kCommandEditUndo, "Undo", "Edit", "Ctrl+Z");
    add(kCommandEditRedo, "Redo", "Edit", "Ctrl+Y");
    add(kCommandEditFind, "Find...", "Edit", "Ctrl+F");
    add(kCommandBuildAssemble, "Assemble", "Build", "F7");
    add(kCommandBuildDisassemble, "Disassemble", "Build", "Shift+F7");
    add(kCommandBuildRun, "Run", "Build", "F5");
    add(kCommandBuildStop, "Stop", "Build", "Shift+F5");
    add(kCommandViewToggleHex, "Hex View", "View", "Ctrl+H", true);
    add(kCommandViewToggleTypes, "Type Editor", "View", "Ctrl+T", true);
    add(kCommandViewToggleLog, "Output Log", "View", "Ctrl+L", true);
    add(kCommandHelpAbout, "About ASMLEX", "Help", "F1");
  }

} // namespace

int main(int argc, char **argv)
{
  asmlex::core::Logger &logger = asmlex::core::Logger::instance();
  logger.set_min_level(asmlex::core::LogLevel::Info);
  logger.add_sink("console", [](const asmlex::core::LogRecord &record)
                  { std::fprintf(stdout, "[%s] %s\n",
                               asmlex::String(asmlex::core::to_string(record.level)).c_str(),
                               record.message.c_str()); });

  ASMLEX_LOG_INFO(asmlex::core::version_banner(), " starting");
  ASMLEX_LOG_INFO("platform timestamp: ", asmlex::platform::local_timestamp());

  asmlex::ui::CommandRegistry registry;
  register_commands(registry);
  ASMLEX_LOG_INFO("registered ", registry.all().size(), " commands");

  asmlex::plugins::PluginHost plugins;
  plugins.scan("plugins");
  plugins.set_command_handler(
      [](const asmlex::plugins::PluginCommand &command)
      {
        ASMLEX_LOG_INFO("plugin command: ", command.id);
      });
  plugins.load_all();

  asmlex::settings::Settings prefs =
      asmlex::settings::load(asmlex::settings::default_settings_path());

  if (argc > 1)
  {
    // Headless mode: run the command named by argv[1], then leave.
    const bool ran = registry.execute(argv[1]);
    ASMLEX_LOG_INFO(ran ? "command completed" : "unknown command: ", argv[1]);
    plugins.unload_all();
    return ran ? 0 : 1;
  }

  ASMLEX_LOG_INFO("interactive mode is not available in this build (core only)");
  plugins.unload_all();
  return 0;
}
