/**
 *
 *  @file test_error.cpp
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
#include <kordex/bindings/Error.hpp>

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

  [[nodiscard]] bool test_ok_error()
  {
    const auto error = kordex::bindings::ok();

    return expect_true(
        !error.has_error(),
        "ok error should not contain an error");
  }

  [[nodiscard]] bool test_binding_error_category()
  {
    constexpr auto category = kordex::bindings::binding_error_category();

    return expect_true(
        std::string_view(category.name()) == "kordex.bindings",
        "binding error category should be kordex.bindings");
  }

  [[nodiscard]] bool test_binding_error_creation()
  {
    const auto error = kordex::bindings::make_binding_error(
        kordex::bindings::BindingErrorCode::InvalidArgument,
        "invalid binding argument");

    return expect_true(
               error.has_error(),
               "binding error should contain an error") &&
           expect_true(
               error.code() == kordex::bindings::ErrorCode::InvalidArgument,
               "binding error should map to InvalidArgument") &&
           expect_true(
               std::string_view(error.category().name()) == "kordex.bindings",
               "binding error category should match") &&
           expect_true(
               std::string_view(error.message()) == "invalid binding argument",
               "binding error message should match");
  }

  [[nodiscard]] bool test_error_code_mapping()
  {
    return expect_true(
               kordex::bindings::to_error_code(
                   kordex::bindings::BindingErrorCode::None) ==
                   kordex::bindings::ErrorCode::Ok,
               "none should map to Ok") &&
           expect_true(
               kordex::bindings::to_error_code(
                   kordex::bindings::BindingErrorCode::InvalidConfig) ==
                   kordex::bindings::ErrorCode::ConfigError,
               "invalid config should map to ConfigError") &&
           expect_true(
               kordex::bindings::to_error_code(
                   kordex::bindings::BindingErrorCode::ModuleNotFound) ==
                   kordex::bindings::ErrorCode::NotFound,
               "module not found should map to NotFound") &&
           expect_true(
               kordex::bindings::to_error_code(
                   kordex::bindings::BindingErrorCode::PermissionDenied) ==
                   kordex::bindings::ErrorCode::PermissionDenied,
               "permission denied should map to PermissionDenied") &&
           expect_true(
               kordex::bindings::to_error_code(
                   kordex::bindings::BindingErrorCode::Cancelled) ==
                   kordex::bindings::ErrorCode::Cancelled,
               "cancelled should map to Cancelled");
  }

  [[nodiscard]] bool test_error_code_strings()
  {
    return expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::BindingErrorCode::None)) == "none",
               "none string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::BindingErrorCode::EngineUnavailable)) ==
                   "engine_unavailable",
               "engine unavailable string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::BindingErrorCode::ScriptExecutionFailed)) ==
                   "script_execution_failed",
               "script execution failed string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::BindingErrorCode::RuntimeBridgeFailed)) ==
                   "runtime_bridge_failed",
               "runtime bridge failed string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::BindingErrorCode::InternalError)) ==
                   "internal_error",
               "internal error string should match");
  }
} // namespace

int main()
{
  const bool ok =
      test_ok_error() &&
      test_binding_error_category() &&
      test_binding_error_creation() &&
      test_error_code_mapping() &&
      test_error_code_strings();

  return ok ? 0 : 1;
}
