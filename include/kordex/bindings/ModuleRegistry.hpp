/**
 *
 *  @file ModuleRegistry.hpp
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

#ifndef KORDEX_BINDINGS_MODULE_REGISTRY_HPP
#define KORDEX_BINDINGS_MODULE_REGISTRY_HPP

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include <kordex/bindings/Error.hpp>
#include <kordex/bindings/Module.hpp>
#include <kordex/bindings/NativeModule.hpp>
#include <kordex/bindings/Result.hpp>

namespace kordex::bindings
{
  /**
   * @struct ModuleRegistryOptions
   * @brief Options controlling module registration and lookup.
   */
  struct ModuleRegistryOptions
  {
    /**
     * @brief Whether native modules can be registered.
     */
    bool allow_native_modules{true};

    /**
     * @brief Whether script modules can be registered.
     */
    bool allow_script_modules{true};

    /**
     * @brief Whether builtin modules can be registered.
     */
    bool allow_builtin_modules{true};

    /**
     * @brief Whether an existing module can be replaced.
     */
    bool allow_overwrite{true};
  };

  /**
   * @class ModuleRegistry
   * @brief Registry for modules visible to the bindings layer.
   *
   * ModuleRegistry owns resolved and registered modules. It is used by
   * EngineContext, RuntimeBridge, and future JavaScript backends to expose
   * builtin modules, native modules, and script modules consistently.
   */
  class ModuleRegistry
  {
  public:
    /**
     * @brief Module storage type.
     */
    using ModuleMap = std::map<std::string, Module>;

    /**
     * @brief Construct a registry with default options.
     */
    ModuleRegistry();

    /**
     * @brief Construct a registry with explicit options.
     */
    explicit ModuleRegistry(ModuleRegistryOptions options);

    /**
     * @brief Return registry options.
     */
    [[nodiscard]] const ModuleRegistryOptions &options() const noexcept;

    /**
     * @brief Update registry options.
     */
    void set_options(ModuleRegistryOptions options);

    /**
     * @brief Register a module.
     */
    [[nodiscard]] Error register_module(Module module);

    /**
     * @brief Register a module with an explicit name.
     */
    [[nodiscard]] Error register_module(
        std::string name,
        Module module);

    /**
     * @brief Register a native module.
     */
    [[nodiscard]] Error register_native_module(
        const NativeModule &native_module);

    /**
     * @brief Return a registered module or an error.
     */
    [[nodiscard]] Result<Module> module(
        const std::string &name) const;

    /**
     * @brief Return true if a module exists.
     */
    [[nodiscard]] bool has_module(
        const std::string &name) const noexcept;

    /**
     * @brief Remove a registered module.
     */
    [[nodiscard]] Error remove_module(
        const std::string &name);

    /**
     * @brief Import a registered module.
     */
    [[nodiscard]] Result<Module> import_module(
        const std::string &name) const;

    /**
     * @brief Return all registered module names.
     */
    [[nodiscard]] std::vector<std::string> names() const;

    /**
     * @brief Return registered module names for a given kind.
     */
    [[nodiscard]] std::vector<std::string> names_by_kind(
        ModuleKind kind) const;

    /**
     * @brief Return the number of registered modules.
     */
    [[nodiscard]] std::size_t size() const noexcept;

    /**
     * @brief Return true if no modules are registered.
     */
    [[nodiscard]] bool empty() const noexcept;

    /**
     * @brief Clear all registered modules.
     */
    void clear() noexcept;

    /**
     * @brief Return the underlying module map.
     */
    [[nodiscard]] const ModuleMap &modules() const noexcept;

  private:
    [[nodiscard]] Error validate_name(
        const std::string &name) const;

    [[nodiscard]] Error validate_module_allowed(
        const Module &module) const;

    ModuleRegistryOptions options_{};
    ModuleMap modules_{};
  };

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_MODULE_REGISTRY_HPP
