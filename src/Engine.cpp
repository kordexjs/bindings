/**
 *
 *  @file Engine.cpp
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

#include <utility>
#include <kordex/bindings/Engine.hpp>
#include <kordex/bindings/backend/NativeBackendDriver.hpp>
#include <kordex/bindings/backend/QuickJsBackendDriver.hpp>

namespace kordex::bindings
{
  bool EngineInfo::has_name() const noexcept
  {
    return !name.empty();
  }

  bool EngineInfo::running() const noexcept
  {
    return state == EngineState::Running;
  }

  bool EngineInfo::stopped() const noexcept
  {
    return state == EngineState::Stopped;
  }

  bool EngineInfo::failed() const noexcept
  {
    return state == EngineState::Failed;
  }

  Engine::Engine()
      : Engine(BindingConfig{})
  {
  }
  Engine::Engine(BindingConfig config)
      : config_(std::move(config)),
        info_(),
        context_(config_),
        backend_driver_()
  {
    info_.backend = config_.backend;
    info_.name = config_.engine_name.empty()
                     ? "kordex-bindings"
                     : config_.engine_name;
  }

  Engine::Engine(
      BindingConfig config,
      std::string name)
      : config_(std::move(config)),
        info_(),
        context_(config_),
        backend_driver_()
  {
    info_.backend = config_.backend;
    info_.name = std::move(name);
  }

  Result<Engine> Engine::create()
  {
    return create(BindingOptions::defaults());
  }

  Result<Engine> Engine::create(
      const BindingOptions &options)
  {
    auto config = BindingConfig::from_options(options);
    if (!config)
    {
      return config.error();
    }

    return create(config.value());
  }

  Result<Engine> Engine::create(
      BindingConfig config)
  {
    const auto validation = config.validate();
    if (validation)
    {
      return validation;
    }

    return Engine(std::move(config));
  }

  const EngineInfo &Engine::info() const noexcept
  {
    return info_;
  }

  const std::string &Engine::name() const noexcept
  {
    return info_.name;
  }

  void Engine::set_name(std::string name)
  {
    info_.name = std::move(name);
    context_.set_name(info_.name.empty() ? "default" : info_.name);
  }

  EngineBackend Engine::backend() const noexcept
  {
    return info_.backend;
  }

  EngineState Engine::state() const noexcept
  {
    return info_.state;
  }

  bool Engine::initialized() const noexcept
  {
    return info_.initialized;
  }

  bool Engine::running() const noexcept
  {
    return info_.running();
  }

  const BindingConfig &Engine::config() const noexcept
  {
    return config_;
  }

  BindingConfig &Engine::config() noexcept
  {
    return config_;
  }

  EngineContext &Engine::context() noexcept
  {
    return context_;
  }

  const EngineContext &Engine::context() const noexcept
  {
    return context_;
  }

  BindingResult Engine::initialize()
  {
    if (info_.initialized && info_.running())
    {
      return BindingResult::success("engine already initialized");
    }

    const auto validation = validate();
    if (validation)
    {
      mark_state(EngineState::Failed);

      return BindingResult::failure(
          make_binding_error(
              BindingErrorCode::EngineInitializationFailed,
              std::string(validation.message())),
          1);
    }

    mark_state(EngineState::Initializing);

    context_ = EngineContext(config_, info_.name.empty() ? "default" : info_.name);

    const auto context_error = context_.initialize();
    if (context_error)
    {
      mark_state(EngineState::Failed);

      return BindingResult::failure(
          make_binding_error(
              BindingErrorCode::EngineInitializationFailed,
              std::string(context_error.message())),
          1);
    }

    const auto backend_error = create_backend_driver();
    if (backend_error)
    {
      mark_state(EngineState::Failed);

      return BindingResult::failure(
          make_binding_error(
              BindingErrorCode::EngineInitializationFailed,
              std::string(backend_error.message())),
          1);
    }

    const auto backend_init = backend_driver_->initialize(context_);
    if (!backend_init.succeeded())
    {
      mark_state(EngineState::Failed);
      return backend_init;
    }

    info_.initialized = true;
    mark_state(EngineState::Running);

    return BindingResult::success("engine initialized");
  }

  BindingResult Engine::shutdown()
  {
    if (!info_.initialized && info_.stopped())
    {
      return BindingResult::success("engine already stopped");
    }

    mark_state(EngineState::Stopping);

    if (backend_driver_)
    {
      const auto backend_result = backend_driver_->shutdown(context_);
      if (!backend_result.succeeded())
      {
        mark_state(EngineState::Failed);
        return backend_result;
      }

      backend_driver_.reset();
    }

    const auto context_error = context_.shutdown();
    if (context_error)
    {
      mark_state(EngineState::Failed);

      return BindingResult::failure(
          make_binding_error(
              BindingErrorCode::InternalError,
              std::string(context_error.message())),
          1);
    }

    info_.initialized = false;
    mark_state(EngineState::Stopped);

    return BindingResult::success("engine stopped");
  }

  Error Engine::validate() const
  {
    if (!info_.has_name())
    {
      return make_binding_error(
          BindingErrorCode::InvalidConfig,
          "engine name cannot be empty");
    }

    const auto config_validation = config_.validate();
    if (config_validation)
    {
      return config_validation;
    }

    if (config_.backend != info_.backend)
    {
      return make_binding_error(
          BindingErrorCode::InvalidConfig,
          "engine backend does not match binding config backend");
    }

    return ok();
  }

  Error Engine::register_module(Module module)
  {
    const auto state_error = require_running();
    if (state_error)
    {
      return state_error;
    }

    return context_.register_module(std::move(module));
  }

  Error Engine::register_native_module(
      const NativeModule &native_module)
  {
    const auto state_error = require_running();
    if (state_error)
    {
      return state_error;
    }

    return context_.register_native_module(native_module);
  }

  Result<Module> Engine::import_module(
      const std::string &name) const
  {
    const auto state_error = require_running();
    if (state_error)
    {
      return state_error;
    }

    return context_.import_module(name);
  }

  Error Engine::set_global(
      std::string name,
      Value value)
  {
    const auto state_error = require_running();
    if (state_error)
    {
      return state_error;
    }

    return context_.set_global(std::move(name), std::move(value));
  }

  Result<Value> Engine::global(
      const std::string &name) const
  {
    const auto state_error = require_running();
    if (state_error)
    {
      return state_error;
    }

    return context_.global(name);
  }

  Error Engine::set_global_function(
      std::string name,
      Function function)
  {
    const auto state_error = require_running();
    if (state_error)
    {
      return state_error;
    }

    return context_.set_global_function(
        std::move(name),
        std::move(function));
  }

  Error Engine::add_global_function(Function function)
  {
    const auto state_error = require_running();
    if (state_error)
    {
      return state_error;
    }

    return context_.add_global_function(std::move(function));
  }

  Result<Value> Engine::call_global_function(
      const std::string &name,
      const FunctionArguments &args) const
  {
    const auto state_error = require_running();
    if (state_error)
    {
      return state_error;
    }

    return context_.call_global_function(name, args);
  }

  ScriptResult Engine::run_script(Script script)
  {
    const auto state_error = require_running();
    if (state_error)
    {
      return ScriptResult::failure(
          make_binding_error(
              BindingErrorCode::EngineUnavailable,
              std::string(state_error.message())),
          1);
    }

    if (!backend_driver_)
    {
      return ScriptResult::failure(
          make_binding_error(
              BindingErrorCode::EngineUnavailable,
              "engine backend driver is not available"),
          1);
    }

    return backend_driver_->run_script(
        context_,
        std::move(script));
  }

  ScriptResult Engine::eval(
      std::string source,
      std::string name)
  {
    const auto state_error = require_running();
    if (state_error)
    {
      return ScriptResult::failure(
          make_binding_error(
              BindingErrorCode::EngineUnavailable,
              std::string(state_error.message())),
          1);
    }

    if (!backend_driver_)
    {
      return ScriptResult::failure(
          make_binding_error(
              BindingErrorCode::EngineUnavailable,
              "engine backend driver is not available"),
          1);
    }

    return backend_driver_->eval(
        context_,
        std::move(source),
        std::move(name));
  }

  std::size_t Engine::module_count() const noexcept
  {
    return context_.registry().size();
  }

  std::size_t Engine::global_count() const noexcept
  {
    return context_.global_count();
  }

  std::size_t Engine::global_function_count() const noexcept
  {
    return context_.global_function_count();
  }

  void Engine::clear() noexcept
  {
    context_.clear();
  }

  Error Engine::require_running() const
  {
    if (!info_.initialized || !info_.running())
    {
      return make_binding_error(
          BindingErrorCode::EngineUnavailable,
          "engine is not running");
    }

    return ok();
  }

  Error Engine::create_backend_driver()
  {
    switch (config_.backend)
    {
    case EngineBackend::Native:
#if defined(KORDEX_BINDINGS_ENABLE_NATIVE_ENGINE) && KORDEX_BINDINGS_ENABLE_NATIVE_ENGINE
      backend_driver_ = std::make_unique<NativeBackendDriver>();
      return ok();
#else
      return make_binding_error(
          BindingErrorCode::EngineUnavailable,
          "native backend is disabled in this build");
#endif

    case EngineBackend::QuickJS:
#if defined(KORDEX_BINDINGS_ENABLE_QUICKJS) && KORDEX_BINDINGS_ENABLE_QUICKJS
      backend_driver_ = std::make_unique<QuickJsBackendDriver>();
      return ok();
#else
      return make_binding_error(
          BindingErrorCode::EngineUnavailable,
          "QuickJS backend is disabled in this build");
#endif

    case EngineBackend::V8:
      return make_binding_error(
          BindingErrorCode::EngineUnavailable,
          "V8 backend is not connected yet");
    }

    return make_binding_error(
        BindingErrorCode::EngineUnavailable,
        "unknown engine backend");
  }

  void Engine::mark_state(EngineState state) noexcept
  {
    info_.state = state;
  }

  const char *to_string(EngineState state) noexcept
  {
    switch (state)
    {
    case EngineState::Created:
      return "created";
    case EngineState::Initializing:
      return "initializing";
    case EngineState::Running:
      return "running";
    case EngineState::Stopping:
      return "stopping";
    case EngineState::Stopped:
      return "stopped";
    case EngineState::Failed:
      return "failed";
    }

    return "failed";
  }

} // namespace kordex::bindings
