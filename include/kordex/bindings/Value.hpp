/**
 *
 *  @file Value.hpp
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

#ifndef KORDEX_BINDINGS_VALUE_HPP
#define KORDEX_BINDINGS_VALUE_HPP

#include <cstddef>
#include <string>
#include <string_view>
#include <variant>

#include <kordex/bindings/Error.hpp>
#include <kordex/bindings/Result.hpp>
#include <kordex/bindings/ValueType.hpp>

namespace kordex::bindings
{
  /**
   * @class Value
   * @brief Generic value transported through the bindings layer.
   *
   * Value is intentionally small and engine-independent. It represents the
   * primitive values that can safely cross the C++ bindings boundary before
   * a concrete JavaScript backend is attached.
   */
  class Value
  {
  public:
    /**
     * @brief Native value storage.
     */
    using Storage = std::variant<std::monostate, std::nullptr_t, bool, double, std::string>;

    /**
     * @brief Construct an undefined value.
     */
    Value();

    /**
     * @brief Construct a null value.
     */
    Value(std::nullptr_t);

    /**
     * @brief Construct a boolean value.
     */
    Value(bool value);

    /**
     * @brief Construct a number value.
     */
    Value(double value);

    /**
     * @brief Construct a string value.
     */
    Value(std::string value);

    /**
     * @brief Construct a string value.
     */
    Value(const char *value);

    /**
     * @brief Create an undefined value.
     */
    [[nodiscard]] static Value undefined();

    /**
     * @brief Create a null value.
     */
    [[nodiscard]] static Value null();

    /**
     * @brief Create a boolean value.
     */
    [[nodiscard]] static Value boolean(bool value);

    /**
     * @brief Create a number value.
     */
    [[nodiscard]] static Value number(double value);

    /**
     * @brief Create a string value.
     */
    [[nodiscard]] static Value string(std::string value);

    /**
     * @brief Return the value type.
     */
    [[nodiscard]] ValueType type() const noexcept;

    /**
     * @brief Return true if the value is undefined.
     */
    [[nodiscard]] bool is_undefined() const noexcept;

    /**
     * @brief Return true if the value is null.
     */
    [[nodiscard]] bool is_null() const noexcept;

    /**
     * @brief Return true if the value is a boolean.
     */
    [[nodiscard]] bool is_boolean() const noexcept;

    /**
     * @brief Return true if the value is a number.
     */
    [[nodiscard]] bool is_number() const noexcept;

    /**
     * @brief Return true if the value is a string.
     */
    [[nodiscard]] bool is_string() const noexcept;

    /**
     * @brief Return true if the value can be considered truthy.
     */
    [[nodiscard]] bool truthy() const noexcept;

    /**
     * @brief Return the boolean value or an error.
     */
    [[nodiscard]] Result<bool> as_boolean() const;

    /**
     * @brief Return the number value or an error.
     */
    [[nodiscard]] Result<double> as_number() const;

    /**
     * @brief Return the string value or an error.
     */
    [[nodiscard]] Result<std::string> as_string() const;

    /**
     * @brief Convert the value to a display string.
     */
    [[nodiscard]] std::string display() const;

    /**
     * @brief Return the underlying storage.
     */
    [[nodiscard]] const Storage &storage() const noexcept;

    /**
     * @brief Return true if two values are equal.
     */
    [[nodiscard]] bool equals(const Value &other) const noexcept;

  private:
    Storage storage_{};
  };

  /**
   * @brief Compare two values.
   */
  [[nodiscard]] bool operator==(const Value &lhs, const Value &rhs) noexcept;

  /**
   * @brief Compare two values.
   */
  [[nodiscard]] bool operator!=(const Value &lhs, const Value &rhs) noexcept;

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_VALUE_HPP
