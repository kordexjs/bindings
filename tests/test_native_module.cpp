/**
 *
 *  @file test_native_module.cpp
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
#include <kordex/bindings/NativeModule.hpp>

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

  [[nodiscard]] bool test_native_module_info_helpers()
  {
    kordex::bindings::NativeModuleInfo info;
    info.name = "fs";
    info.namespace_name = "kordex";
    info.description = "Filesystem module";

    return expect_true(
               info.has_name(),
               "module info should have name") &&
           expect_true(
               info.has_namespace_name(),
               "module info should have namespace") &&
           expect_true(
               info.has_description(),
               "module info should have description");
  }

  [[nodiscard]] bool test_empty_module()
  {
    const kordex::bindings::NativeModule module;

    return expect_true(
               !module.valid(),
               "default module should not be valid") &&
           expect_true(
               !module.importable(),
               "default module should not be importable") &&
           expect_true(
               module.safe(),
               "default module should be safe") &&
           expect_true(
               module.empty(),
               "default module should be empty") &&
           expect_true(
               module.function_count() == 0,
               "default module function count should be zero") &&
           expect_true(
               module.value_count() == 0,
               "default module value count should be zero");
  }

  [[nodiscard]] bool test_create_module()
  {
    auto module = kordex::bindings::NativeModule::create("console");

    return expect_true(
               module.valid(),
               "created module should be valid") &&
           expect_true(
               module.importable(),
               "created module should be importable") &&
           expect_true(
               module.safe(),
               "created module should be safe") &&
           expect_true(
               std::string_view(module.name()) == "console",
               "module name should match");
  }

  [[nodiscard]] bool test_create_module_from_info()
  {
    kordex::bindings::NativeModuleInfo info;
    info.name = "process";
    info.namespace_name = "kordex.runtime";
    info.description = "Process module";
    info.safe = false;

    auto module = kordex::bindings::NativeModule::create(info);

    return expect_true(
               module.valid(),
               "module from info should be valid") &&
           expect_true(
               std::string_view(module.name()) == "process",
               "module name should match") &&
           expect_true(
               std::string_view(module.namespace_name()) == "kordex.runtime",
               "module namespace should match") &&
           expect_true(
               !module.safe(),
               "module safe flag should match");
  }

  [[nodiscard]] bool test_add_and_get_value()
  {
    auto module = kordex::bindings::NativeModule::create("env");

    const auto set_error = module.set_value(
        "name",
        kordex::bindings::Value::string("Kordex"));

    auto value = module.value("name");

    if (!expect_true(
            !set_error,
            "setting module value should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(value),
            "getting module value should succeed"))
    {
      return false;
    }

    auto string_value = value.value().as_string();

    if (!expect_true(
            result_ok(string_value),
            "module value should convert to string"))
    {
      return false;
    }

    return expect_true(
               module.has_value("name"),
               "module should have value") &&
           expect_true(
               module.value_count() == 1,
               "module value count should be one") &&
           expect_true(
               string_value.value() == "Kordex",
               "module value should match");
  }

  [[nodiscard]] bool test_add_and_get_function()
  {
    auto module = kordex::bindings::NativeModule::create("math");

    auto function = kordex::bindings::NativeFunction::create(
        "answer",
        [](const kordex::bindings::NativeFunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::number(42.0);
        });

    const auto add_error = module.add_function(function);

    auto stored_function = module.function("answer");

    if (!expect_true(
            !add_error,
            "adding native function should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(stored_function),
            "getting native function should succeed"))
    {
      return false;
    }

    auto call_result = stored_function.value().call();

    if (!expect_true(
            result_ok(call_result),
            "stored native function call should succeed"))
    {
      return false;
    }

    auto number = call_result.value().as_number();

    if (!expect_true(
            result_ok(number),
            "stored native function result should be number"))
    {
      return false;
    }

    return expect_true(
               module.has_function("answer"),
               "module should have function") &&
           expect_true(
               module.function_count() == 1,
               "module function count should be one") &&
           expect_true(
               number.value() == 42.0,
               "stored native function should return 42");
  }

  [[nodiscard]] bool test_add_function_with_explicit_name()
  {
    auto module = kordex::bindings::NativeModule::create("math");

    auto function = kordex::bindings::NativeFunction::create(
        "internal_answer",
        [](const kordex::bindings::NativeFunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::number(42.0);
        });

    const auto add_error = module.add_function("answer", function);

    return expect_true(
               !add_error,
               "adding native function with explicit name should succeed") &&
           expect_true(
               module.has_function("answer"),
               "module should store explicit function name") &&
           expect_true(
               !module.has_function("internal_answer"),
               "module should not store internal name automatically");
  }

  [[nodiscard]] bool test_call_function_by_name()
  {
    auto module = kordex::bindings::NativeModule::create("math");

    kordex::bindings::NativeFunctionInfo info;
    info.name = "add";
    info.min_args = 2;
    info.max_args = 2;

    auto function = kordex::bindings::NativeFunction::create(
        info,
        [](const kordex::bindings::NativeFunctionArguments &args)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          auto left = args[0].as_number();
          if (!left)
          {
            return left.error();
          }

          auto right = args[1].as_number();
          if (!right)
          {
            return right.error();
          }

          return kordex::bindings::Value::number(
              left.value() + right.value());
        });

    const auto add_error = module.add_function(function);

    kordex::bindings::NativeFunctionArguments args{
        kordex::bindings::Value::number(20.0),
        kordex::bindings::Value::number(22.0)};

    auto result = module.call("add", args);

    if (!expect_true(
            !add_error,
            "adding add function should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(result),
            "module call should succeed"))
    {
      return false;
    }

    auto number = result.value().as_number();

    if (!expect_true(
            result_ok(number),
            "module call result should be number"))
    {
      return false;
    }

    return expect_true(
        number.value() == 42.0,
        "module call should return 42");
  }

  [[nodiscard]] bool test_missing_entries()
  {
    auto module = kordex::bindings::NativeModule::create("empty");

    auto missing_function = module.function("missing_function");
    auto missing_value = module.value("missing_value");
    auto missing_call = module.call("missing_function");

    return expect_true(
               !missing_function,
               "missing function should fail") &&
           expect_true(
               !missing_value,
               "missing value should fail") &&
           expect_true(
               !missing_call,
               "missing call should fail") &&
           expect_true(
               missing_function.error().code() ==
                   kordex::bindings::ErrorCode::NotFound,
               "missing function should map to not found") &&
           expect_true(
               missing_value.error().code() ==
                   kordex::bindings::ErrorCode::NotFound,
               "missing value should map to not found");
  }

  [[nodiscard]] bool test_invalid_names()
  {
    auto module = kordex::bindings::NativeModule::create("test");

    const auto set_error = module.set_value(
        "",
        kordex::bindings::Value::number(1.0));

    auto get_value = module.value("");
    const auto remove_value_error = module.remove_value("");

    auto function = kordex::bindings::NativeFunction::create(
        "valid",
        [](const kordex::bindings::NativeFunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::undefined();
        });

    const auto add_function_error = module.add_function("", function);
    auto get_function = module.function("");
    const auto remove_function_error = module.remove_function("");

    return expect_true(
               set_error.has_error(),
               "set value with empty name should fail") &&
           expect_true(
               !get_value,
               "get value with empty name should fail") &&
           expect_true(
               remove_value_error.has_error(),
               "remove value with empty name should fail") &&
           expect_true(
               add_function_error.has_error(),
               "add function with empty name should fail") &&
           expect_true(
               !get_function,
               "get function with empty name should fail") &&
           expect_true(
               remove_function_error.has_error(),
               "remove function with empty name should fail") &&
           expect_true(
               set_error.code() ==
                   kordex::bindings::ErrorCode::InvalidArgument,
               "empty value name should map to invalid argument") &&
           expect_true(
               add_function_error.code() ==
                   kordex::bindings::ErrorCode::InvalidArgument,
               "empty function name should map to invalid argument");
  }

  [[nodiscard]] bool test_remove_entries()
  {
    auto module = kordex::bindings::NativeModule::create("test");

    const auto set_error = module.set_value(
        "version",
        kordex::bindings::Value::string("0.1.0"));

    auto function = kordex::bindings::NativeFunction::create(
        "hello",
        [](const kordex::bindings::NativeFunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::string("hello");
        });

    const auto add_error = module.add_function(function);

    const auto remove_value_error = module.remove_value("version");
    const auto remove_function_error = module.remove_function("hello");

    return expect_true(
               !set_error && !add_error,
               "setup entries should succeed") &&
           expect_true(
               !remove_value_error,
               "remove value should succeed") &&
           expect_true(
               !remove_function_error,
               "remove function should succeed") &&
           expect_true(
               !module.has_value("version"),
               "removed value should be missing") &&
           expect_true(
               !module.has_function("hello"),
               "removed function should be missing") &&
           expect_true(
               module.empty(),
               "module should be empty after removals");
  }

  [[nodiscard]] bool test_names_and_clear()
  {
    auto module = kordex::bindings::NativeModule::create("test");

    const auto value_error = module.set_value(
        "a",
        kordex::bindings::Value::number(1.0));

    auto function = kordex::bindings::NativeFunction::create(
        "b",
        [](const kordex::bindings::NativeFunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::undefined();
        });

    const auto function_error = module.add_function(function);

    const auto value_names = module.value_names();
    const auto function_names = module.function_names();

    const bool names_ok =
        value_names.size() == 1 &&
        value_names[0] == "a" &&
        function_names.size() == 1 &&
        function_names[0] == "b";

    module.clear();

    return expect_true(
               !value_error && !function_error,
               "adding entries should succeed") &&
           expect_true(
               names_ok,
               "entry names should match") &&
           expect_true(
               module.empty(),
               "module should be empty after clear") &&
           expect_true(
               module.value_count() == 0,
               "value count should be zero after clear") &&
           expect_true(
               module.function_count() == 0,
               "function count should be zero after clear");
  }

  [[nodiscard]] bool test_to_object()
  {
    auto module = kordex::bindings::NativeModule::create("console");

    const auto value_error = module.set_value(
        "version",
        kordex::bindings::Value::string("0.1.0"));

    auto function = kordex::bindings::NativeFunction::create(
        "log",
        [](const kordex::bindings::NativeFunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::undefined();
        });

    const auto function_error = module.add_function(function);

    const auto object = module.to_object();

    auto version = object.get("version");
    auto log = object.get("log");

    if (!expect_true(
            !value_error && !function_error,
            "module setup should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(version) && result_ok(log),
            "module object properties should exist"))
    {
      return false;
    }

    return expect_true(
               std::string_view(object.name()) == "console",
               "module object name should match") &&
           expect_true(
               object.has("version"),
               "module object should contain value") &&
           expect_true(
               object.has("log"),
               "module object should contain function placeholder") &&
           expect_true(
               std::string_view(log.value().display()) ==
                   "[native function]",
               "function placeholder should match");
  }

} // namespace

int main()
{
  const bool ok =
      test_native_module_info_helpers() &&
      test_empty_module() &&
      test_create_module() &&
      test_create_module_from_info() &&
      test_add_and_get_value() &&
      test_add_and_get_function() &&
      test_add_function_with_explicit_name() &&
      test_call_function_by_name() &&
      test_missing_entries() &&
      test_invalid_names() &&
      test_remove_entries() &&
      test_names_and_clear() &&
      test_to_object();

  return ok ? 0 : 1;
}
