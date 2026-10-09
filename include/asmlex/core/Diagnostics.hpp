// ============================================================================
//  ASMLEX - include/asmlex/core/Diagnostics.hpp
//  Source spans, diagnostics and the diagnostic sink used across the tools.
// ============================================================================
#pragma once

#include <asmlex/core/Types.hpp>

#include <cstdio>
#include <functional>
#include <vector>

namespace asmlex::core
{

  struct SourcePos
  {
    u32 line = 1;
    u32 column = 1;
    usize offset = 0;
  };

  struct SourceSpan
  {
    String file;
    SourcePos begin{};
    SourcePos end{};

    [[nodiscard]] bool valid() const noexcept
    {
      return !file.empty() || end.offset > begin.offset;
    }
  };

  enum class Severity
  {
    Note,
    Info,
    Warning,
    Error,
    Fatal
  };

  [[nodiscard]] inline StringView to_string(Severity s) noexcept
  {
    switch (s)
    {
    case Severity::Note:
      return "note";
    case Severity::Info:
      return "info";
    case Severity::Warning:
      return "warning";
    case Severity::Error:
      return "error";
    case Severity::Fatal:
      return "fatal";
    }
    return "unknown";
  }

  struct Diagnostic
  {
    Severity severity = Severity::Error;
    String code; ///< stable machine readable code, e.g. "A1023"
    String text; ///< human readable message
    SourceSpan where{};
    String hint; ///< optional suggested fix

    [[nodiscard]] bool is_error() const noexcept
    {
      return severity == Severity::Error || severity == Severity::Fatal;
    }
  };

  /// Collects diagnostics and mirrors them to stdout for headless runs.
  class DiagnosticSink
  {
  public:
    using Observer = std::function<void(const Diagnostic &)>;

    void report(Diagnostic d)
    {
      if (d.is_error())
        ++error_count_;
      else if (d.severity == Severity::Warning)
        ++warning_count_;
      items_.push_back(std::move(d));
      const Diagnostic &stored = items_.back();
      if (observer_)
        observer_(stored);
    }

    void error(String code, String text, SourceSpan where = {})
    {
      report({Severity::Error, std::move(code), std::move(text), std::move(where), {}});
    }

    void warning(String code, String text, SourceSpan where = {})
    {
      report({Severity::Warning, std::move(code), std::move(text), std::move(where), {}});
    }

    void info(String code, String text)
    {
      report({Severity::Info, std::move(code), std::move(text), {}, {}});
    }

    void note(String code, String text)
    {
      report({Severity::Note, std::move(code), std::move(text), {}, {}});
    }

    void set_observer(Observer o) { observer_ = std::move(o); }

    [[nodiscard]] const std::vector<Diagnostic> &items() const noexcept { return items_; }
    [[nodiscard]] usize error_count() const noexcept { return error_count_; }
    [[nodiscard]] usize warning_count() const noexcept { return warning_count_; }
    [[nodiscard]] bool ok() const noexcept { return error_count_ == 0; }

    void clear() noexcept
    {
      items_.clear();
      error_count_ = 0;
      warning_count_ = 0;
    }

    /// Renders a diagnostic as "file:line:col: severity: text".
    [[nodiscard]] static String format(const Diagnostic &d)
    {
      String out;
      if (!d.where.file.empty())
      {
        out += d.where.file;
        out += ':';
        out += std::to_string(d.where.begin.line);
        out += ':';
        out += std::to_string(d.where.begin.column);
        out += ": ";
      }
      out += to_string(d.severity);
      out += ": ";
      out += d.text;
      if (!d.code.empty())
      {
        out += " [";
        out += d.code;
        out += ']';
      }
      return out;
    }

    void dump(FILE *stream = stdout) const
    {
      for (const Diagnostic &d : items_)
      {
        std::fprintf(stream, "%s\n", format(d).c_str());
      }
    }

  private:
    std::vector<Diagnostic> items_{};
    Observer observer_{};
    usize error_count_ = 0;
    usize warning_count_ = 0;
  };

  /// Install an observer that mirrors every diagnostic to the ASMLEX logger.
  void install_logging_observer(DiagnosticSink &sink);

  /// Render a diagnostic together with its source line and a caret underline.
  [[nodiscard]] String render_diagnostic(const Diagnostic &d, StringView source);

} // namespace asmlex::core

namespace asmlex
{

  // Re-export the diagnostic types one level up, for the same reason the value
  // aliases in core/Types.hpp are re-exported: sibling namespaces such as
  // asmlex::lexer cannot see asmlex::core by unqualified lookup.
  using core::SourcePos;
  using core::SourceSpan;
  using core::Severity;
  using core::Diagnostic;
  using core::DiagnosticSink;
  using core::install_logging_observer;
  using core::render_diagnostic;

} // namespace asmlex
