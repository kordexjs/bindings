/**
 *
 *  @file test_native_function.cpp
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
#include <kordex/bindings/NativeFunction.hpp>

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

  [[nodiscard]] bool test_native_function_info_helpers()
  {
    kordex::bindings::NativeFunctionInfo info;
    info.name = "read_file";
    info.module_name = "fs";
    info.description = "Read a file";
    info.min_args = 1;
    info.max_args = 2;

    return expect_true(
               info.has_name(),
               "native function info should have name") &&
           expect_true(
               info.has_module_name(),
               "native function info should have module name") &&
           expect_true(
               info.has_description(),
               "native function info should have description") &&
           expect_true(
               info.has_max_args(),
               "native function info should have max args") &&
           expect_true(
               info.accepts(1),
               "native function info should accept min args") &&
           expect_true(
               info.accepts(2),
               "native function info should accept max args") &&
           expect_true(
               !info.accepts(0),
               "native function info should reject too few args") &&
           expect_true(
               !info.accepts(3),
               "native function info should reject too many args");
  }

  [[nodiscard]] bool test_empty_native_function()
  {
    const kordex::bindings::NativeFunction function;

    auto result = function.call();

    return expect_true(
               !function.valid(),
               "empty native function should not be valid") &&
           expect_true(
               !function.callable(),
               "empty native function should not be callable") &&
           expect_true(
               !result,
               "calling empty native function should fail") &&
           expect_true(
               result.error().code() ==
                   kordex::bindings::ErrorCode::InternalError,
               "empty native function call should map to internal error");
  }

  [[nodiscard]] bool test_create_and_call_native_function()
  {
    auto function = kordex::bindings::NativeFunction::create(
        "hello",
        [](const kordex::bindings::NativeFunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::string("hello from native");
        });

    auto result = function.call();

    if (!expect_true(
            result_ok(result),
            "native function call should succeed"))
    {
      return false;
    }

    return expect_true(
               function.valid(),
               "created native function should be valid") &&
           expect_true(
               function.callable(),
               "created native function should be callable") &&
           expect_true(
               function.safe(),
               "created native function should be safe by default") &&
           expect_true(
               std::string_view(function.name()) == "hello",
               "native function name should match") &&
           expect_true(
               result.value().is_string(),
               "native function result should be string") &&
           expect_true(
               std::string_view(result.value().display()) ==
                   "hello from native",
               "native function output should match");
  }

  [[nodiscard]] bool test_native_function_arguments()
  {
    kordex::bindings::NativeFunctionInfo info;
    info.name = "multiply";
    info.module_name = "math";
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
              left.value() * right.value());
        });

    kordex::bindings::NativeFunctionArguments args{
        kordex::bindings::Value::number(6.0),
        kordex::bindings::Value::number(7.0)};

    auto result = function.call(args);

    if (!expect_true(
            result_ok(result),
            "multiply native function should succeed"))
    {
      return false;
    }

    auto number = result.value().as_number();

    if (!expect_true(
            result_ok(number),
            "multiply result should convert to number"))
    {
      return false;
    }

    return expect_true(
               result.value().is_number(),
               "multiply should return number") &&
           expect_true(
               number.value() == 42.0,
               "multiply should return 42") &&
           expect_true(
               std::string_view(function.module_name()) == "math",
               "module name should match");
  }

  [[nodiscard]] bool test_invalid_argument_count()
  {
    kordex::bindings::NativeFunctionInfo info;
    info.name = "one_arg";
    info.min_args = 1;
    info.max_args = 1;

    auto function = kordex::bindings::NativeFunction::create(
        info,
        [](const kordex::bindings::NativeFunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::undefined();
        });

    auto result = function.call({});

    return expect_true(
               !result,
               "calling with invalid argument count should fail") &&
           expect_true(
               result.error().code() ==
                   kordex::bindings::ErrorCode::InvalidArgument,
               "invalid argument count should map to invalid argument");
  }

  [[nodiscard]] bool test_not_callable_native_function()
  {
    kordex::bindings::NativeFunctionInfo info;
    info.name = "blocked";
    info.callable = false;

    auto function = kordex::bindings::NativeFunction::create(
        info,
        [](const kordex::bindings::NativeFunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::undefined();
        });

    auto result = function.call();

    return expect_true(
               function.valid(),
               "blocked native function should still be valid") &&
           expect_true(
               !function.callable(),
               "blocked native function should not be callable") &&
           expect_true(
               !result,
               "blocked native function call should fail") &&
           expect_true(
               result.error().code() ==
                   kordex::bindings::ErrorCode::PermissionDenied,
               "blocked native function should map to permission denied");
  }

  [[nodiscard]] bool test_call_operator()
  {
    auto function = kordex::bindings::NativeFunction::create(
        "answer",
        [](const kordex::bindings::NativeFunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::number(42.0);
        });

    auto result = function();

    if (!expect_true(
            result_ok(result),
            "native function operator call should succeed"))
    {
      return false;
    }

    auto number = result.value().as_number();

    if (!expect_true(
            result_ok(number),
            "native function operator result should convert to number"))
    {
      return false;
    }

    return expect_true(
               result.value().is_number(),
               "native function operator should return number") &&
           expect_true(
               number.value() == 42.0,
               "native function operator should return 42");
  }

  [[nodiscard]] bool test_convert_to_function()
  {
    kordex::bindings::NativeFunctionInfo info;
    info.name = "echo";
    info.min_args = 1;
    info.max_args = 1;

    auto native_function = kordex::bindings::NativeFunction::create(
        info,
        [](const kordex::bindings::NativeFunctionArguments &args)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return args[0];
        });

    auto function = native_function.to_function();

    kordex::bindings::FunctionArguments args{
        kordex::bindings::Value::string("hello")};

    auto result = function.call(args);

    if (!expect_true(
            result_ok(result),
            "converted function call should succeed"))
    {
      return false;
    }

    auto string_value = result.value().as_string();

    if (!expect_true(
            result_ok(string_value),
            "converted function result should be string"))
    {
      return false;
    }

    return expect_true(
               function.valid(),
               "converted function should be valid") &&
           expect_true(
               function.callable(),
               "converted function should be callable") &&
           expect_true(
               std::string_view(function.name()) == "echo",
               "converted function name should match") &&
           expect_true(
               string_value.value() == "hello",
               "converted function should return argument");
  }

} // namespace

int main()
{
  const bool ok =
      test_native_function_info_helpers() &&
      test_empty_native_function() &&
      test_create_and_call_native_function() &&
      test_native_function_arguments() &&
      test_invalid_argument_count() &&
      test_not_callable_native_function() &&
      test_call_operator() &&
      test_convert_to_function();

  return ok ? 0 : 1;
}
