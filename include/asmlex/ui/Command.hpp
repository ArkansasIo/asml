// ============================================================================
//  ASMLEX - include/asmlex/ui/Command.hpp
//  The command palette model.  Every user action (menu item, toolbar button,
//  keyboard shortcut, plugin contribution) is a Command, so binding, undo
//  labels and the palette all read from one place.
// ============================================================================
#pragma once

#include <asmlex/core/Types.hpp>

#include <functional>
#include <vector>

namespace asmlex::ui
{

  /// Stable identifiers for the built-in commands.  Plugins use free-form
  /// strings so they never collide with these.
  inline constexpr StringView kCommandFileNew = "file.new";
  inline constexpr StringView kCommandFileOpen = "file.open";
  inline constexpr StringView kCommandFileSave = "file.save";
  inline constexpr StringView kCommandFileSaveAs = "file.save_as";
  inline constexpr StringView kCommandFileClose = "file.close";
  inline constexpr StringView kCommandEditUndo = "edit.undo";
  inline constexpr StringView kCommandEditRedo = "edit.redo";
  inline constexpr StringView kCommandEditFind = "edit.find";
  inline constexpr StringView kCommandBuildAssemble = "build.assemble";
  inline constexpr StringView kCommandBuildDisassemble = "build.disassemble";
  inline constexpr StringView kCommandBuildRun = "build.run";
  inline constexpr StringView kCommandBuildStop = "build.stop";
  inline constexpr StringView kCommandViewToggleHex = "view.toggle_hex";
  inline constexpr StringView kCommandViewToggleTypes = "view.toggle_types";
  inline constexpr StringView kCommandViewToggleLog = "view.toggle_log";
  inline constexpr StringView kCommandHelpAbout = "help.about";

  struct Command
  {
    Id id;
    String title;
    String category; ///< "File", "Edit", "Build", ... used for grouping.
    String shortcut; ///< Display form, e.g. "Ctrl+S".
    bool enabled = true;
    bool checkable = false;
    bool checked = false;
    std::function<void()> action;
  };

  /// Holds every registered command and dispatches execution by id.
  class CommandRegistry
  {
  public:
    /// Register `command`.  An existing command with the same id is replaced.
    void add(Command command);

    /// Remove a command by id; false when it was not registered.
    bool remove(StringView id);

    /// Run the command with this id; false when unknown or disabled.
    bool execute(StringView id);

    [[nodiscard]] Command *find(StringView id);
    [[nodiscard]] const Command *find(StringView id) const;

    /// Enable/disable a command without removing it.
    void set_enabled(StringView id, bool enabled);

    /// Flip a checkable command's checked flag and return the new value.
    bool toggle(StringView id);

    /// Commands whose title or id contains `needle` (case-insensitive).
    [[nodiscard]] std::vector<const Command *> search(StringView needle) const;

    [[nodiscard]] const std::vector<Command> &all() const noexcept { return commands_; }

  private:
    std::vector<Command> commands_;
  };

} // namespace asmlex::ui
