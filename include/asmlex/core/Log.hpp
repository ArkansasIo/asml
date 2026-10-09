// ============================================================================
//  ASMLEX - include/asmlex/core/Log.hpp
//  Tiny level-filtered logger. UI panes subscribe through sinks.
// ============================================================================
#pragma once

#include <asmlex/core/Types.hpp>

#include <functional>
#include <mutex>
#include <sstream>
#include <utility>
#include <vector>

namespace asmlex::core
{

  enum class LogLevel
  {
    Trace,
    Debug,
    Info,
    Warn,
    Error
  };

  [[nodiscard]] inline StringView to_string(LogLevel level) noexcept
  {
    switch (level)
    {
    case LogLevel::Trace:
      return "TRACE";
    case LogLevel::Debug:
      return "DEBUG";
    case LogLevel::Info:
      return "INFO";
    case LogLevel::Warn:
      return "WARN";
    case LogLevel::Error:
      return "ERROR";
    }
    return "?";
  }

  struct LogRecord
  {
    LogLevel level = LogLevel::Info;
    String channel = "asmlex";
    String message;
    u64 stamp_ms = 0;
  };

  class Logger
  {
  public:
    using Sink = std::function<void(const LogRecord &)>;

    static Logger &instance()
    {
      static Logger logger;
      return logger;
    }

    void add_sink(String name, Sink sink)
    {
      std::scoped_lock lock(mutex_);
      sinks_.push_back({std::move(name), std::move(sink)});
    }

    void clear_sinks()
    {
      std::scoped_lock lock(mutex_);
      sinks_.clear();
    }

    void set_min_level(LogLevel level) noexcept { min_level_ = level; }
    [[nodiscard]] LogLevel min_level() const noexcept { return min_level_; }

    void emit(LogRecord record)
    {
      if (static_cast<int>(record.level) < static_cast<int>(min_level_))
        return;
      record.stamp_ms = now_ms();
      std::scoped_lock lock(mutex_);
      history_.push_back(record);
      if (history_.size() > kHistoryLimit)
        history_.erase(history_.begin(), history_.begin() + (history_.size() - kHistoryLimit));
      for (auto &[name, sink] : sinks_)
      {
        (void)name;
        if (sink)
          sink(record);
      }
    }

    [[nodiscard]] const std::vector<LogRecord> &history() const noexcept { return history_; }

    void clear() { history_.clear(); }

    static u64 now_ms();

  private:
    static constexpr usize kHistoryLimit = 4000;

    struct SinkEntry
    {
      String name;
      Sink sink;
    };

    std::mutex mutex_{};
    std::vector<SinkEntry> sinks_{};
    std::vector<LogRecord> history_{};
    LogLevel min_level_ = LogLevel::Info;
  };

  /// Convenience helper: build and emit one log line.
  template <typename... Args>
  void log(LogLevel level, StringView channel, Args &&...args)
  {
    std::ostringstream stream;
    (stream << ... << std::forward<Args>(args));
    Logger::instance().emit({level, String(channel), stream.str(), 0});
  }

#define ASMLEX_LOG_INFO(...) ::asmlex::core::log(::asmlex::core::LogLevel::Info, "asmlex", __VA_ARGS__)
#define ASMLEX_LOG_WARN(...) ::asmlex::core::log(::asmlex::core::LogLevel::Warn, "asmlex", __VA_ARGS__)
#define ASMLEX_LOG_ERROR(...) ::asmlex::core::log(::asmlex::core::LogLevel::Error, "asmlex", __VA_ARGS__)

} // namespace asmlex::core
