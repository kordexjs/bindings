/**
 *
 *  @file Engine.hpp
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

#ifndef KORDEX_BINDINGS_ENGINE_HPP
#define KORDEX_BINDINGS_ENGINE_HPP

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <kordex/bindings/BindingConfig.hpp>
#include <kordex/bindings/BindingOptions.hpp>
#include <kordex/bindings/BindingResult.hpp>
#include <kordex/bindings/EngineContext.hpp>
#include <kordex/bindings/Error.hpp>
#include <kordex/bindings/Module.hpp>
#include <kordex/bindings/NativeModule.hpp>
#include <kordex/bindings/Result.hpp>
#include <kordex/bindings/Script.hpp>
#include <kordex/bindings/ScriptResult.hpp>
#include <kordex/bindings/Value.hpp>

namespace kordex::bindings
{
  /**
   * @enum EngineState
   * @brief Lifecycle state of a bindings engine.
   */
  enum class EngineState
  {
    Created,
    Initializing,
    Running,
    Stopping,
    Stopped,
    Failed
  };

  /**
   * @struct EngineInfo
   * @brief Metadata describing the bindings engine.
   */
  struct EngineInfo
  {
    /**
     * @brief Engine name.
     */
    std::string name{"kordex-bindings"};

    /**
     * @brief Engine backend.
     */
    EngineBackend backend{EngineBackend::Native};

    /**
     * @brief Engine lifecycle state.
     */
    EngineState state{EngineState::Created};

    /**
     * @brief Whether the engine has been initialized.
     */
    bool initialized{false};

    /**
     * @brief Return true if the engine has a name.
     */
    [[nodiscard]] bool has_name() const noexcept;

    /**
     * @brief Return true if the engine is running.
     */
    [[nodiscard]] bool running() const noexcept;

    /**
     * @brief Return true if the engine is stopped.
     */
    [[nodiscard]] bool stopped() const noexcept;

    /**
     * @brief Return true if the engine failed.
     */
    [[nodiscard]] bool failed() const noexcept;
  };

  /**
   * @class Engine
   * @brief Top-level bindings engine facade.
   *
   * Engine owns the bindings configuration and a default EngineContext.
   * It exposes high-level APIs for initialization, shutdown, script
   * execution, module registration, and context access.
   *
   * The current backend is a native placeholder backend. QuickJS or V8 can
   * later be connected behind the same public API.
   */
  class Engine
  {
  public:
    /**
     * @brief Construct an engine with default configuration.
     */
    Engine();

    /**
     * @brief Construct an engine with explicit configuration.
     */
    explicit Engine(BindingConfig config);

    /**
     * @brief Construct an engine with explicit configuration and name.
     */
    Engine(BindingConfig config, std::string name);

    /**
     * @brief Create an engine from default options.
     */
    [[nodiscard]] static Result<Engine> create();

    /**
     * @brief Create an engine from explicit options.
     */
    [[nodiscard]] static Result<Engine> create(
        const BindingOptions &options);

    /**
     * @brief Create an engine from explicit config.
     */
    [[nodiscard]] static Result<Engine> create(
        BindingConfig config);

    /**
     * @brief Return engine metadata.
     */
    [[nodiscard]] const EngineInfo &info() const noexcept;

    /**
     * @brief Return engine name.
     */
    [[nodiscard]] const std::string &name() const noexcept;

    /**
     * @brief Set engine name.
     */
    void set_name(std::string name);

    /**
     * @brief Return engine backend.
     */
    [[nodiscard]] EngineBackend backend() const noexcept;

    /**
     * @brief Return engine state.
     */
    [[nodiscard]] EngineState state() const noexcept;

    /**
     * @brief Return true if the engine is initialized.
     */
    [[nodiscard]] bool initialized() const noexcept;

    /**
     * @brief Return true if the engine is running.
     */
    [[nodiscard]] bool running() const noexcept;

    /**
     * @brief Return engine configuration.
     */
    [[nodiscard]] const BindingConfig &config() const noexcept;

    /**
     * @brief Return mutable engine configuration.
     */
    [[nodiscard]] BindingConfig &config() noexcept;

    /**
     * @brief Return default engine context.
     */
    [[nodiscard]] EngineContext &context() noexcept;

    /**
     * @brief Return default engine context.
     */
    [[nodiscard]] const EngineContext &context() const noexcept;

    /**
     * @brief Initialize the engine and default context.
     */
    [[nodiscard]] BindingResult initialize();

    /**
     * @brief Shutdown the engine and default context.
     */
    [[nodiscard]] BindingResult shutdown();

    /**
     * @brief Validate engine configuration and metadata.
     */
    [[nodiscard]] Error validate() const;

    /**
     * @brief Register a module in the default context.
     */
    [[nodiscard]] Error register_module(Module module);

    /**
     * @brief Register a native module in the default context.
     */
    [[nodiscard]] Error register_native_module(
        const NativeModule &native_module);

    /**
     * @brief Import a module from the default context.
     */
    [[nodiscard]] Result<Module> import_module(
        const std::string &name) const;

    /**
     * @brief Set a global value in the default context.
     */
    [[nodiscard]] Error set_global(
        std::string name,
        Value value);

    /**
     * @brief Return a global value from the default context.
     */
    [[nodiscard]] Result<Value> global(
        const std::string &name) const;

    /**
     * @brief Register a global function in the default context.
     */
    [[nodiscard]] Error set_global_function(
        std::string name,
        Function function);

    /**
     * @brief Register a global function using its own name.
     */
    [[nodiscard]] Error add_global_function(Function function);

    /**
     * @brief Call a global function from the default context.
     */
    [[nodiscard]] Result<Value> call_global_function(
        const std::string &name,
        const FunctionArguments &args = {}) const;

    /**
     * @brief Run a script in the default context.
     */
    [[nodiscard]] ScriptResult run_script(Script script);

    /**
     * @brief Evaluate source code in the default context.
     */
    [[nodiscard]] ScriptResult eval(
        std::string source,
        std::string name = "eval.js");

    /**
     * @brief Return the number of registered modules.
     */
    [[nodiscard]] std::size_t module_count() const noexcept;

    /**
     * @brief Return the number of global values.
     */
    [[nodiscard]] std::size_t global_count() const noexcept;

    /**
     * @brief Return the number of global functions.
     */
    [[nodiscard]] std::size_t global_function_count() const noexcept;

    /**
     * @brief Clear the default context state.
     */
    void clear() noexcept;

  private:
    [[nodiscard]] Error require_running() const;

    void mark_state(EngineState state) noexcept;

    BindingConfig config_{};
    EngineInfo info_{};
    EngineContext context_{};
  };

  /**
   * @brief Convert EngineState to a stable string.
   */
  [[nodiscard]] const char *to_string(EngineState state) noexcept;

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_ENGINE_HPP
