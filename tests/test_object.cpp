/**
 *
 *  @file test_object.cpp
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

#include <string_view>
#include <kordex/bindings/Object.hpp>

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

  template <typename T>
  [[nodiscard]] bool result_ok(
      const kordex::bindings::Result<T> &result)
  {
    return !(!result);
  }

  [[nodiscard]] bool test_empty_object()
  {
    const auto object = kordex::bindings::Object::create();

    return expect_true(
               object.empty(),
               "new object should be empty") &&
           expect_true(
               object.size() == 0,
               "new object size should be zero") &&
           expect_true(
               !object.has_name(),
               "new object should not have name") &&
           expect_true(
               object.keys().empty(),
               "new object keys should be empty") &&
           expect_true(
               object.values().empty(),
               "new object values should be empty");
  }

  [[nodiscard]] bool test_named_object()
  {
    auto object = kordex::bindings::Object::create("global");

    const bool initial_name =
        object.has_name() &&
        std::string_view(object.name()) == "global";

    object.set_name("runtime");

    return expect_true(
               initial_name,
               "named object should expose initial name") &&
           expect_true(
               object.has_name(),
               "renamed object should have name") &&
           expect_true(
               std::string_view(object.name()) == "runtime",
               "renamed object name should match");
  }

  [[nodiscard]] bool test_set_and_get_property()
  {
    auto object = kordex::bindings::Object::create();

    const auto set_error = object.set(
        "name",
        kordex::bindings::Value::string("kordex"));

    auto value = object.get("name");

    if (!expect_true(
            !set_error,
            "set property should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(value),
            "get property should succeed"))
    {
      return false;
    }

    auto string_value = value.value().as_string();

    if (!expect_true(
            result_ok(string_value),
            "property value should convert to string"))
    {
      return false;
    }

    return expect_true(
               object.has("name"),
               "object should have property") &&
           expect_true(
               object.size() == 1,
               "object size should be one") &&
           expect_true(
               string_value.value() == "kordex",
               "property value should match");
  }

  [[nodiscard]] bool test_overwrite_property()
  {
    auto object = kordex::bindings::Object::create();

    const auto first_error = object.set(
        "version",
        kordex::bindings::Value::string("0.1.0"));

    const auto second_error = object.set(
        "version",
        kordex::bindings::Value::string("0.2.0"));

    auto value = object.get("version");

    if (!expect_true(
            !first_error && !second_error,
            "property overwrite should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(value),
            "get overwritten property should succeed"))
    {
      return false;
    }

    auto string_value = value.value().as_string();

    if (!expect_true(
            result_ok(string_value),
            "overwritten property should be string"))
    {
      return false;
    }

    return expect_true(
               object.size() == 1,
               "overwriting should keep one property") &&
           expect_true(
               string_value.value() == "0.2.0",
               "overwritten property should match");
  }

  [[nodiscard]] bool test_missing_property()
  {
    const auto object = kordex::bindings::Object::create();

    auto value = object.get("missing");

    return expect_true(
               !value,
               "missing property should fail") &&
           expect_true(
               value.error().code() ==
                   kordex::bindings::ErrorCode::InvalidArgument,
               "missing property should map to invalid argument");
  }

  [[nodiscard]] bool test_invalid_key()
  {
    auto object = kordex::bindings::Object::create();

    const auto set_error = object.set(
        "",
        kordex::bindings::Value::number(1.0));

    auto get_result = object.get("");
    const auto remove_error = object.remove("");

    return expect_true(
               set_error.has_error(),
               "set with empty key should fail") &&
           expect_true(
               !get_result,
               "get with empty key should fail") &&
           expect_true(
               remove_error.has_error(),
               "remove with empty key should fail") &&
           expect_true(
               set_error.code() ==
                   kordex::bindings::ErrorCode::InvalidArgument,
               "empty key should map to invalid argument");
  }

  [[nodiscard]] bool test_remove_property()
  {
    auto object = kordex::bindings::Object::create();

    const auto set_error = object.set(
        "enabled",
        kordex::bindings::Value::boolean(true));

    const auto remove_error = object.remove("enabled");
    auto removed_value = object.get("enabled");

    return expect_true(
               !set_error,
               "set before remove should succeed") &&
           expect_true(
               !remove_error,
               "remove should succeed") &&
           expect_true(
               !object.has("enabled"),
               "removed property should no longer exist") &&
           expect_true(
               object.empty(),
               "object should be empty after remove") &&
           expect_true(
               !removed_value,
               "removed value should not be found");
  }

  [[nodiscard]] bool test_remove_missing_property()
  {
    auto object = kordex::bindings::Object::create();

    const auto error = object.remove("missing");

    return expect_true(
               error.has_error(),
               "removing missing property should fail") &&
           expect_true(
               error.code() ==
                   kordex::bindings::ErrorCode::InvalidArgument,
               "removing missing property should map to invalid argument");
  }

  [[nodiscard]] bool test_keys_and_values()
  {
    auto object = kordex::bindings::Object::create();

    const auto first_error = object.set(
        "a",
        kordex::bindings::Value::number(1.0));

    const auto second_error = object.set(
        "b",
        kordex::bindings::Value::string("two"));

    const auto keys = object.keys();
    const auto values = object.values();

    return expect_true(
               !first_error && !second_error,
               "setting properties should succeed") &&
           expect_true(
               keys.size() == 2,
               "keys size should be two") &&
           expect_true(
               values.size() == 2,
               "values size should be two") &&
           expect_true(
               keys[0] == "a" && keys[1] == "b",
               "keys should be sorted by map order") &&
           expect_true(
               values[0].is_number() && values[1].is_string(),
               "values should preserve property values");
  }

  [[nodiscard]] bool test_clear_object()
  {
    auto object = kordex::bindings::Object::create();

    const auto first_error = object.set(
        "a",
        kordex::bindings::Value::number(1.0));

    const auto second_error = object.set(
        "b",
        kordex::bindings::Value::number(2.0));

    object.clear();

    return expect_true(
               !first_error && !second_error,
               "setting before clear should succeed") &&
           expect_true(
               object.empty(),
               "object should be empty after clear") &&
           expect_true(
               object.size() == 0,
               "object size should be zero after clear") &&
           expect_true(
               object.keys().empty(),
               "object keys should be empty after clear");
  }

  [[nodiscard]] bool test_object_equality()
  {
    auto left = kordex::bindings::Object::create("module");
    auto right = kordex::bindings::Object::create("module");
    auto other = kordex::bindings::Object::create("other");

    const auto left_error = left.set(
        "name",
        kordex::bindings::Value::string("kordex"));

    const auto right_error = right.set(
        "name",
        kordex::bindings::Value::string("kordex"));

    const auto other_error = other.set(
        "name",
        kordex::bindings::Value::string("kordex"));

    return expect_true(
               !left_error && !right_error && !other_error,
               "setting object properties should succeed") &&
           expect_true(
               left == right,
               "objects with same name and properties should be equal") &&
           expect_true(
               left != other,
               "objects with different name should not be equal") &&
           expect_true(
               left.equals(right),
               "equals should report equal objects");
  }

} // namespace

int main()
{
  const bool ok =
      test_empty_object() &&
      test_named_object() &&
      test_set_and_get_property() &&
      test_overwrite_property() &&
      test_missing_property() &&
      test_invalid_key() &&
      test_remove_property() &&
      test_remove_missing_property() &&
      test_keys_and_values() &&
      test_clear_object() &&
      test_object_equality();

  return ok ? 0 : 1;
}
