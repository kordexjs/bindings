/**
 *
 *  @file EngineContext.hpp
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

#ifndef KORDEX_BINDINGS_ENGINE_CONTEXT_HPP
#define KORDEX_BINDINGS_ENGINE_CONTEXT_HPP

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include <kordex/bindings/BindingConfig.hpp>
#include <kordex/bindings/BindingOptions.hpp>
#include <kordex/bindings/Error.hpp>
#include <kordex/bindings/Function.hpp>
#include <kordex/bindings/Module.hpp>
#include <kordex/bindings/ModuleRegistry.hpp>
#include <kordex/bindings/NativeModule.hpp>
#include <kordex/bindings/Object.hpp>
#include <kordex/bindings/Result.hpp>
#include <kordex/bindings/Script.hpp>
#include <kordex/bindings/ScriptResult.hpp>
#include <kordex/bindings/Value.hpp>

namespace kordex::bindings
{
  /**
   * @struct EngineContextInfo
   * @brief Metadata describing an engine context.
   */
  struct EngineContextInfo
  {
    /**
     * @brief Context name.
     */
    std::string name{"default"};

    /**
     * @brief Optional context identifier.
     */
    std::string id{};

    /**
     * @brief Whether the context is initialized.
     */
    bool initialized{false};

    /**
     * @brief Return true if the context has a name.
     */
    [[nodiscard]] bool has_name() const noexcept;

    /**
     * @brief Return true if the context has an id.
     */
    [[nodiscard]] bool has_id() const noexcept;
  };

  /**
   * @class EngineContext
   * @brief Engine-independent execution context.
   *
   * EngineContext owns global values, global functions, and a module registry.
   * It is intentionally independent from a concrete JavaScript backend.
   *
   * A real backend such as QuickJS or V8 can later use this class as the
   * stable C++ state container for scripts, modules, globals, and bridges.
   */
  class EngineContext
  {
  public:
    /**
     * @brief Global function storage type.
     */
    using FunctionMap = std::map<std::string, Function>;

    /**
     * @brief Construct a context with default config.
     */
    EngineContext();

    /**
     * @brief Construct a context with explicit config.
     */
    explicit EngineContext(BindingConfig config);

    /**
     * @brief Construct a context with explicit config and name.
     */
    EngineContext(BindingConfig config, std::string name);

    /**
     * @brief Create a context from default options.
     */
    [[nodiscard]] static Result<EngineContext> create();

    /**
     * @brief Create a context from explicit options.
     */
    [[nodiscard]] static Result<EngineContext> create(
        const BindingOptions &options);

    /**
     * @brief Create a context from explicit config.
     */
    [[nodiscard]] static Result<EngineContext> create(
        BindingConfig config);

    /**
     * @brief Return context metadata.
     */
    [[nodiscard]] const EngineContextInfo &info() const noexcept;

    /**
     * @brief Return context name.
     */
    [[nodiscard]] const std::string &name() const noexcept;

    /**
     * @brief Return context id.
     */
    [[nodiscard]] const std::string &id() const noexcept;

    /**
     * @brief Set context id.
     */
    void set_id(std::string id);

    /**
     * @brief Set context name.
     */
    void set_name(std::string name);

    /**
     * @brief Return context config.
     */
    [[nodiscard]] const BindingConfig &config() const noexcept;

    /**
     * @brief Return mutable context config.
     */
    [[nodiscard]] BindingConfig &config() noexcept;

    /**
     * @brief Return module registry.
     */
    [[nodiscard]] const ModuleRegistry &registry() const noexcept;

    /**
     * @brief Return mutable module registry.
     */
    [[nodiscard]] ModuleRegistry &registry() noexcept;

    /**
     * @brief Return global object.
     */
    [[nodiscard]] const Object &globals() const noexcept;

    /**
     * @brief Return mutable global object.
     */
    [[nodiscard]] Object &globals() noexcept;

    /**
     * @brief Initialize the context.
     */
    [[nodiscard]] Error initialize();

    /**
     * @brief Shutdown the context.
     */
    [[nodiscard]] Error shutdown();

    /**
     * @brief Return true if the context is initialized.
     */
    [[nodiscard]] bool initialized() const noexcept;

    /**
     * @brief Validate context configuration.
     */
    [[nodiscard]] Error validate() const;

    /**
     * @brief Set a global value.
     */
    [[nodiscard]] Error set_global(
        std::string name,
        Value value);

    /**
     * @brief Return a global value or an error.
     */
    [[nodiscard]] Result<Value> global(
        const std::string &name) const;

    /**
     * @brief Return true if a global value exists.
     */
    [[nodiscard]] bool has_global(
        const std::string &name) const noexcept;

    /**
     * @brief Remove a global value.
     */
    [[nodiscard]] Error remove_global(
        const std::string &name);

    /**
     * @brief Register a global function.
     */
    [[nodiscard]] Error set_global_function(
        std::string name,
        Function function);

    /**
     * @brief Register a global function using its own name.
     */
    [[nodiscard]] Error add_global_function(Function function);

    /**
     * @brief Return a global function or an error.
     */
    [[nodiscard]] Result<Function> global_function(
        const std::string &name) const;

    /**
     * @brief Return true if a global function exists.
     */
    [[nodiscard]] bool has_global_function(
        const std::string &name) const noexcept;

    /**
     * @brief Remove a global function.
     */
    [[nodiscard]] Error remove_global_function(
        const std::string &name);

    /**
     * @brief Call a global function.
     */
    [[nodiscard]] Result<Value> call_global_function(
        const std::string &name,
        const FunctionArguments &args = {}) const;

    /**
     * @brief Register a module in the context registry.
     */
    [[nodiscard]] Error register_module(Module module);

    /**
     * @brief Register a native module in the context registry.
     */
    [[nodiscard]] Error register_native_module(
        const NativeModule &native_module);

    /**
     * @brief Import a module from the context registry.
     */
    [[nodiscard]] Result<Module> import_module(
        const std::string &name) const;

    /**
     * @brief Run a script using the current placeholder execution model.
     */
    [[nodiscard]] ScriptResult run_script(Script script);

    /**
     * @brief Evaluate source code using the current placeholder execution model.
     */
    [[nodiscard]] ScriptResult eval(
        std::string source,
        std::string name = "eval.js");

    /**
     * @brief Return global value names.
     */
    [[nodiscard]] std::vector<std::string> global_names() const;

    /**
     * @brief Return global function names.
     */
    [[nodiscard]] std::vector<std::string> global_function_names() const;

    /**
     * @brief Return the number of global values.
     */
    [[nodiscard]] std::size_t global_count() const noexcept;

    /**
     * @brief Return the number of global functions.
     */
    [[nodiscard]] std::size_t global_function_count() const noexcept;

    /**
     * @brief Clear globals, global functions, and modules.
     */
    void clear() noexcept;

    /**
     * @brief Return registered global functions.
     */
    [[nodiscard]] const FunctionMap &global_functions() const noexcept;

  private:
    [[nodiscard]] Error require_initialized() const;

    [[nodiscard]] Error validate_name(
        const std::string &name,
        const char *kind) const;

    [[nodiscard]] static ModuleRegistryOptions registry_options_from_config(
        const BindingConfig &config) noexcept;

    BindingConfig config_{};
    EngineContextInfo info_{};
    ModuleRegistry registry_{};
    Object globals_{"global"};
    FunctionMap global_functions_{};
  };

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_ENGINE_CONTEXT_HPP
