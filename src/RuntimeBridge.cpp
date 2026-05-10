/**
 *
 *  @file RuntimeBridge.cpp
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
#include <kordex/bindings/RuntimeBridge.hpp>

namespace kordex::bindings
{
  namespace
  {
    [[nodiscard]] BindingErrorCode runtime_status_to_binding_error(
        kordex::runtime::RuntimeExitStatus status) noexcept
    {
      switch (status)
      {
      case kordex::runtime::RuntimeExitStatus::Success:
        return BindingErrorCode::None;

      case kordex::runtime::RuntimeExitStatus::Failed:
        return BindingErrorCode::RuntimeBridgeFailed;

      case kordex::runtime::RuntimeExitStatus::Cancelled:
        return BindingErrorCode::Cancelled;

      case kordex::runtime::RuntimeExitStatus::Timeout:
        return BindingErrorCode::Timeout;
      }

      return BindingErrorCode::RuntimeBridgeFailed;
    }
  } // namespace

  bool RuntimeBridgeInfo::has_name() const noexcept
  {
    return !name.empty();
  }

  RuntimeBridge::RuntimeBridge()
      : owned_runtime_(std::make_unique<kordex::runtime::Runtime>()),
        runtime_(owned_runtime_.get()),
        options_(),
        info_()
  {
    info_.attached = runtime_ != nullptr;
    options_.owns_runtime = true;
  }

  RuntimeBridge::RuntimeBridge(
      kordex::runtime::RuntimeOptions runtime_options)
      : owned_runtime_(nullptr),
        runtime_(nullptr),
        options_(),
        info_()
  {
    auto runtime_result = kordex::runtime::Runtime::from_options(
        runtime_options);

    if (runtime_result)
    {
      owned_runtime_ =
          std::make_unique<kordex::runtime::Runtime>(
              std::move(runtime_result.value()));

      runtime_ = owned_runtime_.get();
      info_.attached = true;
    }

    options_.owns_runtime = true;
  }

  RuntimeBridge::RuntimeBridge(
      kordex::runtime::Runtime &runtime)
      : owned_runtime_(nullptr),
        runtime_(&runtime),
        options_(),
        info_()
  {
    info_.attached = true;
    options_.owns_runtime = false;
  }

  RuntimeBridge::RuntimeBridge(RuntimeBridge &&other) noexcept
      : owned_runtime_(std::move(other.owned_runtime_)),
        runtime_(nullptr),
        options_(other.options_),
        info_(std::move(other.info_))
  {
    runtime_ = owned_runtime_ ? owned_runtime_.get() : other.runtime_;
    other.runtime_ = nullptr;
    other.info_.attached = false;
    other.info_.initialized = false;
  }

  RuntimeBridge &RuntimeBridge::operator=(RuntimeBridge &&other) noexcept
  {
    if (this != &other)
    {
      if (info_.initialized)
      {
        (void)shutdown();
      }

      owned_runtime_ = std::move(other.owned_runtime_);
      runtime_ = owned_runtime_ ? owned_runtime_.get() : other.runtime_;
      options_ = other.options_;
      info_ = std::move(other.info_);

      other.runtime_ = nullptr;
      other.info_.attached = false;
      other.info_.initialized = false;
    }

    return *this;
  }

  RuntimeBridge::~RuntimeBridge()
  {
    if (info_.initialized)
    {
      (void)shutdown();
    }
  }

  Result<RuntimeBridge> RuntimeBridge::create()
  {
    RuntimeBridge bridge;

    const auto validation = bridge.validate();
    if (validation)
    {
      return validation;
    }

    return bridge;
  }

  Result<RuntimeBridge> RuntimeBridge::create(
      const kordex::runtime::RuntimeOptions &runtime_options)
  {
    RuntimeBridge bridge(runtime_options);

    const auto validation = bridge.validate();
    if (validation)
    {
      return validation;
    }

    return bridge;
  }

  Result<RuntimeBridge> RuntimeBridge::attach(
      kordex::runtime::Runtime &runtime)
  {
    RuntimeBridge bridge(runtime);

    const auto validation = bridge.validate();
    if (validation)
    {
      return validation;
    }

    return bridge;
  }

  const RuntimeBridgeInfo &RuntimeBridge::info() const noexcept
  {
    return info_;
  }

  const std::string &RuntimeBridge::name() const noexcept
  {
    return info_.name;
  }

  void RuntimeBridge::set_name(std::string name)
  {
    info_.name = std::move(name);
  }

  const RuntimeBridgeOptions &RuntimeBridge::options() const noexcept
  {
    return options_;
  }

  void RuntimeBridge::set_options(RuntimeBridgeOptions options)
  {
    const bool actual_ownership = owned_runtime_ != nullptr;

    options_ = options;
    options_.owns_runtime = actual_ownership;
  }

  bool RuntimeBridge::attached() const noexcept
  {
    return info_.attached && runtime_ != nullptr;
  }

  bool RuntimeBridge::initialized() const noexcept
  {
    return info_.initialized;
  }

  bool RuntimeBridge::owns_runtime() const noexcept
  {
    return owned_runtime_ != nullptr;
  }

  kordex::runtime::Runtime &RuntimeBridge::runtime()
  {
    return *runtime_;
  }

  const kordex::runtime::Runtime &RuntimeBridge::runtime() const
  {
    return *runtime_;
  }

  BindingResult RuntimeBridge::initialize()
  {
    if (info_.initialized)
    {
      return BindingResult::success("runtime bridge already initialized");
    }

    const auto validation = validate();
    if (validation)
    {
      return BindingResult::failure(
          make_binding_error(
              BindingErrorCode::RuntimeBridgeFailed,
              std::string(validation.message())),
          1);
    }

    if (options_.auto_start_runtime && runtime_ && !runtime_->is_running())
    {
      const auto start_error = runtime_->start();
      if (start_error)
      {
        return BindingResult::failure(
            make_binding_error(
                BindingErrorCode::RuntimeBridgeFailed,
                std::string(start_error.message())),
            1);
      }
    }

    info_.initialized = true;
    info_.attached = runtime_ != nullptr;

    return BindingResult::success("runtime bridge initialized");
  }

  BindingResult RuntimeBridge::shutdown()
  {
    if (!info_.initialized)
    {
      return BindingResult::success("runtime bridge already stopped");
    }

    if (owns_runtime() && runtime_)
    {
      const auto shutdown_error = runtime_->shutdown();
      if (shutdown_error)
      {
        return BindingResult::failure(
            make_binding_error(
                BindingErrorCode::RuntimeBridgeFailed,
                std::string(shutdown_error.message())),
            1);
      }
    }

    info_.initialized = false;

    return BindingResult::success("runtime bridge stopped");
  }

  Error RuntimeBridge::validate() const
  {
    if (!info_.has_name())
    {
      return make_binding_error(
          BindingErrorCode::InvalidConfig,
          "runtime bridge name cannot be empty");
    }

    if (!runtime_)
    {
      return make_binding_error(
          BindingErrorCode::EngineUnavailable,
          "runtime bridge is not attached to a runtime");
    }

    return ok();
  }

  Result<Script> RuntimeBridge::load_script(
      const std::string &path) const
  {
    auto source_file = load_source(path);
    if (!source_file)
    {
      return source_file.error();
    }

    return script_from_source_file(source_file.value());
  }

  Result<kordex::runtime::SourceFile> RuntimeBridge::load_source(
      const std::string &path) const
  {
    const auto state = require_initialized();
    if (state)
    {
      return state;
    }

    if (!options_.allow_source_loading)
    {
      return permission_error("runtime bridge source loading is disabled");
    }

    auto source = runtime_->load_source(path);
    if (!source)
    {
      return make_binding_error(
          BindingErrorCode::RuntimeBridgeFailed,
          std::string(source.error().message()));
    }

    return source.value();
  }

  Result<Module> RuntimeBridge::resolve_module(
      const std::string &specifier) const
  {
    const auto state = require_initialized();
    if (state)
    {
      return state;
    }

    if (!options_.allow_module_resolution)
    {
      return permission_error("runtime bridge module resolution is disabled");
    }

    auto resolved = runtime_->resolve_module(specifier);
    if (!resolved)
    {
      return make_binding_error(
          BindingErrorCode::ModuleNotFound,
          std::string(resolved.error().message()));
    }

    return module_from_resolved_module(resolved.value());
  }

  Result<kordex::runtime::Manifest> RuntimeBridge::load_manifest(
      const std::string &path) const
  {
    const auto state = require_initialized();
    if (state)
    {
      return state;
    }

    if (!options_.allow_manifest_loading)
    {
      return permission_error("runtime bridge manifest loading is disabled");
    }

    auto manifest = runtime_->load_manifest(path);
    if (!manifest)
    {
      return make_binding_error(
          BindingErrorCode::RuntimeBridgeFailed,
          std::string(manifest.error().message()));
    }

    return manifest.value();
  }

  ScriptResult RuntimeBridge::run_file(
      const std::string &path)
  {
    const auto state = require_initialized();
    if (state)
    {
      return ScriptResult::failure(
          make_binding_error(
              BindingErrorCode::RuntimeBridgeFailed,
              std::string(state.message())),
          1);
    }

    if (!options_.allow_source_loading)
    {
      return ScriptResult::failure(
          permission_error("runtime bridge source loading is disabled"),
          1);
    }

    const auto runtime_result = runtime_->run_file(path);

    return from_runtime_result(runtime_result);
  }

  ScriptResult RuntimeBridge::run_script(
      Script script)
  {
    const auto state = require_initialized();
    if (state)
    {
      return ScriptResult::failure(
          make_binding_error(
              BindingErrorCode::RuntimeBridgeFailed,
              std::string(state.message())),
          1);
    }

    return script.run();
  }

  ScriptResult RuntimeBridge::eval(
      std::string source,
      std::string name)
  {
    Script script = Script::from_source(
        std::move(source),
        std::move(name));

    return run_script(std::move(script));
  }

  Error RuntimeBridge::install_into(
      EngineContext &context) const
  {
    const auto state = require_initialized();
    if (state)
    {
      return state;
    }

    if (!context.initialized())
    {
      return make_binding_error(
          BindingErrorCode::ContextUnavailable,
          "engine context must be initialized before installing runtime bridge");
    }

    const auto runtime_state_error = context.set_global(
        "kordex_runtime_state",
        runtime_state_value());

    if (runtime_state_error)
    {
      return runtime_state_error;
    }

    const auto runtime_running_error = context.set_global(
        "kordex_runtime_running",
        runtime_running_value());

    if (runtime_running_error)
    {
      return runtime_running_error;
    }

    return ok();
  }

  Value RuntimeBridge::runtime_state_value() const
  {
    if (!runtime_)
    {
      return Value::string("detached");
    }

    return Value::string(
        kordex::runtime::to_string(runtime_->state()));
  }

  Value RuntimeBridge::runtime_running_value() const
  {
    return Value::boolean(runtime_ && runtime_->is_running());
  }

  ScriptResult RuntimeBridge::from_runtime_result(
      const kordex::runtime::RuntimeResult &runtime_result)
  {
    if (runtime_result.succeeded())
    {
      return ScriptResult::success(
          Value::string(runtime_result.output),
          runtime_result.output);
    }

    const auto binding_code =
        runtime_status_to_binding_error(runtime_result.status);

    Error error = runtime_result.error.has_error()
                      ? make_binding_error(
                            binding_code,
                            std::string(runtime_result.error.message()))
                      : make_binding_error(
                            binding_code,
                            "runtime operation failed");

    switch (runtime_result.status)
    {
    case kordex::runtime::RuntimeExitStatus::Cancelled:
      return ScriptResult::cancelled(std::string(error.message()));

    case kordex::runtime::RuntimeExitStatus::Timeout:
      return ScriptResult::timeout(std::string(error.message()));

    case kordex::runtime::RuntimeExitStatus::Failed:
    case kordex::runtime::RuntimeExitStatus::Success:
      break;
    }

    return ScriptResult::failure(
        std::move(error),
        runtime_result.exit_code);
  }

  Result<Script> RuntimeBridge::script_from_source_file(
      const kordex::runtime::SourceFile &source_file)
  {
    return Script::from_source_file(source_file);
  }

  Module RuntimeBridge::module_from_resolved_module(
      const kordex::runtime::ResolvedModule &resolved_module)
  {
    ModuleInfo info;
    info.name = resolved_module.id.specifier();
    info.path = resolved_module.path;
    info.kind = resolved_module.id.is_builtin()
                    ? ModuleKind::Builtin
                    : ModuleKind::Script;
    info.importable = true;
    info.loaded = resolved_module.found;

    Module module = Module::create(std::move(info));

    (void)module.set_export(
        "specifier",
        Value::string(resolved_module.id.specifier()));

    (void)module.set_export(
        "path",
        Value::string(resolved_module.path));

    (void)module.set_export(
        "found",
        Value::boolean(resolved_module.found));

    (void)module.set_export(
        "builtin",
        Value::boolean(resolved_module.id.is_builtin()));

    return module;
  }

  Error RuntimeBridge::require_initialized() const
  {
    if (!info_.initialized)
    {
      return make_binding_error(
          BindingErrorCode::RuntimeBridgeFailed,
          "runtime bridge is not initialized");
    }

    return require_runtime();
  }

  Error RuntimeBridge::require_runtime() const
  {
    if (!runtime_)
    {
      return make_binding_error(
          BindingErrorCode::EngineUnavailable,
          "runtime bridge is not attached to a runtime");
    }

    return ok();
  }

  Error RuntimeBridge::permission_error(
      const char *message) const
  {
    return make_binding_error(
        BindingErrorCode::PermissionDenied,
        message ? message : "runtime bridge permission denied");
  }

} // namespace kordex::bindings
