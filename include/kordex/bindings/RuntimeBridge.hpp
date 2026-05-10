/**
 *
 *  @file RuntimeBridge.hpp
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

#ifndef KORDEX_BINDINGS_RUNTIME_BRIDGE_HPP
#define KORDEX_BINDINGS_RUNTIME_BRIDGE_HPP

#include <memory>
#include <string>

#include <kordex/runtime/Manifest.hpp>
#include <kordex/runtime/ModuleResolver.hpp>
#include <kordex/runtime/Runtime.hpp>
#include <kordex/runtime/RuntimeConfig.hpp>
#include <kordex/runtime/RuntimeOptions.hpp>
#include <kordex/runtime/RuntimeResult.hpp>
#include <kordex/runtime/SourceFile.hpp>

#include <kordex/bindings/BindingConfig.hpp>
#include <kordex/bindings/BindingOptions.hpp>
#include <kordex/bindings/BindingResult.hpp>
#include <kordex/bindings/EngineContext.hpp>
#include <kordex/bindings/Error.hpp>
#include <kordex/bindings/Module.hpp>
#include <kordex/bindings/Result.hpp>
#include <kordex/bindings/Script.hpp>
#include <kordex/bindings/ScriptResult.hpp>
#include <kordex/bindings/Value.hpp>

namespace kordex::bindings
{
  /**
   * @struct RuntimeBridgeOptions
   * @brief Options controlling the bridge between bindings and runtime.
   */
  struct RuntimeBridgeOptions
  {
    /**
     * @brief Whether the bridge owns the runtime instance.
     */
    bool owns_runtime{true};

    /**
     * @brief Whether the bridge should start the runtime automatically.
     */
    bool auto_start_runtime{false};

    /**
     * @brief Whether the bridge can load files through runtime.
     */
    bool allow_source_loading{true};

    /**
     * @brief Whether the bridge can resolve modules through runtime.
     */
    bool allow_module_resolution{true};

    /**
     * @brief Whether the bridge can load manifests through runtime.
     */
    bool allow_manifest_loading{true};

    /**
     * @brief Whether the bridge can run runtime tasks.
     */
    bool allow_tasks{true};
  };

  /**
   * @struct RuntimeBridgeInfo
   * @brief Metadata describing the runtime bridge.
   */
  struct RuntimeBridgeInfo
  {
    /**
     * @brief Bridge name.
     */
    std::string name{"runtime-bridge"};

    /**
     * @brief Whether the bridge is attached to a runtime.
     */
    bool attached{false};

    /**
     * @brief Whether the bridge has been initialized.
     */
    bool initialized{false};

    /**
     * @brief Return true if the bridge has a name.
     */
    [[nodiscard]] bool has_name() const noexcept;
  };

  /**
   * @class RuntimeBridge
   * @brief Bridge between kordex::runtime and kordex::bindings.
   *
   * RuntimeBridge connects the native runtime layer to the bindings layer.
   * It does not own script execution logic. Instead, it:
   * - loads SourceFile through runtime
   * - converts SourceFile to Script
   * - resolves runtime modules into bindings Module metadata
   * - loads manifests
   * - forwards run_file/run_source to runtime placeholder execution
   * - exposes runtime state as binding values
   */
  class RuntimeBridge
  {
  public:
    /**
     * @brief Construct a bridge with an owned default runtime.
     */
    RuntimeBridge();

    /**
     * @brief Construct a bridge with explicit runtime options.
     */
    explicit RuntimeBridge(kordex::runtime::RuntimeOptions runtime_options);

    /**
     * @brief Construct a bridge around an existing runtime reference.
     */
    explicit RuntimeBridge(kordex::runtime::Runtime &runtime);

    RuntimeBridge(const RuntimeBridge &) = delete;
    RuntimeBridge &operator=(const RuntimeBridge &) = delete;

    RuntimeBridge(RuntimeBridge &&) noexcept;
    RuntimeBridge &operator=(RuntimeBridge &&) noexcept;

    /**
     * @brief Destroy the bridge.
     */
    ~RuntimeBridge();

    /**
     * @brief Create a bridge with an owned default runtime.
     */
    [[nodiscard]] static Result<RuntimeBridge> create();

    /**
     * @brief Create a bridge with explicit runtime options.
     */
    [[nodiscard]] static Result<RuntimeBridge> create(
        const kordex::runtime::RuntimeOptions &runtime_options);

    /**
     * @brief Create a bridge around an existing runtime.
     */
    [[nodiscard]] static Result<RuntimeBridge> attach(
        kordex::runtime::Runtime &runtime);

    /**
     * @brief Return bridge metadata.
     */
    [[nodiscard]] const RuntimeBridgeInfo &info() const noexcept;

    /**
     * @brief Return bridge name.
     */
    [[nodiscard]] const std::string &name() const noexcept;

    /**
     * @brief Set bridge name.
     */
    void set_name(std::string name);

    /**
     * @brief Return bridge options.
     */
    [[nodiscard]] const RuntimeBridgeOptions &options() const noexcept;

    /**
     * @brief Set bridge options.
     */
    void set_options(RuntimeBridgeOptions options);

    /**
     * @brief Return true if the bridge is attached to a runtime.
     */
    [[nodiscard]] bool attached() const noexcept;

    /**
     * @brief Return true if the bridge is initialized.
     */
    [[nodiscard]] bool initialized() const noexcept;

    /**
     * @brief Return true if the bridge owns its runtime.
     */
    [[nodiscard]] bool owns_runtime() const noexcept;

    /**
     * @brief Return the runtime.
     */
    [[nodiscard]] kordex::runtime::Runtime &runtime();

    /**
     * @brief Return the runtime.
     */
    [[nodiscard]] const kordex::runtime::Runtime &runtime() const;

    /**
     * @brief Initialize the bridge.
     */
    [[nodiscard]] BindingResult initialize();

    /**
     * @brief Shutdown the bridge.
     */
    [[nodiscard]] BindingResult shutdown();

    /**
     * @brief Validate bridge state.
     */
    [[nodiscard]] Error validate() const;

    /**
     * @brief Load a runtime source file and convert it to Script.
     */
    [[nodiscard]] Result<Script> load_script(
        const std::string &path) const;

    /**
     * @brief Load a runtime source file.
     */
    [[nodiscard]] Result<kordex::runtime::SourceFile> load_source(
        const std::string &path) const;

    /**
     * @brief Resolve a runtime module and convert it to a bindings Module.
     */
    [[nodiscard]] Result<Module> resolve_module(
        const std::string &specifier) const;

    /**
     * @brief Load a runtime manifest.
     */
    [[nodiscard]] Result<kordex::runtime::Manifest> load_manifest(
        const std::string &path) const;

    /**
     * @brief Run a file through the runtime placeholder execution path.
     */
    [[nodiscard]] ScriptResult run_file(
        const std::string &path);

    /**
     * @brief Run a script through the bindings placeholder execution path.
     */
    [[nodiscard]] ScriptResult run_script(
        Script script);

    /**
     * @brief Run source code through the bindings placeholder execution path.
     */
    [[nodiscard]] ScriptResult eval(
        std::string source,
        std::string name = "eval.js");

    /**
     * @brief Register bridge-provided globals into an engine context.
     */
    [[nodiscard]] Error install_into(
        EngineContext &context) const;

    /**
     * @brief Return runtime state as a bindings value.
     */
    [[nodiscard]] Value runtime_state_value() const;

    /**
     * @brief Return runtime running status as a bindings value.
     */
    [[nodiscard]] Value runtime_running_value() const;

    /**
     * @brief Convert a runtime result to a script result.
     */
    [[nodiscard]] static ScriptResult from_runtime_result(
        const kordex::runtime::RuntimeResult &result);

    /**
     * @brief Convert a runtime source file to a script.
     */
    [[nodiscard]] static Result<Script> script_from_source_file(
        const kordex::runtime::SourceFile &source_file);

    /**
     * @brief Convert a runtime resolved module to bindings module metadata.
     */
    [[nodiscard]] static Module module_from_resolved_module(
        const kordex::runtime::ResolvedModule &resolved_module);

  private:
    [[nodiscard]] Error require_initialized() const;

    [[nodiscard]] Error require_runtime() const;

    [[nodiscard]] Error permission_error(
        const char *message) const;

    std::unique_ptr<kordex::runtime::Runtime> owned_runtime_{};
    kordex::runtime::Runtime *runtime_{nullptr};

    RuntimeBridgeOptions options_{};
    RuntimeBridgeInfo info_{};
  };

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_RUNTIME_BRIDGE_HPP
