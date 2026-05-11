/**
 *
 *  @file NativeFunction.hpp
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

#ifndef KORDEX_BINDINGS_NATIVE_FUNCTION_HPP
#define KORDEX_BINDINGS_NATIVE_FUNCTION_HPP

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include <kordex/bindings/Function.hpp>
#include <kordex/bindings/Result.hpp>
#include <kordex/bindings/Value.hpp>

namespace kordex::bindings
{
  /**
   * @brief Native function argument list.
   */
  using NativeFunctionArguments = std::vector<Value>;

  /**
   * @brief Native function callback signature.
   */
  using NativeFunctionCallback =
      std::function<Result<Value>(const NativeFunctionArguments &)>;

  /**
   * @struct NativeFunctionInfo
   * @brief Metadata describing a native function exposed to scripts.
   */
  struct NativeFunctionInfo
  {
    /**
     * @brief Public function name.
     */
    std::string name{};

    /**
     * @brief Optional module name that owns this function.
     */
    std::string module_name{};

    /**
     * @brief Optional function description.
     */
    std::string description{};

    /**
     * @brief Minimum accepted argument count.
     */
    std::size_t min_args{0};

    /**
     * @brief Maximum accepted argument count.
     *
     * Zero means no explicit maximum.
     */
    std::size_t max_args{0};

    /**
     * @brief Whether the function is callable from scripts.
     */
    bool callable{true};

    /**
     * @brief Whether the function is considered safe.
     */
    bool safe{true};

    /**
     * @brief Return true if the function has a name.
     */
    [[nodiscard]] bool has_name() const noexcept;

    /**
     * @brief Return true if the function belongs to a module.
     */
    [[nodiscard]] bool has_module_name() const noexcept;

    /**
     * @brief Return true if the function has a description.
     */
    [[nodiscard]] bool has_description() const noexcept;

    /**
     * @brief Return true if a maximum argument count is configured.
     */
    [[nodiscard]] bool has_max_args() const noexcept;

    /**
     * @brief Return true if the provided argument count is accepted.
     */
    [[nodiscard]] bool accepts(std::size_t count) const noexcept;
  };

  /**
   * @class NativeFunction
   * @brief C++ function exposed through the bindings layer.
   *
   * NativeFunction wraps a C++ callback with metadata and validation rules.
   * It can also be converted into the generic Function abstraction used by
   * objects, modules, and engine contexts.
   */
  class NativeFunction
  {
  public:
    /**
     * @brief Construct an empty invalid native function.
     */
    NativeFunction() = default;

    /**
     * @brief Construct a native function from metadata and callback.
     */
    NativeFunction(
        NativeFunctionInfo info,
        NativeFunctionCallback callback);

    /**
     * @brief Create a native function from name and callback.
     */
    [[nodiscard]] static NativeFunction create(
        std::string name,
        NativeFunctionCallback callback);

    /**
     * @brief Create a native function from metadata and callback.
     */
    [[nodiscard]] static NativeFunction create(
        NativeFunctionInfo info,
        NativeFunctionCallback callback);

    /**
     * @brief Return true if this native function has a callback.
     */
    [[nodiscard]] bool valid() const noexcept;

    /**
     * @brief Return true if this native function is callable.
     */
    [[nodiscard]] bool callable() const noexcept;

    /**
     * @brief Return true if this native function is marked safe.
     */
    [[nodiscard]] bool safe() const noexcept;

    /**
     * @brief Return native function metadata.
     */
    [[nodiscard]] const NativeFunctionInfo &info() const noexcept;

    /**
     * @brief Return native function name.
     */
    [[nodiscard]] const std::string &name() const noexcept;

    /**
     * @brief Return owning module name.
     */
    [[nodiscard]] const std::string &module_name() const noexcept;

    /**
     * @brief Call the native function.
     */
    [[nodiscard]] Result<Value> call(
        const NativeFunctionArguments &args = {}) const;

    /**
     * @brief Call the native function with no arguments.
     */
    [[nodiscard]] Result<Value> operator()() const;

    /**
     * @brief Call the native function with arguments.
     */
    [[nodiscard]] Result<Value> operator()(
        const NativeFunctionArguments &args) const;

    /**
     * @brief Convert to the generic Function abstraction.
     */
    [[nodiscard]] Function to_function() const;

  private:
    [[nodiscard]] Error validate_call(
        const NativeFunctionArguments &args) const;

    NativeFunctionInfo info_{};
    NativeFunctionCallback callback_{};
  };

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_NATIVE_FUNCTION_HPP
