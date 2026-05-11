/**
 *
 *  @file test_binding_result.cpp
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
#include <kordex/bindings/BindingResult.hpp>

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
    const auto result = kordex::bindings::BindingResult::success(
        "script executed successfully");

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
               "success exit code should be 0") &&
           expect_true(
               !result.error.has_error(),
               "success result should not contain error") &&
           expect_true(
               result.has_output(),
               "success result should contain output") &&
           expect_true(
               std::string_view(result.output) ==
                   "script executed successfully",
               "success output should match");
  }

  [[nodiscard]] bool test_failure_result()
  {
    const auto error = kordex::bindings::make_binding_error(
        kordex::bindings::BindingErrorCode::ScriptExecutionFailed,
        "script execution failed");

    const auto result = kordex::bindings::BindingResult::failure(
        error,
        2);

    return expect_true(
               !result.succeeded(),
               "failure result should not report success") &&
           expect_true(
               result.failed(),
               "failure result should report failure") &&
           expect_true(
               result.exit_code == 2,
               "failure exit code should match") &&
           expect_true(
               result.error.has_error(),
               "failure result should contain error") &&
           expect_true(
               result.error.code() ==
                   kordex::bindings::ErrorCode::InternalError,
               "failure error code should match mapped error code") &&
           expect_true(
               std::string_view(result.error.message()) ==
                   "script execution failed",
               "failure error message should match");
  }

  [[nodiscard]] bool test_failure_exit_code_zero_is_normalized()
  {
    const auto error = kordex::bindings::make_binding_error(
        kordex::bindings::BindingErrorCode::InternalError,
        "internal failure");

    const auto result = kordex::bindings::BindingResult::failure(
        error,
        0);

    return expect_true(
               result.failed(),
               "failure result should report failure") &&
           expect_true(
               result.exit_code == 1,
               "failure exit code zero should be normalized to 1");
  }

  [[nodiscard]] bool test_cancelled_result()
  {
    const auto result = kordex::bindings::BindingResult::cancelled(
        "operation cancelled");

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
                   "operation cancelled",
               "cancelled error message should match");
  }

  [[nodiscard]] bool test_timeout_result()
  {
    const auto result = kordex::bindings::BindingResult::timeout(
        "operation timed out");

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
                   "operation timed out",
               "timeout error message should match");
  }

  [[nodiscard]] bool test_output_helpers()
  {
    auto result = kordex::bindings::BindingResult::success();

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

  [[nodiscard]] bool test_status_strings()
  {
    return expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::BindingExitStatus::Success)) ==
                   "success",
               "success status string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::BindingExitStatus::Failed)) ==
                   "failed",
               "failed status string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::BindingExitStatus::Cancelled)) ==
                   "cancelled",
               "cancelled status string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::BindingExitStatus::Timeout)) ==
                   "timeout",
               "timeout status string should match");
  }
} // namespace

int main()
{
  const bool ok =
      test_success_result() &&
      test_failure_result() &&
      test_failure_exit_code_zero_is_normalized() &&
      test_cancelled_result() &&
      test_timeout_result() &&
      test_output_helpers() &&
      test_status_strings();

  return ok ? 0 : 1;
}
