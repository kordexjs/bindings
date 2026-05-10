/**
 *
 *  @file NativeModule.hpp
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

#ifndef KORDEX_BINDINGS_NATIVE_MODULE_HPP
#define KORDEX_BINDINGS_NATIVE_MODULE_HPP

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include <kordex/bindings/Error.hpp>
#include <kordex/bindings/NativeFunction.hpp>
#include <kordex/bindings/Object.hpp>
#include <kordex/bindings/Result.hpp>
#include <kordex/bindings/Value.hpp>

namespace kordex::bindings
{
  /**
   * @struct NativeModuleInfo
   * @brief Metadata describing a native module.
   */
  struct NativeModuleInfo
  {
    /**
     * @brief Public module name.
     *
     * Examples:
     * - fs
     * - process
     * - timer
     * - console
     */
    std::string name{};

    /**
     * @brief Optional module namespace.
     *
     * Example:
     * - kordex
     * - kordex.runtime
     */
    std::string namespace_name{};

    /**
     * @brief Optional module description.
     */
    std::string description{};

    /**
     * @brief Whether the module can be imported by scripts.
     */
    bool importable{true};

    /**
     * @brief Whether the module is considered safe.
     */
    bool safe{true};

    /**
     * @brief Return true if the module has a name.
     */
    [[nodiscard]] bool has_name() const noexcept;

    /**
     * @brief Return true if the module has a namespace.
     */
    [[nodiscard]] bool has_namespace_name() const noexcept;

    /**
     * @brief Return true if the module has a description.
     */
    [[nodiscard]] bool has_description() const noexcept;
  };

  /**
   * @class NativeModule
   * @brief C++ module exposed through the bindings layer.
   *
   * NativeModule owns native functions and constant values. It can be
   * converted to a lightweight Object representation for engine contexts.
   */
  class NativeModule
  {
  public:
    /**
     * @brief Native function map type.
     */
    using FunctionMap = std::map<std::string, NativeFunction>;

    /**
     * @brief Constant value map type.
     */
    using ValueMap = std::map<std::string, Value>;

    /**
     * @brief Construct an empty invalid module.
     */
    NativeModule() = default;

    /**
     * @brief Construct a native module from metadata.
     */
    explicit NativeModule(NativeModuleInfo info);

    /**
     * @brief Create a native module from name.
     */
    [[nodiscard]] static NativeModule create(std::string name);

    /**
     * @brief Create a native module from metadata.
     */
    [[nodiscard]] static NativeModule create(NativeModuleInfo info);

    /**
     * @brief Return module metadata.
     */
    [[nodiscard]] const NativeModuleInfo &info() const noexcept;

    /**
     * @brief Return module name.
     */
    [[nodiscard]] const std::string &name() const noexcept;

    /**
     * @brief Return module namespace.
     */
    [[nodiscard]] const std::string &namespace_name() const noexcept;

    /**
     * @brief Return true if module has a valid name.
     */
    [[nodiscard]] bool valid() const noexcept;

    /**
     * @brief Return true if module can be imported.
     */
    [[nodiscard]] bool importable() const noexcept;

    /**
     * @brief Return true if module is marked safe.
     */
    [[nodiscard]] bool safe() const noexcept;

    /**
     * @brief Register a native function.
     */
    [[nodiscard]] Error add_function(NativeFunction function);

    /**
     * @brief Register a native function with explicit name.
     */
    [[nodiscard]] Error add_function(
        std::string name,
        NativeFunction function);

    /**
     * @brief Return a native function or an error.
     */
    [[nodiscard]] Result<NativeFunction> function(
        const std::string &name) const;

    /**
     * @brief Return true if a function exists.
     */
    [[nodiscard]] bool has_function(
        const std::string &name) const noexcept;

    /**
     * @brief Remove a native function.
     */
    [[nodiscard]] Error remove_function(
        const std::string &name);

    /**
     * @brief Set a constant value.
     */
    [[nodiscard]] Error set_value(
        std::string name,
        Value value);

    /**
     * @brief Return a constant value or an error.
     */
    [[nodiscard]] Result<Value> value(
        const std::string &name) const;

    /**
     * @brief Return true if a constant value exists.
     */
    [[nodiscard]] bool has_value(
        const std::string &name) const noexcept;

    /**
     * @brief Remove a constant value.
     */
    [[nodiscard]] Error remove_value(
        const std::string &name);

    /**
     * @brief Call a native function by name.
     */
    [[nodiscard]] Result<Value> call(
        const std::string &name,
        const NativeFunctionArguments &args = {}) const;

    /**
     * @brief Return all function names.
     */
    [[nodiscard]] std::vector<std::string> function_names() const;

    /**
     * @brief Return all value names.
     */
    [[nodiscard]] std::vector<std::string> value_names() const;

    /**
     * @brief Return the number of registered functions.
     */
    [[nodiscard]] std::size_t function_count() const noexcept;

    /**
     * @brief Return the number of registered values.
     */
    [[nodiscard]] std::size_t value_count() const noexcept;

    /**
     * @brief Return true if module has no functions and no values.
     */
    [[nodiscard]] bool empty() const noexcept;

    /**
     * @brief Clear functions and values.
     */
    void clear() noexcept;

    /**
     * @brief Return registered functions.
     */
    [[nodiscard]] const FunctionMap &functions() const noexcept;

    /**
     * @brief Return registered values.
     */
    [[nodiscard]] const ValueMap &values() const noexcept;

    /**
     * @brief Convert this module to a lightweight Object.
     *
     * Constant values are copied as properties. Native functions are exposed
     * as string placeholders for now because Value does not yet own callable
     * engine values.
     */
    [[nodiscard]] Object to_object() const;

  private:
    [[nodiscard]] Error validate_name(
        const std::string &name,
        const char *kind) const;

    NativeModuleInfo info_{};
    FunctionMap functions_{};
    ValueMap values_{};
  };

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_NATIVE_MODULE_HPP
