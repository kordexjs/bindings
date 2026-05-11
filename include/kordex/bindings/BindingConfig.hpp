/**
 *
 *  @file BindingConfig.hpp
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

#ifndef KORDEX_BINDINGS_BINDING_CONFIG_HPP
#define KORDEX_BINDINGS_BINDING_CONFIG_HPP

#include <cstddef>
#include <string>
#include <vector>

#include <kordex/bindings/BindingOptions.hpp>
#include <kordex/bindings/Error.hpp>
#include <kordex/bindings/Result.hpp>

namespace kordex::bindings
{
  /**
   * @struct BindingConfig
   * @brief Normalized bindings configuration.
   *
   * BindingOptions represents caller-provided values.
   * BindingConfig represents the final validated configuration used by
   * Engine, EngineContext, ModuleRegistry, and RuntimeBridge.
   */
  struct BindingConfig
  {
    EngineBackend backend{EngineBackend::Native};
    ModulePolicy module_policy{ModulePolicy::Full};

    bool allow_native_modules{true};
    bool allow_native_functions{true};
    bool allow_runtime_bridge{true};

    bool diagnostics{true};
    bool debug{false};
    bool source_maps{false};

    std::size_t max_stack_size{0};
    std::size_t max_heap_size{0};

    std::vector<std::string> module_paths{};

    std::string engine_name{"native"};
    std::string environment{"development"};

    /**
     * @brief Build a BindingConfig from explicit BindingOptions.
     */
    [[nodiscard]] static Result<BindingConfig> from_options(
        const BindingOptions &options);

    /**
     * @brief Build a BindingConfig from BindingOptions and KORDEX_* env vars.
     */
    [[nodiscard]] static Result<BindingConfig> from_environment(
        const BindingOptions &base = BindingOptions::defaults());

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
     * @brief Return true if an engine name is configured.
     */
    [[nodiscard]] bool has_engine_name() const noexcept;

    /**
     * @brief Validate the final bindings configuration.
     */
    [[nodiscard]] Error validate() const;
  };

  /**
   * @brief Parse an engine backend string.
   */
  [[nodiscard]] Result<EngineBackend> parse_engine_backend(
      const std::string &value);

  /**
   * @brief Parse a module policy string.
   */
  [[nodiscard]] Result<ModulePolicy> parse_module_policy(
      const std::string &value);

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_BINDING_CONFIG_HPP
