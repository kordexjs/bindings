/**
 *
 *  @file Function.hpp
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

#ifndef KORDEX_BINDINGS_FUNCTION_HPP
#define KORDEX_BINDINGS_FUNCTION_HPP

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include <kordex/bindings/Result.hpp>
#include <kordex/bindings/Value.hpp>

namespace kordex::bindings
{
  /**
   * @brief Function argument list.
   */
  using FunctionArguments = std::vector<Value>;

  /**
   * @brief Function callback signature.
   */
  using FunctionCallback = std::function<Result<Value>(const FunctionArguments &)>;

  /**
   * @struct FunctionInfo
   * @brief Metadata describing a bindings function.
   */
  struct FunctionInfo
  {
    /**
     * @brief Function name.
     */
    std::string name{};

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
     * @brief Return true if the function has a name.
     */
    [[nodiscard]] bool has_name() const noexcept;

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
   * @class Function
   * @brief Engine-independent callable function wrapper.
   *
   * Function stores a callback and simple metadata. It is used by the
   * bindings layer before a concrete JavaScript backend is attached.
   */
  class Function
  {
  public:
    /**
     * @brief Construct an empty invalid function.
     */
    Function() = default;

    /**
     * @brief Construct a function from metadata and callback.
     */
    Function(FunctionInfo info, FunctionCallback callback);

    /**
     * @brief Create a function from name and callback.
     */
    [[nodiscard]] static Function create(
        std::string name,
        FunctionCallback callback);

    /**
     * @brief Create a function from metadata and callback.
     */
    [[nodiscard]] static Function create(
        FunctionInfo info,
        FunctionCallback callback);

    /**
     * @brief Return true if the function has a callable callback.
     */
    [[nodiscard]] bool valid() const noexcept;

    /**
     * @brief Return true if the function can be called.
     */
    [[nodiscard]] bool callable() const noexcept;

    /**
     * @brief Return function metadata.
     */
    [[nodiscard]] const FunctionInfo &info() const noexcept;

    /**
     * @brief Return function name.
     */
    [[nodiscard]] const std::string &name() const noexcept;

    /**
     * @brief Call the function with arguments.
     */
    [[nodiscard]] Result<Value> call(
        const FunctionArguments &args = {}) const;

    /**
     * @brief Call the function with no arguments.
     */
    [[nodiscard]] Result<Value> operator()() const;

    /**
     * @brief Call the function with arguments.
     */
    [[nodiscard]] Result<Value> operator()(
        const FunctionArguments &args) const;

  private:
    [[nodiscard]] Error validate_call(
        const FunctionArguments &args) const;

    FunctionInfo info_{};
    FunctionCallback callback_{};
  };

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_FUNCTION_HPP
