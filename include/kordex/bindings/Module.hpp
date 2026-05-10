/**
 *
 *  @file Module.hpp
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

#ifndef KORDEX_BINDINGS_MODULE_HPP
#define KORDEX_BINDINGS_MODULE_HPP

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include <kordex/bindings/Error.hpp>
#include <kordex/bindings/Function.hpp>
#include <kordex/bindings/NativeModule.hpp>
#include <kordex/bindings/Object.hpp>
#include <kordex/bindings/Result.hpp>
#include <kordex/bindings/Value.hpp>

namespace kordex::bindings
{
  /**
   * @enum ModuleKind
   * @brief Type of bindings module.
   */
  enum class ModuleKind
  {
    Unknown,
    Native,
    Script,
    Builtin
  };

  /**
   * @struct ModuleInfo
   * @brief Metadata describing a bindings module.
   */
  struct ModuleInfo
  {
    /**
     * @brief Public module name.
     */
    std::string name{};

    /**
     * @brief Optional module path.
     */
    std::string path{};

    /**
     * @brief Module kind.
     */
    ModuleKind kind{ModuleKind::Unknown};

    /**
     * @brief Optional description.
     */
    std::string description{};

    /**
     * @brief Whether the module can be imported.
     */
    bool importable{true};

    /**
     * @brief Whether the module has been loaded.
     */
    bool loaded{false};

    /**
     * @brief Return true if the module has a name.
     */
    [[nodiscard]] bool has_name() const noexcept;

    /**
     * @brief Return true if the module has a path.
     */
    [[nodiscard]] bool has_path() const noexcept;

    /**
     * @brief Return true if the module has a description.
     */
    [[nodiscard]] bool has_description() const noexcept;
  };

  /**
   * @class Module
   * @brief Engine-independent module representation.
   *
   * Module stores exported values and functions. It represents a module after
   * it has been resolved by runtime and before it is attached to a concrete
   * JavaScript engine.
   */
  class Module
  {
  public:
    /**
     * @brief Exported value map type.
     */
    using ValueMap = std::map<std::string, Value>;

    /**
     * @brief Exported function map type.
     */
    using FunctionMap = std::map<std::string, Function>;

    /**
     * @brief Construct an empty invalid module.
     */
    Module() = default;

    /**
     * @brief Construct a module from metadata.
     */
    explicit Module(ModuleInfo info);

    /**
     * @brief Create a module from name and kind.
     */
    [[nodiscard]] static Module create(
        std::string name,
        ModuleKind kind = ModuleKind::Script);

    /**
     * @brief Create a module from metadata.
     */
    [[nodiscard]] static Module create(ModuleInfo info);

    /**
     * @brief Create a module from a native module.
     */
    [[nodiscard]] static Module from_native(
        const NativeModule &native_module);

    /**
     * @brief Return module metadata.
     */
    [[nodiscard]] const ModuleInfo &info() const noexcept;

    /**
     * @brief Return module name.
     */
    [[nodiscard]] const std::string &name() const noexcept;

    /**
     * @brief Return module path.
     */
    [[nodiscard]] const std::string &path() const noexcept;

    /**
     * @brief Return module kind.
     */
    [[nodiscard]] ModuleKind kind() const noexcept;

    /**
     * @brief Return true if the module has a valid name and kind.
     */
    [[nodiscard]] bool valid() const noexcept;

    /**
     * @brief Return true if the module can be imported.
     */
    [[nodiscard]] bool importable() const noexcept;

    /**
     * @brief Return true if the module is loaded.
     */
    [[nodiscard]] bool loaded() const noexcept;

    /**
     * @brief Mark the module as loaded.
     */
    void mark_loaded() noexcept;

    /**
     * @brief Mark the module as unloaded.
     */
    void mark_unloaded() noexcept;

    /**
     * @brief Export a value.
     */
    [[nodiscard]] Error set_export(
        std::string name,
        Value value);

    /**
     * @brief Return an exported value or an error.
     */
    [[nodiscard]] Result<Value> export_value(
        const std::string &name) const;

    /**
     * @brief Return true if an exported value exists.
     */
    [[nodiscard]] bool has_export(
        const std::string &name) const noexcept;

    /**
     * @brief Remove an exported value.
     */
    [[nodiscard]] Error remove_export(
        const std::string &name);

    /**
     * @brief Export a function.
     */
    [[nodiscard]] Error set_function(
        std::string name,
        Function function);

    /**
     * @brief Export a function using its own name.
     */
    [[nodiscard]] Error add_function(Function function);

    /**
     * @brief Return an exported function or an error.
     */
    [[nodiscard]] Result<Function> function(
        const std::string &name) const;

    /**
     * @brief Return true if an exported function exists.
     */
    [[nodiscard]] bool has_function(
        const std::string &name) const noexcept;

    /**
     * @brief Remove an exported function.
     */
    [[nodiscard]] Error remove_function(
        const std::string &name);

    /**
     * @brief Call an exported function.
     */
    [[nodiscard]] Result<Value> call(
        const std::string &name,
        const FunctionArguments &args = {}) const;

    /**
     * @brief Return exported value names.
     */
    [[nodiscard]] std::vector<std::string> export_names() const;

    /**
     * @brief Return exported function names.
     */
    [[nodiscard]] std::vector<std::string> function_names() const;

    /**
     * @brief Return the number of exported values.
     */
    [[nodiscard]] std::size_t export_count() const noexcept;

    /**
     * @brief Return the number of exported functions.
     */
    [[nodiscard]] std::size_t function_count() const noexcept;

    /**
     * @brief Return true if there are no exports or functions.
     */
    [[nodiscard]] bool empty() const noexcept;

    /**
     * @brief Clear exported values and functions.
     */
    void clear() noexcept;

    /**
     * @brief Return exported values.
     */
    [[nodiscard]] const ValueMap &exports() const noexcept;

    /**
     * @brief Return exported functions.
     */
    [[nodiscard]] const FunctionMap &functions() const noexcept;

    /**
     * @brief Convert module exports to an object.
     *
     * Function exports are represented as string placeholders because Value
     * does not yet own callable engine values.
     */
    [[nodiscard]] Object to_object() const;

  private:
    [[nodiscard]] Error validate_name(
        const std::string &name,
        const char *kind) const;

    ModuleInfo info_{};
    ValueMap exports_{};
    FunctionMap functions_{};
  };

  /**
   * @brief Convert ModuleKind to a stable string.
   */
  [[nodiscard]] const char *to_string(ModuleKind kind) noexcept;

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_MODULE_HPP
