/**
 *
 *  @file ModuleLoader.cpp
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
#include <filesystem>
#include <regex>
#include <sstream>
#include <string>
#include <utility>

#include <kordex/bindings/ModuleLoader.hpp>
#include <kordex/bindings/TypeScriptLoader.hpp>
#include <kordex/bindings/EngineContext.hpp>

namespace kordex::bindings
{
  namespace
  {
    [[nodiscard]] bool is_relative_specifier(
        const std::string &specifier) noexcept
    {
      return specifier == "." ||
             specifier == ".." ||
             specifier.rfind("./", 0) == 0 ||
             specifier.rfind("../", 0) == 0;
    }

    [[nodiscard]] bool is_absolute_specifier(
        const std::string &specifier)
    {
      return !specifier.empty() &&
             std::filesystem::path(specifier).is_absolute();
    }

    [[nodiscard]] std::string normalize_path(
        const std::string &path)
    {
      try
      {
        return std::filesystem::weakly_canonical(path).string();
      }
      catch (...)
      {
        return std::filesystem::path(path).lexically_normal().string();
      }
    }

    [[nodiscard]] bool is_json_script(
        const Script &script) noexcept
    {
      return script.type() == ScriptType::Json;
    }

    [[nodiscard]] bool is_typescript_script(
        const Script &script) noexcept
    {
      return script.type() == ScriptType::TypeScript;
    }

    [[nodiscard]] std::vector<std::string> collect_import_specifiers(
        const std::string &source)
    {
      std::vector<std::string> specifiers;

      const std::regex import_from_regex(
          R"(\bimport\s+[^;]*?\s+from\s+["']([^"']+)["']\s*;?)");

      const std::regex import_side_effect_regex(
          R"(\bimport\s+["']([^"']+)["']\s*;?)");

      for (std::sregex_iterator it(
               source.begin(),
               source.end(),
               import_from_regex),
           end;
           it != end;
           ++it)
      {
        specifiers.push_back((*it)[1].str());
      }

      for (std::sregex_iterator it(
               source.begin(),
               source.end(),
               import_side_effect_regex),
           end;
           it != end;
           ++it)
      {
        specifiers.push_back((*it)[1].str());
      }

      std::sort(specifiers.begin(), specifiers.end());
      specifiers.erase(
          std::unique(specifiers.begin(), specifiers.end()),
          specifiers.end());

      return specifiers;
    }

    [[nodiscard]] std::string trim(
        std::string value)
    {
      auto not_space = [](unsigned char character)
      {
        return !std::isspace(character);
      };

      value.erase(
          value.begin(),
          std::find_if(value.begin(), value.end(), not_space));

      value.erase(
          std::find_if(value.rbegin(), value.rend(), not_space).base(),
          value.end());

      return value;
    }

    [[nodiscard]] std::vector<std::string> split_comma(
        const std::string &value)
    {
      std::vector<std::string> parts;
      std::stringstream stream(value);
      std::string part;

      while (std::getline(stream, part, ','))
      {
        part = trim(part);
        if (!part.empty())
        {
          parts.push_back(part);
        }
      }

      return parts;
    }

    [[nodiscard]] std::string transform_named_bindings(
        const std::string &bindings)
    {
      std::ostringstream stream;
      const auto parts = split_comma(bindings);

      for (std::size_t index = 0; index < parts.size(); ++index)
      {
        std::string part = parts[index];

        const auto as_position = part.find(" as ");
        if (as_position != std::string::npos)
        {
          const std::string from = trim(part.substr(0, as_position));
          const std::string to = trim(part.substr(as_position + 4));

          stream << from << ": " << to;
        }
        else
        {
          stream << part;
        }

        if (index + 1 < parts.size())
        {
          stream << ", ";
        }
      }

      return stream.str();
    }

    [[nodiscard]] std::string append_export_assignment(
        std::string source,
        const std::string &name,
        const std::string &export_name)
    {
      source += "\nexports.";
      source += export_name;
      source += " = ";
      source += name;
      source += ";\n";

      return source;
    }

    [[nodiscard]] std::string transform_export_syntax(
        std::string source)
    {
      std::vector<std::pair<std::string, std::string>> exports;

      {
        const std::regex export_decl_regex(
            R"(\bexport\s+(const|let|var|function|class)\s+([A-Za-z_$][A-Za-z0-9_$]*))");

        std::string transformed;
        std::size_t last = 0;

        for (std::sregex_iterator it(
                 source.begin(),
                 source.end(),
                 export_decl_regex),
             end;
             it != end;
             ++it)
        {
          const auto match = *it;

          transformed.append(
              source,
              last,
              static_cast<std::size_t>(match.position()) - last);

          transformed += match[1].str();
          transformed += " ";
          transformed += match[2].str();

          exports.push_back({match[2].str(), match[2].str()});

          last = static_cast<std::size_t>(match.position()) +
                 static_cast<std::size_t>(match.length());
        }

        transformed.append(source, last, std::string::npos);
        source = std::move(transformed);
      }

      {
        const std::regex export_default_regex(
            R"(\bexport\s+default\s+([^;]+);?)");

        std::string transformed;
        std::size_t last = 0;
        bool found_default = false;

        for (std::sregex_iterator it(
                 source.begin(),
                 source.end(),
                 export_default_regex),
             end;
             it != end;
             ++it)
        {
          const auto match = *it;

          transformed.append(
              source,
              last,
              static_cast<std::size_t>(match.position()) - last);

          transformed += "const __kordex_default_export = ";
          transformed += match[1].str();
          transformed += ";\n";

          found_default = true;

          last = static_cast<std::size_t>(match.position()) +
                 static_cast<std::size_t>(match.length());
        }

        transformed.append(source, last, std::string::npos);
        source = std::move(transformed);

        if (found_default)
        {
          exports.push_back({"__kordex_default_export", "default"});
        }
      }

      {
        const std::regex export_list_regex(
            R"(\bexport\s*\{([^}]+)\}\s*;?)");

        std::string transformed;
        std::size_t last = 0;

        for (std::sregex_iterator it(
                 source.begin(),
                 source.end(),
                 export_list_regex),
             end;
             it != end;
             ++it)
        {
          const auto match = *it;

          transformed.append(
              source,
              last,
              static_cast<std::size_t>(match.position()) - last);

          for (const auto &part : split_comma(match[1].str()))
          {
            const auto as_position = part.find(" as ");

            if (as_position != std::string::npos)
            {
              exports.push_back(
                  {trim(part.substr(0, as_position)),
                   trim(part.substr(as_position + 4))});
            }
            else
            {
              exports.push_back({part, part});
            }
          }

          last = static_cast<std::size_t>(match.position()) +
                 static_cast<std::size_t>(match.length());
        }

        transformed.append(source, last, std::string::npos);
        source = std::move(transformed);
      }

      for (const auto &[name, export_name] : exports)
      {
        source = append_export_assignment(source, name, export_name);
      }

      return source;
    }

    [[nodiscard]] bool is_builtin_specifier(
        const std::string &specifier) noexcept
    {
      return specifier.rfind("kordex:", 0) == 0;
    }
  } // namespace

  bool ModuleLoaderReport::has_loaded_modules() const noexcept
  {
    return !loaded_modules.empty();
  }

  ModuleLoader::ModuleLoader(
      ModuleLoaderOptions options)
      : options_(options),
        context_(nullptr),
        cache_()
  {
  }

  ModuleLoader::ModuleLoader(
      const EngineContext *context,
      ModuleLoaderOptions options)
      : options_(options),
        context_(context),
        cache_()
  {
  }

  const ModuleLoaderOptions &ModuleLoader::options() const noexcept
  {
    return options_;
  }

  bool ModuleLoader::has_module_syntax(
      const std::string &source)
  {
    return std::regex_search(
               source,
               std::regex(R"(\bimport\s+)")) ||
           std::regex_search(
               source,
               std::regex(R"(\bexport\s+)"));
  }

  Result<ModuleLoaderResult> ModuleLoader::load_entry(
      Script entry)
  {
    auto normalized = normalize_entry(std::move(entry));
    if (!normalized)
    {
      return normalized.error();
    }

    ModuleLoaderResult result;
    result.report.entry_path = normalized.value().path();

    const bool needs_loading =
        is_json_script(normalized.value()) ||
        is_typescript_script(normalized.value()) ||
        has_module_syntax(normalized.value().source());

    if (!needs_loading)
    {
      result.script = normalized.value();
      result.report.bundled = false;
      return result;
    }

    auto loaded_entry = load_module_graph(
        normalized.value(),
        normalized.value().path().empty()
            ? normalized.value().name()
            : normalized.value().path(),
        result.report);

    if (!loaded_entry)
    {
      return loaded_entry.error();
    }

    auto bundle = build_bundle(
        loaded_entry.value(),
        cache_);

    if (!bundle)
    {
      return bundle.error();
    }

    Script bundled = Script::from_source(
        bundle.value(),
        normalized.value().name().empty()
            ? "bundle.js"
            : normalized.value().name());

    result.script = bundled;
    result.report.bundled = true;

    return result;
  }

  void ModuleLoader::clear_cache()
  {
    cache_.clear();
  }

  std::size_t ModuleLoader::cache_size() const noexcept
  {
    return cache_.size();
  }

  Result<Script> ModuleLoader::normalize_entry(
      Script entry) const
  {
    const auto validation = entry.validate();
    if (validation)
    {
      return validation;
    }

    if (is_json_script(entry))
    {
      if (!options_.allow_json)
      {
        return make_binding_error(
            BindingErrorCode::PermissionDenied,
            "JSON modules are disabled");
      }

      return entry;
    }

    if (is_typescript_script(entry) && options_.allow_typescript)
    {
      TypeScriptLoader loader;
      auto loaded = loader.transpile(std::move(entry));
      if (!loaded)
      {
        return loaded.error();
      }

      return loaded.value().script;
    }

    return entry;
  }

  Result<ModuleLoader::LoadedModule> ModuleLoader::load_module_graph(
      const Script &script,
      const std::string &requested_id,
      ModuleLoaderReport &report)
  {
    LoadedModule module;
    module.path = script.path().empty()
                      ? requested_id
                      : normalize_path(script.path());

    module.id = make_module_id(module.path);
    module.script = script;
    module.json = is_json_script(script);

    if (options_.cache_enabled)
    {
      const auto cached = cache_.find(module.id);
      if (cached != cache_.end())
      {
        report.used_cache = true;
        return cached->second;
      }
    }

    if (std::find(
            report.loaded_modules.begin(),
            report.loaded_modules.end(),
            module.path) == report.loaded_modules.end())
    {
      report.loaded_modules.push_back(module.path);
    }

    if (!module.json)
    {
      module.dependencies = collect_import_specifiers(script.source());

      for (const auto &specifier : module.dependencies)
      {
        auto dependency = load_dependency(
            specifier,
            module,
            report);

        if (!dependency)
        {
          return dependency.error();
        }
      }
    }

    auto transformed = transform_module_source(module, cache_);
    if (!transformed)
    {
      return transformed.error();
    }

    module.transformed_source = transformed.value();

    if (options_.cache_enabled)
    {
      cache_[module.id] = module;
    }

    return module;
  }

  Result<ModuleLoader::LoadedModule> ModuleLoader::load_builtin_module(
      const std::string &specifier,
      ModuleLoaderReport &report)
  {
    if (!context_)
    {
      return make_binding_error(
          BindingErrorCode::ContextUnavailable,
          "builtin imports require an engine context");
    }

    const std::string canonical = canonical_builtin_name(specifier);

    auto imported = context_->import_module(canonical);
    if (!imported)
    {
      imported = context_->import_module(specifier);
    }

    if (!imported)
    {
      return make_binding_error(
          BindingErrorCode::ModuleNotFound,
          std::string(imported.error().message()));
    }

    LoadedModule module;
    module.id = specifier;
    module.path = specifier;
    module.native_module = imported.value();
    module.builtin = true;
    module.json = false;

    if (options_.cache_enabled)
    {
      const auto cached = cache_.find(module.id);
      if (cached != cache_.end())
      {
        report.used_cache = true;
        return cached->second;
      }
    }

    auto transformed = transform_builtin_module_source(module);
    if (!transformed)
    {
      return transformed.error();
    }

    module.transformed_source = transformed.value();

    if (std::find(
            report.loaded_modules.begin(),
            report.loaded_modules.end(),
            module.path) == report.loaded_modules.end())
    {
      report.loaded_modules.push_back(module.path);
    }

    if (options_.cache_enabled)
    {
      cache_[module.id] = module;
    }

    return module;
  }

  Result<std::string> ModuleLoader::transform_builtin_module_source(
      const LoadedModule &module) const
  {
    if (!module.builtin)
    {
      return make_binding_error(
          BindingErrorCode::InvalidArgument,
          "module is not a builtin module");
    }

    std::ostringstream source;

    for (const auto &[name, value] : module.native_module.exports())
    {
      source << "exports.";
      source << name;
      source << " = ";

      if (value.is_undefined())
      {
        source << "undefined";
      }
      else if (value.is_null())
      {
        source << "null";
      }
      else if (value.is_boolean())
      {
        auto boolean = value.as_boolean();
        source << (boolean && boolean.value() ? "true" : "false");
      }
      else if (value.is_number())
      {
        auto number = value.as_number();
        source << (number ? std::to_string(number.value()) : "0");
      }
      else if (value.is_string())
      {
        auto text = value.as_string();
        source << js_string(text ? text.value() : std::string{});
      }
      else
      {
        source << "undefined";
      }

      source << ";\n";
    }

    for (const auto &name : module.native_module.function_names())
    {
      source << "exports.";
      source << name;
      source << " = function(...args) { ";
      source << "return __kordex_call_native(";
      source << js_string(canonical_builtin_name(module.id));
      source << ", ";
      source << js_string(name);
      source << ", ...args);";
      source << " };\n";
    }

    return source.str();
  }

  std::string ModuleLoader::canonical_builtin_name(
      const std::string &specifier)
  {
    constexpr std::string_view prefix = "kordex:";

    if (specifier.size() > prefix.size() &&
        specifier.rfind(prefix, 0) == 0)
    {
      return specifier.substr(prefix.size());
    }

    return specifier;
  }

  Result<ModuleLoader::LoadedModule> ModuleLoader::load_dependency(
      const std::string &specifier,
      const LoadedModule &parent,
      ModuleLoaderReport &report)
  {
    if (is_relative_specifier(specifier) && !options_.allow_relative)
    {
      return make_binding_error(
          BindingErrorCode::PermissionDenied,
          "relative module imports are disabled: " + specifier);
    }

    if (is_absolute_specifier(specifier) && !options_.allow_absolute)
    {
      return make_binding_error(
          BindingErrorCode::PermissionDenied,
          "absolute module imports are disabled: " + specifier);
    }

    if (is_builtin_specifier(specifier))
    {
      if (!options_.allow_builtins)
      {
        return make_binding_error(
            BindingErrorCode::PermissionDenied,
            "builtin module imports are disabled: " + specifier);
      }

      return load_builtin_module(
          specifier,
          report);
    }

    if (!is_relative_specifier(specifier) &&
        !is_absolute_specifier(specifier))
    {
      return make_binding_error(
          BindingErrorCode::ModuleNotFound,
          "package imports are not connected yet: " + specifier);
    }

    auto path = resolve_dependency_path(
        specifier,
        parent);

    if (!path)
    {
      return path.error();
    }

    auto dependency_script = load_script_from_path(path.value());
    if (!dependency_script)
    {
      return dependency_script.error();
    }

    auto normalized = normalize_entry(dependency_script.value());
    if (!normalized)
    {
      return normalized.error();
    }

    return load_module_graph(
        normalized.value(),
        path.value(),
        report);
  }

  Result<Script> ModuleLoader::load_script_from_path(
      const std::string &path) const
  {
    auto script = Script::load(path);
    if (!script)
    {
      return make_binding_error(
          BindingErrorCode::ModuleNotFound,
          std::string(script.error().message()));
    }

    return script.value();
  }

  Result<std::string> ModuleLoader::resolve_dependency_path(
      const std::string &specifier,
      const LoadedModule &parent) const
  {
    kordex::runtime::ModuleResolverOptions options;
    options.base_dir = parent_directory(parent.path);
    options.allow_builtins = options_.allow_builtins;
    options.allow_packages = options_.allow_packages;
    options.resolve_extensions = true;
    options.resolve_index = true;

    kordex::runtime::ModuleResolver resolver(options);

    auto resolved = resolver.resolve(specifier);
    if (!resolved)
    {
      return make_binding_error(
          BindingErrorCode::ModuleNotFound,
          std::string(resolved.error().message()));
    }

    if (!resolved.value().is_file())
    {
      return make_binding_error(
          BindingErrorCode::ModuleNotFound,
          "only file modules are supported in this loader step: " +
              specifier);
    }

    return normalize_path(resolved.value().path);
  }

  Result<std::string> ModuleLoader::transform_module_source(
      const LoadedModule &module,
      const std::map<std::string, LoadedModule> &modules) const
  {
    if (module.json)
    {
      std::string source = "exports.default = JSON.parse(";
      source += js_string(module.script.source());
      source += ");\n";
      return source;
    }

    std::string source = module.script.source();

    const std::regex import_named_regex(
        R"(\bimport\s*\{([^}]+)\}\s*from\s*["']([^"']+)["']\s*;?)");

    const std::regex import_default_regex(
        R"(\bimport\s+([A-Za-z_$][A-Za-z0-9_$]*)\s+from\s*["']([^"']+)["']\s*;?)");

    const std::regex import_namespace_regex(
        R"(\bimport\s*\*\s*as\s+([A-Za-z_$][A-Za-z0-9_$]*)\s+from\s*["']([^"']+)["']\s*;?)");

    const std::regex import_side_effect_regex(
        R"(\bimport\s+["']([^"']+)["']\s*;?)");

    auto replacement_for = [&](const std::string &specifier) -> Result<std::string>
    {
      if (is_builtin_specifier(specifier))
      {
        if (cache_.find(specifier) == cache_.end())
        {
          return make_binding_error(
              BindingErrorCode::ModuleNotFound,
              "builtin module was resolved but not loaded: " + specifier);
        }

        return specifier;
      }

      auto dependency_path = resolve_dependency_path(
          specifier,
          module);

      if (!dependency_path)
      {
        return dependency_path.error();
      }

      const std::string dependency_id = make_module_id(dependency_path.value());

      if (modules.find(dependency_id) == modules.end() &&
          cache_.find(dependency_id) == cache_.end())
      {
        return make_binding_error(
            BindingErrorCode::ModuleNotFound,
            "module was resolved but not loaded: " + specifier);
      }

      return dependency_id;
    };

    {
      std::string transformed;
      std::size_t last = 0;

      for (std::sregex_iterator it(
               source.begin(),
               source.end(),
               import_named_regex),
           end;
           it != end;
           ++it)
      {
        const auto match = *it;
        transformed.append(
            source,
            last,
            static_cast<std::size_t>(match.position()) - last);

        auto module_id = replacement_for(match[2].str());
        if (!module_id)
        {
          return module_id.error();
        }

        transformed += "const { ";
        transformed += transform_named_bindings(match[1].str());
        transformed += " } = __kordex_require(";
        transformed += js_string(module_id.value());
        transformed += ");";

        last = static_cast<std::size_t>(match.position()) +
               static_cast<std::size_t>(match.length());
      }

      transformed.append(source, last, std::string::npos);
      source = std::move(transformed);
    }

    {
      std::string transformed;
      std::size_t last = 0;

      for (std::sregex_iterator it(
               source.begin(),
               source.end(),
               import_default_regex),
           end;
           it != end;
           ++it)
      {
        const auto match = *it;
        transformed.append(
            source,
            last,
            static_cast<std::size_t>(match.position()) - last);

        auto module_id = replacement_for(match[2].str());
        if (!module_id)
        {
          return module_id.error();
        }

        transformed += "const ";
        transformed += match[1].str();
        transformed += " = __kordex_require(";
        transformed += js_string(module_id.value());
        transformed += ").default;";

        last = static_cast<std::size_t>(match.position()) +
               static_cast<std::size_t>(match.length());
      }

      transformed.append(source, last, std::string::npos);
      source = std::move(transformed);
    }

    {
      std::string transformed;
      std::size_t last = 0;

      for (std::sregex_iterator it(
               source.begin(),
               source.end(),
               import_namespace_regex),
           end;
           it != end;
           ++it)
      {
        const auto match = *it;
        transformed.append(
            source,
            last,
            static_cast<std::size_t>(match.position()) - last);

        auto module_id = replacement_for(match[2].str());
        if (!module_id)
        {
          return module_id.error();
        }

        transformed += "const ";
        transformed += match[1].str();
        transformed += " = __kordex_require(";
        transformed += js_string(module_id.value());
        transformed += ");";

        last = static_cast<std::size_t>(match.position()) +
               static_cast<std::size_t>(match.length());
      }

      transformed.append(source, last, std::string::npos);
      source = std::move(transformed);
    }

    {
      std::string transformed;
      std::size_t last = 0;

      for (std::sregex_iterator it(
               source.begin(),
               source.end(),
               import_side_effect_regex),
           end;
           it != end;
           ++it)
      {
        const auto match = *it;
        transformed.append(
            source,
            last,
            static_cast<std::size_t>(match.position()) - last);

        auto module_id = replacement_for(match[1].str());
        if (!module_id)
        {
          return module_id.error();
        }

        transformed += "__kordex_require(";
        transformed += js_string(module_id.value());
        transformed += ");";

        last = static_cast<std::size_t>(match.position()) +
               static_cast<std::size_t>(match.length());
      }

      transformed.append(source, last, std::string::npos);
      source = std::move(transformed);
    }

    source = transform_export_syntax(std::move(source));

    return source;
  }

  Result<std::string> ModuleLoader::build_bundle(
      const LoadedModule &entry,
      const std::map<std::string, LoadedModule> &modules) const
  {
    std::ostringstream output;

    output << "(function(){\n";
    output << "const __kordex_modules = Object.create(null);\n";
    output << "const __kordex_cache = Object.create(null);\n";
    output << "function __kordex_require(id){\n";
    output << "  if (__kordex_cache[id]) return __kordex_cache[id].exports;\n";
    output << "  const factory = __kordex_modules[id];\n";
    output << "  if (!factory) throw new Error('Module not found: ' + id);\n";
    output << "  const module = { exports: {} };\n";
    output << "  __kordex_cache[id] = module;\n";
    output << "  factory(module.exports, __kordex_require, module);\n";
    output << "  return module.exports;\n";
    output << "}\n";

    for (const auto &[id, module] : modules)
    {
      output << "__kordex_modules[";
      output << js_string(id);
      output << "] = function(exports, __kordex_require, module){\n";
      output << module.transformed_source << "\n";
      output << "};\n";
    }

    output << "const __kordex_entry = __kordex_require(";
    output << js_string(entry.id);
    output << ");\n";
    output << "if (__kordex_entry && Object.prototype.hasOwnProperty.call(__kordex_entry, 'default')) ";
    output << "return __kordex_entry.default;\n";
    output << "return __kordex_entry;\n";
    output << "})()";

    return output.str();
  }

  std::string ModuleLoader::make_module_id(
      const std::string &path) const
  {
    return normalize_path(path);
  }

  std::string ModuleLoader::parent_directory(
      const std::string &path)
  {
    if (path.empty())
    {
      return ".";
    }

    const auto parent = std::filesystem::path(path).parent_path();

    if (parent.empty())
    {
      return ".";
    }

    return parent.string();
  }

  std::string ModuleLoader::json_escape(
      const std::string &value)
  {
    std::string result;
    result.reserve(value.size() + 8);

    for (char character : value)
    {
      switch (character)
      {
      case '\\':
        result += "\\\\";
        break;
      case '"':
        result += "\\\"";
        break;
      case '\n':
        result += "\\n";
        break;
      case '\r':
        result += "\\r";
        break;
      case '\t':
        result += "\\t";
        break;
      default:
        result += character;
        break;
      }
    }

    return result;
  }

  std::string ModuleLoader::js_string(
      const std::string &value)
  {
    return "\"" + json_escape(value) + "\"";
  }

} // namespace kordex::bindings
