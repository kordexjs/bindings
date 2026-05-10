/**
 *
 *  @file EngineContext.cpp
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
#include <kordex/bindings/EngineContext.hpp>

namespace kordex::bindings
{
  bool EngineContextInfo::has_name() const noexcept
  {
    return !name.empty();
  }

  bool EngineContextInfo::has_id() const noexcept
  {
    return !id.empty();
  }

  EngineContext::EngineContext()
      : EngineContext(BindingConfig{})
  {
  }

  EngineContext::EngineContext(BindingConfig config)
      : config_(std::move(config)),
        info_(),
        registry_(registry_options_from_config(config_)),
        globals_("global"),
        global_functions_()
  {
  }

  EngineContext::EngineContext(
      BindingConfig config,
      std::string name)
      : config_(std::move(config)),
        info_(),
        registry_(registry_options_from_config(config_)),
        globals_("global"),
        global_functions_()
  {
    info_.name = std::move(name);
  }

  Result<EngineContext> EngineContext::create()
  {
    return create(BindingOptions::defaults());
  }

  Result<EngineContext> EngineContext::create(
      const BindingOptions &options)
  {
    auto config = BindingConfig::from_options(options);
    if (!config)
    {
      return config.error();
    }

    return create(config.value());
  }

  Result<EngineContext> EngineContext::create(
      BindingConfig config)
  {
    const auto validation = config.validate();
    if (validation)
    {
      return validation;
    }

    return EngineContext(std::move(config));
  }

  const EngineContextInfo &EngineContext::info() const noexcept
  {
    return info_;
  }

  const std::string &EngineContext::name() const noexcept
  {
    return info_.name;
  }

  const std::string &EngineContext::id() const noexcept
  {
    return info_.id;
  }

  void EngineContext::set_id(std::string id)
  {
    info_.id = std::move(id);
  }

  void EngineContext::set_name(std::string name)
  {
    info_.name = std::move(name);
  }

  const BindingConfig &EngineContext::config() const noexcept
  {
    return config_;
  }

  BindingConfig &EngineContext::config() noexcept
  {
    return config_;
  }

  const ModuleRegistry &EngineContext::registry() const noexcept
  {
    return registry_;
  }

  ModuleRegistry &EngineContext::registry() noexcept
  {
    return registry_;
  }

  const Object &EngineContext::globals() const noexcept
  {
    return globals_;
  }

  Object &EngineContext::globals() noexcept
  {
    return globals_;
  }

  Error EngineContext::initialize()
  {
    if (info_.initialized)
    {
      return ok();
    }

    const auto validation = validate();
    if (validation)
    {
      return validation;
    }

    registry_.set_options(registry_options_from_config(config_));

    info_.initialized = true;

    return ok();
  }

  Error EngineContext::shutdown()
  {
    if (!info_.initialized)
    {
      return ok();
    }

    info_.initialized = false;

    return ok();
  }

  bool EngineContext::initialized() const noexcept
  {
    return info_.initialized;
  }

  Error EngineContext::validate() const
  {
    if (!info_.has_name())
    {
      return make_binding_error(
          BindingErrorCode::InvalidConfig,
          "engine context name cannot be empty");
    }

    const auto config_validation = config_.validate();
    if (config_validation)
    {
      return config_validation;
    }

    return ok();
  }

  Error EngineContext::set_global(
      std::string name,
      Value value)
  {
    const auto state = require_initialized();
    if (state)
    {
      return state;
    }

    const auto validation = validate_name(name, "global");
    if (validation)
    {
      return validation;
    }

    return globals_.set(std::move(name), std::move(value));
  }

  Result<Value> EngineContext::global(
      const std::string &name) const
  {
    const auto state = require_initialized();
    if (state)
    {
      return state;
    }

    const auto validation = validate_name(name, "global");
    if (validation)
    {
      return validation;
    }

    return globals_.get(name);
  }

  bool EngineContext::has_global(
      const std::string &name) const noexcept
  {
    return globals_.has(name);
  }

  Error EngineContext::remove_global(
      const std::string &name)
  {
    const auto state = require_initialized();
    if (state)
    {
      return state;
    }

    const auto validation = validate_name(name, "global");
    if (validation)
    {
      return validation;
    }

    return globals_.remove(name);
  }

  Error EngineContext::set_global_function(
      std::string name,
      Function function)
  {
    const auto state = require_initialized();
    if (state)
    {
      return state;
    }

    const auto validation = validate_name(name, "global function");
    if (validation)
    {
      return validation;
    }

    if (!config_.allow_native_functions)
    {
      return make_binding_error(
          BindingErrorCode::PermissionDenied,
          "native functions are disabled by bindings config");
    }

    if (!function.valid())
    {
      return make_binding_error(
          BindingErrorCode::FunctionCallFailed,
          "global function is invalid");
    }

    global_functions_[std::move(name)] = std::move(function);

    return ok();
  }

  Error EngineContext::add_global_function(Function function)
  {
    if (!function.valid())
    {
      return make_binding_error(
          BindingErrorCode::FunctionCallFailed,
          "global function is invalid");
    }

    if (function.name().empty())
    {
      return make_binding_error(
          BindingErrorCode::InvalidArgument,
          "global function name cannot be empty");
    }

    return set_global_function(function.name(), std::move(function));
  }

  Result<Function> EngineContext::global_function(
      const std::string &name) const
  {
    const auto state = require_initialized();
    if (state)
    {
      return state;
    }

    const auto validation = validate_name(name, "global function");
    if (validation)
    {
      return validation;
    }

    const auto found = global_functions_.find(name);
    if (found == global_functions_.end())
    {
      return make_binding_error(
          BindingErrorCode::FunctionCallFailed,
          "global function was not found: " + name);
    }

    return found->second;
  }

  bool EngineContext::has_global_function(
      const std::string &name) const noexcept
  {
    return global_functions_.find(name) != global_functions_.end();
  }

  Error EngineContext::remove_global_function(
      const std::string &name)
  {
    const auto state = require_initialized();
    if (state)
    {
      return state;
    }

    const auto validation = validate_name(name, "global function");
    if (validation)
    {
      return validation;
    }

    const auto erased = global_functions_.erase(name);
    if (erased == 0)
    {
      return make_binding_error(
          BindingErrorCode::FunctionCallFailed,
          "global function was not found: " + name);
    }

    return ok();
  }

  Result<Value> EngineContext::call_global_function(
      const std::string &name,
      const FunctionArguments &args) const
  {
    auto function = global_function(name);
    if (!function)
    {
      return function.error();
    }

    return function.value().call(args);
  }

  Error EngineContext::register_module(Module module)
  {
    const auto state = require_initialized();
    if (state)
    {
      return state;
    }

    return registry_.register_module(std::move(module));
  }

  Error EngineContext::register_native_module(
      const NativeModule &native_module)
  {
    const auto state = require_initialized();
    if (state)
    {
      return state;
    }

    if (!config_.allow_native_modules)
    {
      return make_binding_error(
          BindingErrorCode::PermissionDenied,
          "native modules are disabled by bindings config");
    }

    return registry_.register_native_module(native_module);
  }

  Result<Module> EngineContext::import_module(
      const std::string &name) const
  {
    const auto state = require_initialized();
    if (state)
    {
      return state;
    }

    return registry_.import_module(name);
  }

  ScriptResult EngineContext::run_script(Script script)
  {
    const auto state = require_initialized();
    if (state)
    {
      return ScriptResult::failure(
          make_binding_error(
              BindingErrorCode::ContextUnavailable,
              std::string(state.message())),
          1);
    }

    return script.run();
  }

  ScriptResult EngineContext::eval(
      std::string source,
      std::string name)
  {
    Script script = Script::from_source(
        std::move(source),
        std::move(name));

    return run_script(std::move(script));
  }

  std::vector<std::string> EngineContext::global_names() const
  {
    return globals_.keys();
  }

  std::vector<std::string> EngineContext::global_function_names() const
  {
    std::vector<std::string> result;
    result.reserve(global_functions_.size());

    for (const auto &[name, function] : global_functions_)
    {
      (void)function;
      result.push_back(name);
    }

    return result;
  }

  std::size_t EngineContext::global_count() const noexcept
  {
    return globals_.size();
  }

  std::size_t EngineContext::global_function_count() const noexcept
  {
    return global_functions_.size();
  }

  void EngineContext::clear() noexcept
  {
    globals_.clear();
    global_functions_.clear();
    registry_.clear();
  }

  const EngineContext::FunctionMap &EngineContext::global_functions() const noexcept
  {
    return global_functions_;
  }

  Error EngineContext::require_initialized() const
  {
    if (!info_.initialized)
    {
      return make_binding_error(
          BindingErrorCode::ContextUnavailable,
          "engine context is not initialized");
    }

    return ok();
  }

  Error EngineContext::validate_name(
      const std::string &name,
      const char *kind) const
  {
    if (name.empty())
    {
      return make_binding_error(
          BindingErrorCode::InvalidArgument,
          std::string(kind ? kind : "entry") + " name cannot be empty");
    }

    return ok();
  }

  ModuleRegistryOptions EngineContext::registry_options_from_config(
      const BindingConfig &config) noexcept
  {
    ModuleRegistryOptions options;

    options.allow_native_modules =
        config.allow_native_modules &&
        config.module_policy != ModulePolicy::Disabled;

    options.allow_builtin_modules =
        config.module_policy != ModulePolicy::Disabled;

    options.allow_script_modules =
        config.module_policy == ModulePolicy::Full;

    options.allow_overwrite = true;

    return options;
  }

} // namespace kordex::bindings
