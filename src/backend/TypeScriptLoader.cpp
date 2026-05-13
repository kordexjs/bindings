/**
 *
 *  @file TypeScriptLoader.cpp
 *  @author Softadastra
 *
 *  Copyright 2026, Softadastra.
 *  All rights reserved.
 *  https://github.com/softadastra/kordex-bindings
 *
 *  Use of this source code is governed by a MIT license
 *  that can be found in the LICENSE file.
 *
 *  Kordex Bindings
 *
 */

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>
#include <string>
#include <utility>

#include <kordex/bindings/TypeScriptLoader.hpp>

namespace kordex::bindings
{
  namespace
  {
    [[nodiscard]] bool is_space(char value) noexcept
    {
      return value == ' ' ||
             value == '\t' ||
             value == '\n' ||
             value == '\r' ||
             value == '\f' ||
             value == '\v';
    }

    [[nodiscard]] bool is_blank(
        const std::string &value) noexcept
    {
      return std::all_of(
          value.begin(),
          value.end(),
          [](char character)
          {
            return is_space(character);
          });
    }

    [[nodiscard]] std::string trim_left(
        std::string value)
    {
      value.erase(
          value.begin(),
          std::find_if(
              value.begin(),
              value.end(),
              [](char character)
              {
                return !is_space(character);
              }));

      return value;
    }

    [[nodiscard]] bool starts_with(
        const std::string &value,
        const std::string &prefix) noexcept
    {
      return value.size() >= prefix.size() &&
             value.substr(0, prefix.size()) == prefix;
    }

    [[nodiscard]] std::size_t count_lines(
        const std::string &source) noexcept
    {
      if (source.empty())
      {
        return 0;
      }

      return static_cast<std::size_t>(
                 std::count(source.begin(), source.end(), '\n')) +
             1;
    }

    [[nodiscard]] TypeScriptDiagnostic make_diagnostic(
        TypeScriptDiagnosticLevel level,
        std::string message,
        std::string path = {},
        std::size_t line = 0,
        std::size_t column = 0)
    {
      TypeScriptDiagnostic diagnostic;
      diagnostic.level = level;
      diagnostic.message = std::move(message);
      diagnostic.path = std::move(path);
      diagnostic.line = line;
      diagnostic.column = column;
      return diagnostic;
    }

    [[nodiscard]] int balance_delta(
        const std::string &line,
        char open_char,
        char close_char) noexcept
    {
      int delta = 0;
      bool in_single_quote = false;
      bool in_double_quote = false;
      bool escaped = false;

      for (char character : line)
      {
        if (escaped)
        {
          escaped = false;
          continue;
        }

        if (character == '\\')
        {
          escaped = true;
          continue;
        }

        if (character == '\'' && !in_double_quote)
        {
          in_single_quote = !in_single_quote;
          continue;
        }

        if (character == '"' && !in_single_quote)
        {
          in_double_quote = !in_double_quote;
          continue;
        }

        if (in_single_quote || in_double_quote)
        {
          continue;
        }

        if (character == open_char)
        {
          ++delta;
        }
        else if (character == close_char)
        {
          --delta;
        }
      }

      return delta;
    }

    [[nodiscard]] std::string strip_type_syntax(
        std::string source)
    {
      /*
       * Remove simple type-only import statements:
       *   import type { X } from "./x";
       */
      source = std::regex_replace(
          source,
          std::regex(R"((^|\n)\s*import\s+type\s+[^;\n]+;?)"),
          "$1");

      /*
       * Remove simple type aliases:
       *   type User = { name: string };
       *   export type User = ...
       */
      source = std::regex_replace(
          source,
          std::regex(R"((^|\n)\s*(export\s+)?type\s+[A-Za-z_$][A-Za-z0-9_$]*\s*=\s*[^;\n]+;?)"),
          "$1");

      /*
       * Remove simple interfaces on one line:
       *   interface User { name: string }
       *   export interface User { ... }
       */
      source = std::regex_replace(
          source,
          std::regex(R"((^|\n)\s*(export\s+)?interface\s+[A-Za-z_$][A-Za-z0-9_$]*(\s+extends\s+[A-Za-z_$][A-Za-z0-9_$]*)?\s*\{[^}]*\}\s*)"),
          "$1");

      /*
       * Remove parameter type annotations:
       *   (name: string)     -> (name)
       *   , age: number      -> , age
       */
      source = std::regex_replace(
          source,
          std::regex(R"(([\(\,]\s*[A-Za-z_$][A-Za-z0-9_$]*)\s*:\s*[^,\)\=\{\}\n]+)"),
          "$1");

      /*
       * Remove variable type annotations before assignment:
       *   const name: string = "x";
       */
      source = std::regex_replace(
          source,
          std::regex(R"(\b(const|let|var)\s+([A-Za-z_$][A-Za-z0-9_$]*)\s*:\s*[^=;\n]+(\s*=))"),
          "$1 $2$3");

      /*
       * Remove function return type annotations:
       *   function f(): number { ... }
       *   const f = (): number => ...
       */
      source = std::regex_replace(
          source,
          std::regex(R"(\)\s*:\s*[^=\{\n]+(\s*\{))"),
          ")$1");

      source = std::regex_replace(
          source,
          std::regex(R"(\)\s*:\s*[^=\{\n]+(\s*=>))"),
          ")$1");

      /*
       * Remove class field type annotations:
       *   name: string;
       *   count: number = 0;
       */
      source = std::regex_replace(
          source,
          std::regex(R"((^|\n)(\s*[A-Za-z_$][A-Za-z0-9_$]*)\s*:\s*[^=;\n]+;)"),
          "$1$2;");

      source = std::regex_replace(
          source,
          std::regex(R"((^|\n)(\s*[A-Za-z_$][A-Za-z0-9_$]*)\s*:\s*[^=;\n]+(\s*=))"),
          "$1$2$3");

      /*
       * Remove "as Type" assertions:
       *   value as string -> value
       */
      source = std::regex_replace(
          source,
          std::regex(R"(\s+as\s+[A-Za-z_$][A-Za-z0-9_$<>\[\]\|&\s,\.]*)"),
          "");

      /*
       * Remove non-null assertions in common cases:
       *   value!.name -> value.name
       */
      source = std::regex_replace(
          source,
          std::regex(R"(([A-Za-z_$][A-Za-z0-9_$]*)!\.)"),
          "$1.");

      return source;
    }

    [[nodiscard]] std::string remove_multiline_interfaces(
        const std::string &source)
    {
      std::istringstream input(source);
      std::ostringstream output;

      std::string line;
      bool skipping_interface = false;
      int brace_balance = 0;

      while (std::getline(input, line))
      {
        const std::string trimmed = trim_left(line);

        if (!skipping_interface &&
            (starts_with(trimmed, "interface ") ||
             starts_with(trimmed, "export interface ")))
        {
          skipping_interface = true;
          brace_balance += balance_delta(line, '{', '}');

          if (brace_balance <= 0 &&
              line.find('{') != std::string::npos)
          {
            skipping_interface = false;
            brace_balance = 0;
          }

          continue;
        }

        if (skipping_interface)
        {
          brace_balance += balance_delta(line, '{', '}');

          if (brace_balance <= 0)
          {
            skipping_interface = false;
            brace_balance = 0;
          }

          continue;
        }

        output << line << '\n';
      }

      return output.str();
    }

    [[nodiscard]] std::vector<TypeScriptDiagnostic> collect_basic_diagnostics(
        const Script &script,
        const TypeScriptLoaderOptions &options)
    {
      std::vector<TypeScriptDiagnostic> diagnostics;

      if (script.source().empty())
      {
        diagnostics.push_back(
            make_diagnostic(
                TypeScriptDiagnosticLevel::Error,
                "TypeScript source cannot be empty",
                script.path()));
        return diagnostics;
      }

      if (is_blank(script.source()))
      {
        diagnostics.push_back(
            make_diagnostic(
                TypeScriptDiagnosticLevel::Error,
                "TypeScript source contains only whitespace",
                script.path(),
                1,
                1));
        return diagnostics;
      }

      int brace_balance = 0;
      int parenthesis_balance = 0;
      int bracket_balance = 0;

      std::istringstream stream(script.source());
      std::string line;
      std::size_t line_number = 1;

      while (std::getline(stream, line))
      {
        brace_balance += balance_delta(line, '{', '}');
        parenthesis_balance += balance_delta(line, '(', ')');
        bracket_balance += balance_delta(line, '[', ']');

        const std::string trimmed = trim_left(line);

        if (options.warn_unsupported &&
            starts_with(trimmed, "enum "))
        {
          diagnostics.push_back(
              make_diagnostic(
                  TypeScriptDiagnosticLevel::Warning,
                  "TypeScript enum is not transformed by the MVP loader",
                  script.path(),
                  line_number,
                  1));
        }

        if (options.warn_unsupported &&
            starts_with(trimmed, "namespace "))
        {
          diagnostics.push_back(
              make_diagnostic(
                  TypeScriptDiagnosticLevel::Warning,
                  "TypeScript namespace is not transformed by the MVP loader",
                  script.path(),
                  line_number,
                  1));
        }

        if (options.warn_unsupported &&
            starts_with(trimmed, "@"))
        {
          diagnostics.push_back(
              make_diagnostic(
                  TypeScriptDiagnosticLevel::Warning,
                  "TypeScript decorators are not transformed by the MVP loader",
                  script.path(),
                  line_number,
                  1));
        }

        ++line_number;
      }

      const std::size_t lines = count_lines(script.source());

      if (brace_balance != 0)
      {
        diagnostics.push_back(
            make_diagnostic(
                TypeScriptDiagnosticLevel::Error,
                "unbalanced TypeScript braces",
                script.path(),
                lines,
                1));
      }

      if (parenthesis_balance != 0)
      {
        diagnostics.push_back(
            make_diagnostic(
                TypeScriptDiagnosticLevel::Error,
                "unbalanced TypeScript parentheses",
                script.path(),
                lines,
                1));
      }

      if (bracket_balance != 0)
      {
        diagnostics.push_back(
            make_diagnostic(
                TypeScriptDiagnosticLevel::Error,
                "unbalanced TypeScript brackets",
                script.path(),
                lines,
                1));
      }

      return diagnostics;
    }
  } // namespace

  bool TypeScriptDiagnostic::has_message() const noexcept
  {
    return !message.empty();
  }

  bool TypeScriptDiagnostic::has_location() const noexcept
  {
    return !path.empty() && line > 0;
  }

  bool TypeScriptDiagnostic::is_error() const noexcept
  {
    return level == TypeScriptDiagnosticLevel::Error;
  }

  bool TypeScriptDiagnostic::is_warning() const noexcept
  {
    return level == TypeScriptDiagnosticLevel::Warning;
  }

  bool TypeScriptLoadResult::has_diagnostics() const noexcept
  {
    return !diagnostics.empty();
  }

  bool TypeScriptLoadResult::has_errors() const noexcept
  {
    return std::any_of(
        diagnostics.begin(),
        diagnostics.end(),
        [](const TypeScriptDiagnostic &diagnostic)
        {
          return diagnostic.is_error();
        });
  }

  bool TypeScriptLoadResult::has_warnings() const noexcept
  {
    return std::any_of(
        diagnostics.begin(),
        diagnostics.end(),
        [](const TypeScriptDiagnostic &diagnostic)
        {
          return diagnostic.is_warning();
        });
  }

  TypeScriptLoader::TypeScriptLoader(
      TypeScriptLoaderOptions options)
      : options_(options)
  {
  }

  const TypeScriptLoaderOptions &TypeScriptLoader::options() const noexcept
  {
    return options_;
  }

  bool TypeScriptLoader::is_typescript(
      const Script &script) noexcept
  {
    return script.type() == ScriptType::TypeScript;
  }

  Result<TypeScriptLoadResult> TypeScriptLoader::load(
      const std::string &path) const
  {
    auto script = Script::load(path);
    if (!script)
    {
      return script.error();
    }

    return transpile(script.value());
  }

  std::vector<TypeScriptDiagnostic> TypeScriptLoader::check(
      const Script &script) const
  {
    if (!is_typescript(script))
    {
      return {};
    }

    return collect_basic_diagnostics(script, options_);
  }

  Result<TypeScriptLoadResult> TypeScriptLoader::transpile(
      Script script) const
  {
    const auto validation = validate_script(script);
    if (validation)
    {
      return validation;
    }

    TypeScriptLoadResult result;
    result.script = script;

    if (!is_typescript(script))
    {
      result.transformed = false;
      return result;
    }

    if (options_.check)
    {
      result.diagnostics = check(script);

      if (result.has_errors())
      {
        return make_binding_error(
            BindingErrorCode::ScriptCompileFailed,
            "TypeScript check failed");
      }
    }

    auto transformed = transpile_source(
        script.source(),
        script.path());

    if (!transformed)
    {
      return transformed.error();
    }

    result.script.set_source(transformed.value());

    if (options_.emit_javascript)
    {
      result.script.set_type(ScriptType::JavaScript);
    }

    if (!result.script.name().empty())
    {
      const auto name = result.script.name();

      if (name.size() > 3 &&
          name.substr(name.size() - 3) == ".ts")
      {
        result.script.set_name(name.substr(0, name.size() - 3) + ".js");
      }
      else if (name.size() > 4 &&
               (name.substr(name.size() - 4) == ".mts" ||
                name.substr(name.size() - 4) == ".cts"))
      {
        result.script.set_name(name.substr(0, name.size() - 4) + ".js");
      }
    }

    result.transformed = true;

    return result;
  }

  Result<std::string> TypeScriptLoader::transpile_source(
      const std::string &source,
      const std::string &path) const
  {
    (void)path;

    if (source.empty())
    {
      return make_binding_error(
          BindingErrorCode::ScriptCompileFailed,
          "TypeScript source cannot be empty");
    }

    if (!options_.strip_types)
    {
      return source;
    }

    std::string transformed = remove_multiline_interfaces(source);
    transformed = strip_type_syntax(std::move(transformed));

    return transformed;
  }

  Error TypeScriptLoader::validate_script(
      const Script &script) const
  {
    const auto validation = script.validate();
    if (validation)
    {
      return validation;
    }

    if (!script.executable())
    {
      return make_binding_error(
          BindingErrorCode::ScriptCompileFailed,
          "TypeScript loader requires an executable script");
    }

    return ok();
  }

  const char *to_string(
      TypeScriptDiagnosticLevel level) noexcept
  {
    switch (level)
    {
    case TypeScriptDiagnosticLevel::Info:
      return "info";
    case TypeScriptDiagnosticLevel::Warning:
      return "warning";
    case TypeScriptDiagnosticLevel::Error:
      return "error";
    }

    return "error";
  }

} // namespace kordex::bindings
