/**
 *
 *  @file Value.cpp
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

#include <sstream>
#include <utility>

#include <kordex/bindings/Value.hpp>

namespace kordex::bindings
{
  namespace
  {
    [[nodiscard]] Error make_type_error(
        ValueType actual,
        ValueType expected)
    {
      return make_binding_error(
          BindingErrorCode::ValueConversionFailed,
          std::string("cannot convert value of type ") +
              to_string(actual) +
              " to " +
              to_string(expected));
    }

    [[nodiscard]] std::string number_to_string(double value)
    {
      std::ostringstream stream;
      stream << value;
      return stream.str();
    }
  } // namespace

  Value::Value()
      : storage_(std::monostate{})
  {
  }

  Value::Value(std::nullptr_t)
      : storage_(nullptr)
  {
  }

  Value::Value(bool value)
      : storage_(value)
  {
  }

  Value::Value(double value)
      : storage_(value)
  {
  }

  Value::Value(std::string value)
      : storage_(std::move(value))
  {
  }

  Value::Value(const char *value)
      : storage_(std::string(value ? value : ""))
  {
  }

  Value Value::undefined()
  {
    return Value{};
  }

  Value Value::null()
  {
    return Value(nullptr);
  }

  Value Value::boolean(bool value)
  {
    return Value(value);
  }

  Value Value::number(double value)
  {
    return Value(value);
  }

  Value Value::string(std::string value)
  {
    return Value(std::move(value));
  }

  ValueType Value::type() const noexcept
  {
    if (std::holds_alternative<std::monostate>(storage_))
    {
      return ValueType::Undefined;
    }

    if (std::holds_alternative<std::nullptr_t>(storage_))
    {
      return ValueType::Null;
    }

    if (std::holds_alternative<bool>(storage_))
    {
      return ValueType::Boolean;
    }

    if (std::holds_alternative<double>(storage_))
    {
      return ValueType::Number;
    }

    if (std::holds_alternative<std::string>(storage_))
    {
      return ValueType::String;
    }

    return ValueType::Undefined;
  }

  bool Value::is_undefined() const noexcept
  {
    return type() == ValueType::Undefined;
  }

  bool Value::is_null() const noexcept
  {
    return type() == ValueType::Null;
  }

  bool Value::is_boolean() const noexcept
  {
    return type() == ValueType::Boolean;
  }

  bool Value::is_number() const noexcept
  {
    return type() == ValueType::Number;
  }

  bool Value::is_string() const noexcept
  {
    return type() == ValueType::String;
  }

  bool Value::truthy() const noexcept
  {
    if (is_undefined() || is_null())
    {
      return false;
    }

    if (is_boolean())
    {
      return std::get<bool>(storage_);
    }

    if (is_number())
    {
      return std::get<double>(storage_) != 0.0;
    }

    if (is_string())
    {
      return !std::get<std::string>(storage_).empty();
    }

    return true;
  }

  Result<bool> Value::as_boolean() const
  {
    if (!is_boolean())
    {
      return make_type_error(type(), ValueType::Boolean);
    }

    return std::get<bool>(storage_);
  }

  Result<double> Value::as_number() const
  {
    if (!is_number())
    {
      return make_type_error(type(), ValueType::Number);
    }

    return std::get<double>(storage_);
  }

  Result<std::string> Value::as_string() const
  {
    if (!is_string())
    {
      return make_type_error(type(), ValueType::String);
    }

    return std::get<std::string>(storage_);
  }

  std::string Value::display() const
  {
    switch (type())
    {
    case ValueType::Undefined:
      return "undefined";

    case ValueType::Null:
      return "null";

    case ValueType::Boolean:
      return std::get<bool>(storage_) ? "true" : "false";

    case ValueType::Number:
      return number_to_string(std::get<double>(storage_));

    case ValueType::String:
      return std::get<std::string>(storage_);

    case ValueType::Object:
    case ValueType::Function:
    case ValueType::NativeFunction:
    case ValueType::Module:
      break;
    }

    return "undefined";
  }

  const Value::Storage &Value::storage() const noexcept
  {
    return storage_;
  }

  bool Value::equals(const Value &other) const noexcept
  {
    return storage_ == other.storage_;
  }

  bool operator==(const Value &lhs, const Value &rhs) noexcept
  {
    return lhs.equals(rhs);
  }

  bool operator!=(const Value &lhs, const Value &rhs) noexcept
  {
    return !lhs.equals(rhs);
  }

} // namespace kordex::bindings
