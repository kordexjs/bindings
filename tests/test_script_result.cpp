/**
 *
 *  @file test_script_result.cpp
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
#include <kordex/bindings/ScriptResult.hpp>

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

  [[nodiscard]] bool test_success_result()
  {
    const auto result = kordex::bindings::ScriptResult::success(
        kordex::bindings::Value::number(42.0),
        "script executed successfully");

    auto number = result.value.as_number();

    if (!expect_true(
            !(!number),
            "success value should convert to number"))
    {
      return false;
    }

    return expect_true(
               result.succeeded(),
               "success result should report success") &&
           expect_true(
               !result.failed(),
               "success result should not report failure") &&
           expect_true(
               !result.was_cancelled(),
               "success result should not report cancellation") &&
           expect_true(
               !result.timed_out(),
               "success result should not report timeout") &&
           expect_true(
               result.exit_code == 0,
               "success exit code should be zero") &&
           expect_true(
               !result.error.has_error(),
               "success result should not contain error") &&
           expect_true(
               result.has_value(),
               "success result should contain value") &&
           expect_true(
               result.has_output(),
               "success result should contain output") &&
           expect_true(
               number.value() == 42.0,
               "success value should be 42") &&
           expect_true(
               std::string_view(result.output) ==
                   "script executed successfully",
               "success output should match");
  }

  [[nodiscard]] bool test_success_without_value()
  {
    const auto result = kordex::bindings::ScriptResult::success();

    return expect_true(
               result.succeeded(),
               "empty success result should report success") &&
           expect_true(
               !result.has_value(),
               "empty success result should not have value") &&
           expect_true(
               result.value.is_undefined(),
               "empty success value should be undefined") &&
           expect_true(
               !result.has_output(),
               "empty success result should not have output");
  }

  [[nodiscard]] bool test_failure_result()
  {
    const auto error = kordex::bindings::make_binding_error(
        kordex::bindings::BindingErrorCode::ScriptExecutionFailed,
        "script execution failed");

    const auto result = kordex::bindings::ScriptResult::failure(
        error,
        2);

    return expect_true(
               !result.succeeded(),
               "failure result should not report success") &&
           expect_true(
               result.failed(),
               "failure result should report failure") &&
           expect_true(
               !result.was_cancelled(),
               "failure result should not report cancellation") &&
           expect_true(
               !result.timed_out(),
               "failure result should not report timeout") &&
           expect_true(
               result.exit_code == 2,
               "failure exit code should match") &&
           expect_true(
               result.error.has_error(),
               "failure result should contain error") &&
           expect_true(
               result.error.code() ==
                   kordex::bindings::ErrorCode::InternalError,
               "failure error code should match") &&
           expect_true(
               std::string_view(result.error.message()) ==
                   "script execution failed",
               "failure error message should match") &&
           expect_true(
               result.value.is_undefined(),
               "failure value should be undefined");
  }

  [[nodiscard]] bool test_failure_exit_code_zero_is_normalized()
  {
    const auto error = kordex::bindings::make_binding_error(
        kordex::bindings::BindingErrorCode::InternalError,
        "internal failure");

    const auto result = kordex::bindings::ScriptResult::failure(
        error,
        0);

    return expect_true(
               result.failed(),
               "failure result should report failure") &&
           expect_true(
               result.exit_code == 1,
               "failure exit code zero should normalize to one");
  }

  [[nodiscard]] bool test_cancelled_result()
  {
    const auto result = kordex::bindings::ScriptResult::cancelled(
        "script cancelled");

    return expect_true(
               !result.succeeded(),
               "cancelled result should not report success") &&
           expect_true(
               result.was_cancelled(),
               "cancelled result should report cancellation") &&
           expect_true(
               result.exit_code == 130,
               "cancelled exit code should be 130") &&
           expect_true(
               result.error.has_error(),
               "cancelled result should contain error") &&
           expect_true(
               result.error.code() ==
                   kordex::bindings::ErrorCode::Cancelled,
               "cancelled error code should match") &&
           expect_true(
               std::string_view(result.error.message()) ==
                   "script cancelled",
               "cancelled error message should match") &&
           expect_true(
               result.value.is_undefined(),
               "cancelled value should be undefined");
  }

  [[nodiscard]] bool test_timeout_result()
  {
    const auto result = kordex::bindings::ScriptResult::timeout(
        "script timed out");

    return expect_true(
               !result.succeeded(),
               "timeout result should not report success") &&
           expect_true(
               result.timed_out(),
               "timeout result should report timeout") &&
           expect_true(
               result.exit_code == 124,
               "timeout exit code should be 124") &&
           expect_true(
               result.error.has_error(),
               "timeout result should contain error") &&
           expect_true(
               result.error.code() ==
                   kordex::bindings::ErrorCode::Timeout,
               "timeout error code should match") &&
           expect_true(
               std::string_view(result.error.message()) ==
                   "script timed out",
               "timeout error message should match") &&
           expect_true(
               result.value.is_undefined(),
               "timeout value should be undefined");
  }

  [[nodiscard]] bool test_output_helpers()
  {
    auto result = kordex::bindings::ScriptResult::success();

    const bool initially_empty =
        !result.has_output() &&
        !result.has_error_output();

    result.output = "stdout text";
    result.error_output = "stderr text";

    return expect_true(
               initially_empty,
               "result should initially have no output") &&
           expect_true(
               result.has_output(),
               "result should detect stdout output") &&
           expect_true(
               result.has_error_output(),
               "result should detect stderr output");
  }

  [[nodiscard]] bool test_from_binding_success_result()
  {
    const auto binding_result = kordex::bindings::BindingResult::success(
        "binding output");

    const auto result = kordex::bindings::ScriptResult::from_binding_result(
        binding_result);

    return expect_true(
               result.succeeded(),
               "script result from binding success should succeed") &&
           expect_true(
               result.exit_code == 0,
               "script result exit code should match") &&
           expect_true(
               !result.error.has_error(),
               "script result should not contain error") &&
           expect_true(
               result.has_value(),
               "script result should contain converted output value") &&
           expect_true(
               result.value.is_string(),
               "converted binding output should be string") &&
           expect_true(
               std::string_view(result.value.display()) == "binding output",
               "converted binding output should match") &&
           expect_true(
               std::string_view(result.output) == "binding output",
               "script output should match binding output");
  }

  [[nodiscard]] bool test_from_binding_failure_result()
  {
    const auto error = kordex::bindings::make_binding_error(
        kordex::bindings::BindingErrorCode::FunctionCallFailed,
        "function call failed");

    const auto binding_result = kordex::bindings::BindingResult::failure(
        error,
        3);

    const auto result = kordex::bindings::ScriptResult::from_binding_result(
        binding_result);

    return expect_true(
               result.failed(),
               "script result from binding failure should fail") &&
           expect_true(
               result.exit_code == 3,
               "script result failure exit code should match") &&
           expect_true(
               result.error.has_error(),
               "script result should contain error") &&
           expect_true(
               result.error.code() ==
                   kordex::bindings::ErrorCode::InternalError,
               "script result error code should match") &&
           expect_true(
               result.value.is_undefined(),
               "script failure value should be undefined");
  }

  [[nodiscard]] bool test_to_binding_result()
  {
    auto result = kordex::bindings::ScriptResult::success(
        kordex::bindings::Value::string("value"),
        "script output");

    result.error_output = "script stderr";

    const auto binding_result = result.to_binding_result();

    return expect_true(
               binding_result.succeeded(),
               "converted binding result should succeed") &&
           expect_true(
               binding_result.exit_code == 0,
               "converted binding exit code should match") &&
           expect_true(
               !binding_result.error.has_error(),
               "converted binding result should not contain error") &&
           expect_true(
               std::string_view(binding_result.output) == "script output",
               "converted binding output should match") &&
           expect_true(
               std::string_view(binding_result.error_output) ==
                   "script stderr",
               "converted binding stderr should match");
  }

  [[nodiscard]] bool test_status_strings()
  {
    return expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::ScriptExitStatus::Success)) ==
                   "success",
               "success status string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::ScriptExitStatus::Failed)) ==
                   "failed",
               "failed status string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::ScriptExitStatus::Cancelled)) ==
                   "cancelled",
               "cancelled status string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::ScriptExitStatus::Timeout)) ==
                   "timeout",
               "timeout status string should match");
  }

  [[nodiscard]] bool test_status_conversions()
  {
    return expect_true(
               kordex::bindings::to_script_exit_status(
                   kordex::bindings::BindingExitStatus::Success) ==
                   kordex::bindings::ScriptExitStatus::Success,
               "binding success should convert to script success") &&
           expect_true(
               kordex::bindings::to_script_exit_status(
                   kordex::bindings::BindingExitStatus::Failed) ==
                   kordex::bindings::ScriptExitStatus::Failed,
               "binding failed should convert to script failed") &&
           expect_true(
               kordex::bindings::to_script_exit_status(
                   kordex::bindings::BindingExitStatus::Cancelled) ==
                   kordex::bindings::ScriptExitStatus::Cancelled,
               "binding cancelled should convert to script cancelled") &&
           expect_true(
               kordex::bindings::to_script_exit_status(
                   kordex::bindings::BindingExitStatus::Timeout) ==
                   kordex::bindings::ScriptExitStatus::Timeout,
               "binding timeout should convert to script timeout") &&
           expect_true(
               kordex::bindings::to_binding_exit_status(
                   kordex::bindings::ScriptExitStatus::Success) ==
                   kordex::bindings::BindingExitStatus::Success,
               "script success should convert to binding success") &&
           expect_true(
               kordex::bindings::to_binding_exit_status(
                   kordex::bindings::ScriptExitStatus::Failed) ==
                   kordex::bindings::BindingExitStatus::Failed,
               "script failed should convert to binding failed") &&
           expect_true(
               kordex::bindings::to_binding_exit_status(
                   kordex::bindings::ScriptExitStatus::Cancelled) ==
                   kordex::bindings::BindingExitStatus::Cancelled,
               "script cancelled should convert to binding cancelled") &&
           expect_true(
               kordex::bindings::to_binding_exit_status(
                   kordex::bindings::ScriptExitStatus::Timeout) ==
                   kordex::bindings::BindingExitStatus::Timeout,
               "script timeout should convert to binding timeout");
  }
} // namespace

int main()
{
  const bool ok =
      test_success_result() &&
      test_success_without_value() &&
      test_failure_result() &&
      test_failure_exit_code_zero_is_normalized() &&
      test_cancelled_result() &&
      test_timeout_result() &&
      test_output_helpers() &&
      test_from_binding_success_result() &&
      test_from_binding_failure_result() &&
      test_to_binding_result() &&
      test_status_strings() &&
      test_status_conversions();

  return ok ? 0 : 1;
}
