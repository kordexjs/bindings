/**
 *
 *  @file test_function.cpp
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
#include <kordex/bindings/Function.hpp>

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

  [[nodiscard]] bool test_function_info_helpers()
  {
    kordex::bindings::FunctionInfo info;
    info.name = "sum";
    info.min_args = 1;
    info.max_args = 2;

    return expect_true(
               info.has_name(),
               "function info should have name") &&
           expect_true(
               info.has_max_args(),
               "function info should have max args") &&
           expect_true(
               info.accepts(1),
               "function info should accept min args") &&
           expect_true(
               info.accepts(2),
               "function info should accept max args") &&
           expect_true(
               !info.accepts(0),
               "function info should reject too few args") &&
           expect_true(
               !info.accepts(3),
               "function info should reject too many args");
  }

  [[nodiscard]] bool test_empty_function()
  {
    const kordex::bindings::Function function;

    auto result = function.call();

    return expect_true(
               !function.valid(),
               "empty function should not be valid") &&
           expect_true(
               !function.callable(),
               "empty function should not be callable") &&
           expect_true(
               !result,
               "calling empty function should fail") &&
           expect_true(
               result.error().code() ==
                   kordex::bindings::ErrorCode::InternalError,
               "empty function call should map to internal error");
  }

  [[nodiscard]] bool test_create_and_call_function()
  {
    auto function = kordex::bindings::Function::create(
        "hello",
        [](const kordex::bindings::FunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::string("hello from function");
        });

    auto result = function.call();

    if (!expect_true(
            result_ok(result),
            "function call should succeed"))
    {
      return false;
    }

    return expect_true(
               function.valid(),
               "created function should be valid") &&
           expect_true(
               function.callable(),
               "created function should be callable") &&
           expect_true(
               std::string_view(function.name()) == "hello",
               "function name should match") &&
           expect_true(
               result.value().is_string(),
               "function result should be string") &&
           expect_true(
               std::string_view(result.value().display()) ==
                   "hello from function",
               "function output should match");
  }

  [[nodiscard]] bool test_function_arguments()
  {
    kordex::bindings::FunctionInfo info;
    info.name = "add";
    info.min_args = 2;
    info.max_args = 2;

    auto function = kordex::bindings::Function::create(
        info,
        [](const kordex::bindings::FunctionArguments &args)
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

    kordex::bindings::FunctionArguments args{
        kordex::bindings::Value::number(20.0),
        kordex::bindings::Value::number(22.0)};

    auto result = function.call(args);

    if (!expect_true(
            result_ok(result),
            "add function should succeed"))
    {
      return false;
    }

    auto number = result.value().as_number();

    if (!expect_true(
            result_ok(number),
            "add function result should convert to number"))
    {
      return false;
    }

    return expect_true(
               result.value().is_number(),
               "add function should return number") &&
           expect_true(
               number.value() == 42.0,
               "add function should return 42");
  }

  [[nodiscard]] bool test_invalid_argument_count()
  {
    kordex::bindings::FunctionInfo info;
    info.name = "one_arg";
    info.min_args = 1;
    info.max_args = 1;

    auto function = kordex::bindings::Function::create(
        info,
        [](const kordex::bindings::FunctionArguments &)
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

  [[nodiscard]] bool test_not_callable_function()
  {
    kordex::bindings::FunctionInfo info;
    info.name = "blocked";
    info.callable = false;

    auto function = kordex::bindings::Function::create(
        info,
        [](const kordex::bindings::FunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::undefined();
        });

    auto result = function.call();

    return expect_true(
               function.valid(),
               "blocked function should still be valid") &&
           expect_true(
               !function.callable(),
               "blocked function should not be callable") &&
           expect_true(
               !result,
               "blocked function call should fail") &&
           expect_true(
               result.error().code() ==
                   kordex::bindings::ErrorCode::PermissionDenied,
               "blocked function should map to permission denied");
  }

  [[nodiscard]] bool test_call_operator()
  {
    auto function = kordex::bindings::Function::create(
        "answer",
        [](const kordex::bindings::FunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::number(42.0);
        });

    auto result = function();

    if (!expect_true(
            result_ok(result),
            "operator call should succeed"))
    {
      return false;
    }

    auto number = result.value().as_number();

    if (!expect_true(
            result_ok(number),
            "operator call result should convert to number"))
    {
      return false;
    }

    return expect_true(
               result.value().is_number(),
               "operator call should return number") &&
           expect_true(
               number.value() == 42.0,
               "operator call should return 42");
  }
} // namespace

int main()
{
  const bool ok =
      test_function_info_helpers() &&
      test_empty_function() &&
      test_create_and_call_function() &&
      test_function_arguments() &&
      test_invalid_argument_count() &&
      test_not_callable_function() &&
      test_call_operator();

  return ok ? 0 : 1;
}
