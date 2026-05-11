/**
 *
 *  @file BindingConfig.cpp
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
#include <string>

#include <kordex/bindings/BindingConfig.hpp>

namespace kordex::bindings
{
  namespace
  {
    [[nodiscard]] std::string to_lower(std::string value)
    {
      std::transform(
          value.begin(),
          value.end(),
          value.begin(),
          [](unsigned char c)
          {
            return static_cast<char>(std::tolower(c));
          });

      return value;
    }
  } // namespace

  Result<EngineBackend> parse_engine_backend(
      const std::string &value)
  {
    const std::string normalized = to_lower(value);

    if (normalized == "native")
    {
      return EngineBackend::Native;
    }

    if (normalized == "quickjs" || normalized == "quick-js")
    {
      return EngineBackend::QuickJS;
    }

    if (normalized == "v8")
    {
      return EngineBackend::V8;
    }

    return make_binding_error(
        BindingErrorCode::InvalidConfig,
        "invalid bindings engine backend: " + value);
  }

  Result<ModulePolicy> parse_module_policy(
      const std::string &value)
  {
    const std::string normalized = to_lower(value);

    if (normalized == "disabled" || normalized == "none")
    {
      return ModulePolicy::Disabled;
    }

    if (normalized == "builtin_only" ||
        normalized == "builtin-only" ||
        normalized == "builtins")
    {
      return ModulePolicy::BuiltinOnly;
    }

    if (normalized == "full" || normalized == "enabled")
    {
      return ModulePolicy::Full;
    }

    return make_binding_error(
        BindingErrorCode::InvalidConfig,
        "invalid bindings module policy: " + value);
  }

  Result<BindingConfig> BindingConfig::from_options(
      const BindingOptions &options)
  {
    BindingConfig config;

    config.backend = options.backend;
    config.module_policy = options.module_policy;

    config.allow_native_modules = options.allow_native_modules;
    config.allow_native_functions = options.allow_native_functions;
    config.allow_runtime_bridge = options.allow_runtime_bridge;

    config.diagnostics = options.diagnostics;
    config.debug = options.debug;
    config.source_maps = options.source_maps;

    config.max_stack_size = options.max_stack_size;
    config.max_heap_size = options.max_heap_size;

    config.module_paths = options.module_paths;

    config.engine_name = options.engine_name.empty()
                             ? std::string(to_string(options.backend))
                             : options.engine_name;

    config.environment = options.debug ? "development" : "production";

    const auto validation = config.validate();
    if (validation)
    {
      return validation;
    }

    return config;
  }

  Result<BindingConfig> BindingConfig::from_environment(
      const BindingOptions &base)
  {
    /*
     * Environment support will be expanded when the env module is added as a
     * direct bindings dependency.
     *
     * For now, this keeps the API stable and returns the normalized options.
     */
    return from_options(base);
  }

  bool BindingConfig::has_stack_limit() const noexcept
  {
    return max_stack_size > 0;
  }

  bool BindingConfig::has_heap_limit() const noexcept
  {
    return max_heap_size > 0;
  }

  bool BindingConfig::has_module_paths() const noexcept
  {
    return !module_paths.empty();
  }

  bool BindingConfig::has_engine_name() const noexcept
  {
    return !engine_name.empty();
  }

  Error BindingConfig::validate() const
  {
    if (engine_name.empty())
    {
      return make_binding_error(
          BindingErrorCode::InvalidConfig,
          "bindings engine name cannot be empty");
    }

    if (environment.empty())
    {
      return make_binding_error(
          BindingErrorCode::InvalidConfig,
          "bindings environment cannot be empty");
    }

    if (module_policy == ModulePolicy::Disabled &&
        allow_native_modules)
    {
      return make_binding_error(
          BindingErrorCode::InvalidConfig,
          "native modules cannot be enabled when module policy is disabled");
    }

    if (!allow_native_modules &&
        module_policy == ModulePolicy::BuiltinOnly)
    {
      return make_binding_error(
          BindingErrorCode::InvalidConfig,
          "builtin-only module policy requires native modules to be enabled");
    }

    if (!allow_native_functions &&
        allow_native_modules)
    {
      return make_binding_error(
          BindingErrorCode::InvalidConfig,
          "native modules require native functions to be enabled");
    }

    return ok();
  }

} // namespace kordex::bindings
