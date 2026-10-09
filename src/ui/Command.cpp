// ============================================================================
//  ASMLEX - src/ui/Command.cpp
//  Command registry backing the menu bar, toolbar and command palette.
// ============================================================================
#include <asmlex/ui/Command.hpp>

#include <algorithm>

namespace asmlex::ui
{

  void CommandRegistry::add(Command command)
  {
    const auto it = std::find_if(commands_.begin(), commands_.end(),
                                 [&](const Command &c)
                                 { return c.id == command.id; });
    if (it != commands_.end())
      *it = std::move(command);
    else
      commands_.push_back(std::move(command));
  }

  bool CommandRegistry::remove(StringView id)
  {
    const auto it = std::find_if(commands_.begin(), commands_.end(),
                                 [&](const Command &c)
                                 { return c.id == id; });
    if (it == commands_.end())
      return false;
    commands_.erase(it);
    return true;
  }

  bool CommandRegistry::execute(StringView id)
  {
    const Command *command = find(id);
    if (command == nullptr || !command->enabled || !command->action)
      return false;
    command->action();
    return true;
  }

  Command *CommandRegistry::find(StringView id)
  {
    const auto it = std::find_if(commands_.begin(), commands_.end(),
                                 [&](const Command &c)
                                 { return c.id == id; });
    return it == commands_.end() ? nullptr : &*it;
  }

  const Command *CommandRegistry::find(StringView id) const
  {
    const auto it = std::find_if(commands_.begin(), commands_.end(),
                                 [&](const Command &c)
                                 { return c.id == id; });
    return it == commands_.end() ? nullptr : &*it;
  }

  void CommandRegistry::set_enabled(StringView id, bool enabled)
  {
    if (Command *command = find(id))
      command->enabled = enabled;
  }

  bool CommandRegistry::toggle(StringView id)
  {
    Command *command = find(id);
    if (command == nullptr || !command->checkable)
      return false;
    command->checked = !command->checked;
    return command->checked;
  }

  std::vector<const Command *> CommandRegistry::search(StringView needle) const
  {
    const String key = lower(needle);
    std::vector<const Command *> hits;
    for (const Command &command : commands_)
    {
      if (key.empty() || lower(command.title).find(key) != String::npos ||
          lower(command.id).find(key) != String::npos ||
          lower(command.category).find(key) != String::npos)
      {
        hits.push_back(&command);
      }
    }
    return hits;
  }

} // namespace asmlex::ui
