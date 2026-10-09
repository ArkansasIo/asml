// ============================================================================
//  ASMLEX - src/core/Diagnostics.cpp
//  Diagnostics translation unit.  The sink is header-only, but this file hosts
//  shared helpers used by every frontend: a console observer installer and a
//  small source-line renderer for caret diagnostics.
// ============================================================================
#include <asmlex/core/Diagnostics.hpp>
#include <asmlex/core/Log.hpp>

#include <cstdio>

namespace asmlex::core
{

  namespace
  {
    /// Returns the 1-based line `line_no` of `source`, or an empty view.
    [[nodiscard]] StringView source_line(StringView source, u32 line_no)
    {
      u32 current = 1;
      usize begin = 0;
      for (usize i = 0; i <= source.size(); ++i)
      {
        const bool eol = (i == source.size()) || source[i] == '\n';
        if (!eol)
          continue;
        if (current == line_no)
          return source.substr(begin, i - begin);
        ++current;
        begin = i + 1;
      }
      return {};
    }
  } // namespace

  /// Install an observer that mirrors every diagnostic to the ASMLEX logger so
  /// it shows up in the UI log pane and on stdout for headless runs.
  void install_logging_observer(DiagnosticSink &sink)
  {
    sink.set_observer([](const Diagnostic &d)
                      {
                        LogLevel level = LogLevel::Info;
                        switch (d.severity)
                        {
                        case Severity::Note:
                          level = LogLevel::Trace;
                          break;
                        case Severity::Info:
                          level = LogLevel::Info;
                          break;
                        case Severity::Warning:
                          level = LogLevel::Warn;
                          break;
                        case Severity::Error:
                        case Severity::Fatal:
                          level = LogLevel::Error;
                          break;
                        }
                        log(level, "diag", DiagnosticSink::format(d)); });
  }

  /// Render a diagnostic together with the offending source line and a caret.
  [[nodiscard]] String render_diagnostic(const Diagnostic &d, StringView source)
  {
    String out = DiagnosticSink::format(d);
    if (d.where.begin.line == 0)
      return out;

    const StringView line = source_line(source, d.where.begin.line);
    if (line.empty())
      return out;

    out += '\n';
    out += "    ";
    out += line;
    out += '\n';
    out += "    ";

    const u32 col = d.where.begin.column > 0 ? d.where.begin.column - 1 : 0;
    for (u32 i = 0; i < col; ++i)
      out += (i < line.size() && line[i] == '\t') ? '\t' : ' ';
    out += '^';

    const u32 span = (d.where.end.column > d.where.begin.column)
                         ? (d.where.end.column - d.where.begin.column)
                         : 1;
    for (u32 i = 1; i < span; ++i)
      out += '~';

    if (!d.hint.empty())
    {
      out += "\n    hint: ";
      out += d.hint;
    }
    return out;
  }

} // namespace asmlex::core
