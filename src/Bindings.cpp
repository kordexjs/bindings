/**
 *
 *  @file Bindings.cpp
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
#include <kordex/bindings/Bindings.hpp>

namespace kordex::bindings
{
  std::string_view package_name() noexcept
  {
    return "kordex-bindings";
  }

  std::string_view package_description() noexcept
  {
    return "Native bindings layer for Kordex.";
  }

  bool native_engine_enabled() noexcept
  {
#if defined(KORDEX_BINDINGS_ENABLE_NATIVE_ENGINE) && KORDEX_BINDINGS_ENABLE_NATIVE_ENGINE
    return true;
#else
    return false;
#endif
  }

  bool quickjs_enabled() noexcept
  {
#if defined(KORDEX_BINDINGS_ENABLE_QUICKJS) && KORDEX_BINDINGS_ENABLE_QUICKJS
    return true;
#else
    return false;
#endif
  }

  bool v8_enabled() noexcept
  {
#if defined(KORDEX_BINDINGS_ENABLE_V8) && KORDEX_BINDINGS_ENABLE_V8
    return true;
#else
    return false;
#endif
  }

  EngineBackend default_backend() noexcept
  {
    if (quickjs_enabled())
    {
      return EngineBackend::QuickJS;
    }

    if (v8_enabled())
    {
      return EngineBackend::V8;
    }

    return EngineBackend::Native;
  }

  Result<Engine> create_engine()
  {
    BindingOptions options = BindingOptions::defaults();
    options.backend = default_backend();

    return Engine::create(options);
  }

  Result<Engine> create_engine(
      const BindingOptions &options)
  {
    return Engine::create(options);
  }

  Result<Engine> create_engine(
      BindingConfig config)
  {
    return Engine::create(std::move(config));
  }

  Result<EngineContext> create_context()
  {
    BindingOptions options = BindingOptions::defaults();
    options.backend = default_backend();

    return EngineContext::create(options);
  }

  Result<EngineContext> create_context(
      const BindingOptions &options)
  {
    return EngineContext::create(options);
  }

  Result<RuntimeBridge> create_runtime_bridge()
  {
    return RuntimeBridge::create();
  }

  ScriptResult run_source(
      std::string source,
      std::string name)
  {
    auto engine_result = create_engine();
    if (!engine_result)
    {
      return ScriptResult::failure(
          engine_result.error(),
          1);
    }

    auto engine = std::move(engine_result.value());

    auto init_result = engine.initialize();
    if (!init_result.succeeded())
    {
      return ScriptResult::from_binding_result(init_result);
    }

    auto result = engine.eval(
        std::move(source),
        std::move(name));

    (void)engine.shutdown();

    return result;
  }

  ScriptResult run_script(
      Script script)
  {
    auto engine_result = create_engine();
    if (!engine_result)
    {
      return ScriptResult::failure(
          engine_result.error(),
          1);
    }

    auto engine = std::move(engine_result.value());

    auto init_result = engine.initialize();
    if (!init_result.succeeded())
    {
      return ScriptResult::from_binding_result(init_result);
    }

    auto result = engine.run_script(std::move(script));

    (void)engine.shutdown();

    return result;
  }

} // namespace kordex::bindings
