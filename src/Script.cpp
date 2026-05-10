/**
 *
 *  @file Script.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Gaspard Kirira.
 *  All rights reserved.
 *  https://github.com/kordexjs/bindings
 *
 *  Use of this source code is governed by a MIT license
 *  that can be found in the LICENSE file.
 *
 *  Kordex Bindings
 *
 */

#include <utility>
#include <kordex/bindings/Script.hpp>

namespace kordex::bindings
{
  namespace
  {
    [[nodiscard]] bool ends_with(
        std::string_view value,
        std::string_view suffix) noexcept
    {
      return value.size() >= suffix.size() &&
             value.substr(value.size() - suffix.size()) == suffix;
    }

    [[nodiscard]] std::string basename_or_path(
        const std::string &path)
    {
      if (path.empty())
      {
        return {};
      }

      const auto slash = path.find_last_of("/\\");
      if (slash == std::string::npos)
      {
        return path;
      }

      if (slash + 1 >= path.size())
      {
        return path;
      }

      return path.substr(slash + 1);
    }
  } // namespace

  bool ScriptInfo::has_path() const noexcept
  {
    return !path.empty();
  }

  bool ScriptInfo::has_name() const noexcept
  {
    return !name.empty();
  }

  bool ScriptInfo::executable() const noexcept
  {
    return type == ScriptType::JavaScript ||
           type == ScriptType::TypeScript;
  }

  Script::Script(
      std::string source,
      ScriptInfo info)
      : source_(std::move(source)),
        info_(std::move(info))
  {
    if (info_.type == ScriptType::Unknown && info_.has_path())
    {
      info_.type = detect_script_type(info_.path);
    }

    if (!info_.has_name() && info_.has_path())
    {
      info_.name = basename_or_path(info_.path);
    }
  }

  Script Script::from_source(
      std::string source,
      std::string name)
  {
    ScriptInfo info;
    info.name = std::move(name);
    info.type = ScriptType::JavaScript;
    info.loaded_from_file = false;

    return Script(std::move(source), std::move(info));
  }

  Script Script::from_source(
      std::string source,
      ScriptInfo info)
  {
    return Script(std::move(source), std::move(info));
  }

  Result<Script> Script::from_source_file(
      const kordex::runtime::SourceFile &source_file)
  {
    const auto validation = source_file.validate();
    if (validation)
    {
      return make_binding_error(
          BindingErrorCode::InvalidArgument,
          std::string(validation.message()));
    }

    ScriptInfo info;
    info.path = source_file.path;
    info.name = basename_or_path(source_file.path);
    info.type = from_runtime_source_type(source_file.type);
    info.loaded_from_file = true;

    Script script(source_file.content, std::move(info));

    const auto script_validation = script.validate();
    if (script_validation)
    {
      return script_validation;
    }

    return script;
  }

  Result<Script> Script::load(
      const std::string &path)
  {
    auto source_file = kordex::runtime::SourceFile::load(path);
    if (!source_file)
    {
      return make_binding_error(
          BindingErrorCode::ScriptCompileFailed,
          std::string(source_file.error().message()));
    }

    return from_source_file(source_file.value());
  }

  const ScriptInfo &Script::info() const noexcept
  {
    return info_;
  }

  const std::string &Script::source() const noexcept
  {
    return source_;
  }

  const std::string &Script::path() const noexcept
  {
    return info_.path;
  }

  const std::string &Script::name() const noexcept
  {
    return info_.name;
  }

  ScriptType Script::type() const noexcept
  {
    return info_.type;
  }

  bool Script::valid() const noexcept
  {
    return !source_.empty() &&
           info_.type != ScriptType::Unknown;
  }

  bool Script::empty() const noexcept
  {
    return source_.empty();
  }

  std::size_t Script::size() const noexcept
  {
    return source_.size();
  }

  bool Script::loaded_from_file() const noexcept
  {
    return info_.loaded_from_file;
  }

  bool Script::compiled() const noexcept
  {
    return info_.compiled;
  }

  bool Script::executed() const noexcept
  {
    return info_.executed;
  }

  bool Script::executable() const noexcept
  {
    return info_.executable();
  }

  void Script::set_source(std::string source)
  {
    source_ = std::move(source);
    info_.compiled = false;
    info_.executed = false;
  }

  void Script::set_path(std::string path)
  {
    info_.path = std::move(path);

    if (info_.type == ScriptType::Unknown)
    {
      info_.type = detect_script_type(info_.path);
    }

    if (info_.name.empty())
    {
      info_.name = basename_or_path(info_.path);
    }
  }

  void Script::set_name(std::string name)
  {
    info_.name = std::move(name);
  }

  void Script::set_type(ScriptType type) noexcept
  {
    info_.type = type;
    info_.compiled = false;
    info_.executed = false;
  }

  void Script::mark_compiled() noexcept
  {
    info_.compiled = true;
  }

  void Script::mark_uncompiled() noexcept
  {
    info_.compiled = false;
  }

  void Script::mark_executed() noexcept
  {
    info_.executed = true;
  }

  void Script::mark_unexecuted() noexcept
  {
    info_.executed = false;
  }

  Error Script::validate() const
  {
    if (source_.empty())
    {
      return make_binding_error(
          BindingErrorCode::InvalidArgument,
          "script source cannot be empty");
    }

    if (info_.type == ScriptType::Unknown)
    {
      return make_binding_error(
          BindingErrorCode::InvalidArgument,
          "script type is unknown");
    }

    if (!info_.executable() && info_.type != ScriptType::Json)
    {
      return make_binding_error(
          BindingErrorCode::InvalidArgument,
          "script type is not supported");
    }

    return ok();
  }

  ScriptResult Script::compile()
  {
    const auto validation = validate();
    if (validation)
    {
      return ScriptResult::failure(
          make_binding_error(
              BindingErrorCode::ScriptCompileFailed,
              std::string(validation.message())),
          1);
    }

    mark_compiled();

    return ScriptResult::success(
        Value::undefined(),
        "script compiled successfully");
  }

  ScriptResult Script::run()
  {
    const auto validation = validate();
    if (validation)
    {
      return ScriptResult::failure(
          make_binding_error(
              BindingErrorCode::ScriptExecutionFailed,
              std::string(validation.message())),
          1);
    }

    if (!compiled())
    {
      const auto compile_result = compile();
      if (!compile_result.succeeded())
      {
        return compile_result;
      }
    }

    if (!executable())
    {
      return ScriptResult::failure(
          make_binding_error(
              BindingErrorCode::ScriptExecutionFailed,
              "script type is not executable"),
          1);
    }

    mark_executed();

    return ScriptResult::success(
        Value::string(source_),
        "script executed by native placeholder");
  }

  ScriptType detect_script_type(
      std::string_view path) noexcept
  {
    if (ends_with(path, ".js") ||
        ends_with(path, ".mjs") ||
        ends_with(path, ".cjs"))
    {
      return ScriptType::JavaScript;
    }

    if (ends_with(path, ".ts") ||
        ends_with(path, ".mts") ||
        ends_with(path, ".cts"))
    {
      return ScriptType::TypeScript;
    }

    if (ends_with(path, ".json"))
    {
      return ScriptType::Json;
    }

    return ScriptType::Unknown;
  }

  ScriptType from_runtime_source_type(
      kordex::runtime::SourceType type) noexcept
  {
    switch (type)
    {
    case kordex::runtime::SourceType::JavaScript:
      return ScriptType::JavaScript;

    case kordex::runtime::SourceType::TypeScript:
      return ScriptType::TypeScript;

    case kordex::runtime::SourceType::Json:
      return ScriptType::Json;

    case kordex::runtime::SourceType::Unknown:
      return ScriptType::Unknown;
    }

    return ScriptType::Unknown;
  }

  const char *to_string(ScriptType type) noexcept
  {
    switch (type)
    {
    case ScriptType::Unknown:
      return "unknown";
    case ScriptType::JavaScript:
      return "javascript";
    case ScriptType::TypeScript:
      return "typescript";
    case ScriptType::Json:
      return "json";
    }

    return "unknown";
  }

} // namespace kordex::bindings
