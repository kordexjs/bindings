/**
 *
 *  @file Bindings.hpp
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

#ifndef KORDEX_BINDINGS_BINDINGS_HPP
#define KORDEX_BINDINGS_BINDINGS_HPP

#include <string_view>

#include <kordex/bindings/BindingConfig.hpp>
#include <kordex/bindings/BindingOptions.hpp>
#include <kordex/bindings/BindingResult.hpp>
#include <kordex/bindings/Engine.hpp>
#include <kordex/bindings/EngineContext.hpp>
#include <kordex/bindings/Error.hpp>
#include <kordex/bindings/Function.hpp>
#include <kordex/bindings/Module.hpp>
#include <kordex/bindings/ModuleRegistry.hpp>
#include <kordex/bindings/NativeFunction.hpp>
#include <kordex/bindings/NativeModule.hpp>
#include <kordex/bindings/Object.hpp>
#include <kordex/bindings/Result.hpp>
#include <kordex/bindings/RuntimeBridge.hpp>
#include <kordex/bindings/Script.hpp>
#include <kordex/bindings/ScriptResult.hpp>
#include <kordex/bindings/Value.hpp>
#include <kordex/bindings/ValueType.hpp>
#include <kordex/bindings/Version.hpp>
#include <kordex/bindings/TypeScriptLoader.hpp>

namespace kordex::bindings
{
  /**
   * @brief Return the public Kordex bindings package name.
   */
  [[nodiscard]] std::string_view package_name() noexcept;

  /**
   * @brief Return the public Kordex bindings package description.
   */
  [[nodiscard]] std::string_view package_description() noexcept;

  /**
   * @brief Return true if the built-in native placeholder engine is enabled.
   */
  [[nodiscard]] bool native_engine_enabled() noexcept;

  /**
   * @brief Return true if QuickJS backend support is enabled.
   */
  [[nodiscard]] bool quickjs_enabled() noexcept;

  /**
   * @brief Return true if V8 backend support is enabled.
   */
  [[nodiscard]] bool v8_enabled() noexcept;

  /**
   * @brief Return the default engine backend available in this build.
   */
  [[nodiscard]] EngineBackend default_backend() noexcept;

  /**
   * @brief Create a default bindings engine.
   */
  [[nodiscard]] Result<Engine> create_engine();

  /**
   * @brief Create a bindings engine from options.
   */
  [[nodiscard]] Result<Engine> create_engine(
      const BindingOptions &options);

  /**
   * @brief Create a bindings engine from config.
   */
  [[nodiscard]] Result<Engine> create_engine(
      BindingConfig config);

  /**
   * @brief Create a default engine context.
   */
  [[nodiscard]] Result<EngineContext> create_context();

  /**
   * @brief Create an engine context from options.
   */
  [[nodiscard]] Result<EngineContext> create_context(
      const BindingOptions &options);

  /**
   * @brief Create a default runtime bridge.
   */
  [[nodiscard]] Result<RuntimeBridge> create_runtime_bridge();

  /**
   * @brief Run source code with a temporary native placeholder engine.
   */
  [[nodiscard]] ScriptResult run_source(
      std::string source,
      std::string name = "main.js");

  /**
   * @brief Run a script with a temporary native placeholder engine.
   */
  [[nodiscard]] ScriptResult run_script(
      Script script);

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_BINDINGS_HPP
