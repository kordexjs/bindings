/**
 *
 *  @file ValueType.hpp
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

#ifndef KORDEX_BINDINGS_VALUE_TYPE_HPP
#define KORDEX_BINDINGS_VALUE_TYPE_HPP

namespace kordex::bindings
{
  /**
   * @enum ValueType
   * @brief Type of value handled by the bindings layer.
   */
  enum class ValueType
  {
    Undefined,
    Null,
    Boolean,
    Number,
    String,
    Object,
    Function,
    NativeFunction,
    Module
  };

  /**
   * @brief Convert ValueType to a stable string.
   */
  [[nodiscard]] constexpr const char *to_string(ValueType type) noexcept
  {
    switch (type)
    {
    case ValueType::Undefined:
      return "undefined";
    case ValueType::Null:
      return "null";
    case ValueType::Boolean:
      return "boolean";
    case ValueType::Number:
      return "number";
    case ValueType::String:
      return "string";
    case ValueType::Object:
      return "object";
    case ValueType::Function:
      return "function";
    case ValueType::NativeFunction:
      return "native_function";
    case ValueType::Module:
      return "module";
    }

    return "undefined";
  }

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_VALUE_TYPE_HPP
