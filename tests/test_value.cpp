/**
 *
 *  @file test_value.cpp
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

#include <string_view>
#include <kordex/bindings/Value.hpp>

namespace
{
  [[nodiscard]] bool expect_true(
      bool condition,
      const char *message)
  {
    if (!condition)
    {
      (void)message;
      return false;
    }

    return true;
  }

  [[nodiscard]] bool test_value_type_strings()
  {
    return expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::ValueType::Undefined)) == "undefined",
               "undefined string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::ValueType::Null)) == "null",
               "null string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::ValueType::Boolean)) == "boolean",
               "boolean string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::ValueType::Number)) == "number",
               "number string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::ValueType::String)) == "string",
               "string string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::ValueType::NativeFunction)) ==
                   "native_function",
               "native function string should match");
  }

  [[nodiscard]] bool test_undefined_value()
  {
    const auto value = kordex::bindings::Value::undefined();

    return expect_true(
               value.type() == kordex::bindings::ValueType::Undefined,
               "undefined value should have undefined type") &&
           expect_true(
               value.is_undefined(),
               "undefined value should report undefined") &&
           expect_true(
               !value.truthy(),
               "undefined value should be falsy") &&
           expect_true(
               std::string_view(value.display()) == "undefined",
               "undefined display should match");
  }

  [[nodiscard]] bool test_null_value()
  {
    const auto value = kordex::bindings::Value::null();

    return expect_true(
               value.type() == kordex::bindings::ValueType::Null,
               "null value should have null type") &&
           expect_true(
               value.is_null(),
               "null value should report null") &&
           expect_true(
               !value.truthy(),
               "null value should be falsy") &&
           expect_true(
               std::string_view(value.display()) == "null",
               "null display should match");
  }

  [[nodiscard]] bool test_boolean_value()
  {
    const auto true_value = kordex::bindings::Value::boolean(true);
    const auto false_value = kordex::bindings::Value::boolean(false);

    auto extracted_true = true_value.as_boolean();
    auto extracted_false = false_value.as_boolean();

    return expect_true(
               true_value.is_boolean(),
               "true value should be boolean") &&
           expect_true(
               false_value.is_boolean(),
               "false value should be boolean") &&
           expect_true(
               true_value.truthy(),
               "true value should be truthy") &&
           expect_true(
               !false_value.truthy(),
               "false value should be falsy") &&
           expect_true(
               extracted_true && extracted_true.value(),
               "true value should extract true") &&
           expect_true(
               extracted_false && !extracted_false.value(),
               "false value should extract false") &&
           expect_true(
               std::string_view(true_value.display()) == "true",
               "true display should match") &&
           expect_true(
               std::string_view(false_value.display()) == "false",
               "false display should match");
  }

  [[nodiscard]] bool test_number_value()
  {
    const auto value = kordex::bindings::Value::number(42.5);
    const auto zero = kordex::bindings::Value::number(0.0);

    auto extracted = value.as_number();

    return expect_true(
               value.is_number(),
               "number value should be number") &&
           expect_true(
               value.truthy(),
               "non-zero number should be truthy") &&
           expect_true(
               !zero.truthy(),
               "zero number should be falsy") &&
           expect_true(
               extracted && extracted.value() == 42.5,
               "number value should extract 42.5") &&
           expect_true(
               std::string_view(value.display()) == "42.5",
               "number display should match");
  }

  [[nodiscard]] bool test_string_value()
  {
    const auto value = kordex::bindings::Value::string("kordex");
    const auto empty = kordex::bindings::Value::string("");

    auto extracted = value.as_string();

    return expect_true(
               value.is_string(),
               "string value should be string") &&
           expect_true(
               value.truthy(),
               "non-empty string should be truthy") &&
           expect_true(
               !empty.truthy(),
               "empty string should be falsy") &&
           expect_true(
               extracted &&
                   std::string_view(extracted.value()) == "kordex",
               "string value should extract kordex") &&
           expect_true(
               std::string_view(value.display()) == "kordex",
               "string display should match");
  }

  [[nodiscard]] bool test_invalid_conversions()
  {
    const auto value = kordex::bindings::Value::string("not a number");

    auto as_number = value.as_number();
    auto as_boolean = value.as_boolean();

    return expect_true(
               !as_number,
               "string to number conversion should fail") &&
           expect_true(
               !as_boolean,
               "string to boolean conversion should fail") &&
           expect_true(
               as_number.error().code() ==
                   kordex::bindings::ErrorCode::InvalidArgument,
               "conversion error should map to invalid argument");
  }

  [[nodiscard]] bool test_value_equality()
  {
    const auto left = kordex::bindings::Value::string("same");
    const auto right = kordex::bindings::Value::string("same");
    const auto other = kordex::bindings::Value::string("other");

    return expect_true(
               left == right,
               "equal values should compare equal") &&
           expect_true(
               left != other,
               "different values should compare different") &&
           expect_true(
               left.equals(right),
               "equals should report equal values");
  }

  [[nodiscard]] bool test_constructors()
  {
    const kordex::bindings::Value undefined;
    const kordex::bindings::Value null_value(nullptr);
    const kordex::bindings::Value bool_value(true);
    const kordex::bindings::Value number_value(10.0);
    const kordex::bindings::Value string_value("hello");

    return expect_true(
               undefined.is_undefined(),
               "default constructor should create undefined") &&
           expect_true(
               null_value.is_null(),
               "nullptr constructor should create null") &&
           expect_true(
               bool_value.is_boolean(),
               "bool constructor should create boolean") &&
           expect_true(
               number_value.is_number(),
               "double constructor should create number") &&
           expect_true(
               string_value.is_string(),
               "const char pointer constructor should create string");
  }
} // namespace

int main()
{
  const bool ok =
      test_value_type_strings() &&
      test_undefined_value() &&
      test_null_value() &&
      test_boolean_value() &&
      test_number_value() &&
      test_string_value() &&
      test_invalid_conversions() &&
      test_value_equality() &&
      test_constructors();

  return ok ? 0 : 1;
}
