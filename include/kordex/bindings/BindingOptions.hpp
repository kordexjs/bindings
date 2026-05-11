/**
 *
 *  @file BindingOptions.hpp
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

#ifndef KORDEX_BINDINGS_BINDING_OPTIONS_HPP
#define KORDEX_BINDINGS_BINDING_OPTIONS_HPP

#include <cstddef>
#include <string>
#include <vector>

namespace kordex::bindings
{
  /**
   * @enum EngineBackend
   * @brief JavaScript engine backend selected by bindings.
   */
  enum class EngineBackend
  {
    Native,
    QuickJS,
    V8
  };

  /**
   * @enum ModulePolicy
   * @brief Policy used when resolving script modules.
   */
  enum class ModulePolicy
  {
    Disabled,
    BuiltinOnly,
    Full
  };

  /**
   * @struct BindingOptions
   * @brief Explicit options used to configure the bindings layer.
   *
   * BindingOptions represents caller-provided configuration before it is
   * normalized into BindingConfig.
   */
  struct BindingOptions
  {
    /**
     * @brief Engine backend to use.
     */
    EngineBackend backend{EngineBackend::Native};

    /**
     * @brief Module resolution policy.
     */
    ModulePolicy module_policy{ModulePolicy::Full};

    /**
     * @brief Whether native Kordex modules can be registered.
     */
    bool allow_native_modules{true};

    /**
     * @brief Whether native functions can be called from scripts.
     */
    bool allow_native_functions{true};

    /**
     * @brief Whether scripts can access the runtime bridge.
     */
    bool allow_runtime_bridge{true};

    /**
     * @brief Whether script execution diagnostics are enabled.
     */
    bool diagnostics{true};

    /**
     * @brief Whether debug mode is enabled.
     */
    bool debug{false};

    /**
     * @brief Whether script source maps are enabled.
     */
    bool source_maps{false};

    /**
     * @brief Maximum stack size requested for the engine.
     *
     * Zero means engine default.
     */
    std::size_t max_stack_size{0};

    /**
     * @brief Maximum heap size requested for the engine.
     *
     * Zero means engine default.
     */
    std::size_t max_heap_size{0};

    /**
     * @brief Optional module search paths.
     */
    std::vector<std::string> module_paths{};

    /**
     * @brief Optional engine name override.
     */
    std::string engine_name{};

    /**
     * @brief Return default binding options.
     */
    [[nodiscard]] static BindingOptions defaults();

    /**
     * @brief Return development binding options.
     */
    [[nodiscard]] static BindingOptions development();

    /**
     * @brief Return production binding options.
     */
    [[nodiscard]] static BindingOptions production();

    /**
     * @brief Return test binding options.
     */
    [[nodiscard]] static BindingOptions test();

    /**
     * @brief Return true if a stack limit is configured.
     */
    [[nodiscard]] bool has_stack_limit() const noexcept;

    /**
     * @brief Return true if a heap limit is configured.
     */
    [[nodiscard]] bool has_heap_limit() const noexcept;

    /**
     * @brief Return true if custom module paths are configured.
     */
    [[nodiscard]] bool has_module_paths() const noexcept;

    /**
     * @brief Return true if an engine name override is configured.
     */
    [[nodiscard]] bool has_engine_name() const noexcept;
  };

  /**
   * @brief Convert EngineBackend to a stable string.
   */
  [[nodiscard]] const char *to_string(EngineBackend backend) noexcept;

  /**
   * @brief Convert ModulePolicy to a stable string.
   */
  [[nodiscard]] const char *to_string(ModulePolicy policy) noexcept;

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_BINDING_OPTIONS_HPP
