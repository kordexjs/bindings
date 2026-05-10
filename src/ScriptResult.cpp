/**
 *
 *  @file ScriptResult.cpp
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

#include <utility>
#include <kordex/bindings/ScriptResult.hpp>

namespace kordex::bindings
{
  ScriptResult ScriptResult::success(
      Value value_result,
      std::string output_value)
  {
    ScriptResult result;
    result.status = ScriptExitStatus::Success;
    result.exit_code = 0;
    result.error = ok();
    result.value = std::move(value_result);
    result.output = std::move(output_value);
    return result;
  }

  ScriptResult ScriptResult::failure(
      Error error_value,
      int exit_code_value)
  {
    ScriptResult result;
    result.status = ScriptExitStatus::Failed;
    result.exit_code = exit_code_value == 0 ? 1 : exit_code_value;
    result.error = std::move(error_value);
    result.value = Value::undefined();
    return result;
  }

  ScriptResult ScriptResult::cancelled(
      std::string message)
  {
    ScriptResult result;
    result.status = ScriptExitStatus::Cancelled;
    result.exit_code = 130;
    result.error = make_binding_error(
        BindingErrorCode::Cancelled,
        std::move(message));
    result.value = Value::undefined();
    return result;
  }

  ScriptResult ScriptResult::timeout(
      std::string message)
  {
    ScriptResult result;
    result.status = ScriptExitStatus::Timeout;
    result.exit_code = 124;
    result.error = make_binding_error(
        BindingErrorCode::Timeout,
        std::move(message));
    result.value = Value::undefined();
    return result;
  }

  ScriptResult ScriptResult::from_binding_result(
      const BindingResult &binding_result)
  {
    ScriptResult result;
    result.status = to_script_exit_status(binding_result.status);
    result.exit_code = binding_result.exit_code;
    result.error = binding_result.error;
    result.output = binding_result.output;
    result.error_output = binding_result.error_output;
    result.value = binding_result.succeeded()
                       ? Value::string(binding_result.output)
                       : Value::undefined();

    return result;
  }

  BindingResult ScriptResult::to_binding_result() const
  {
    BindingResult result;
    result.status = to_binding_exit_status(status);
    result.exit_code = exit_code;
    result.error = error;
    result.output = output;
    result.error_output = error_output;
    return result;
  }

  bool ScriptResult::succeeded() const noexcept
  {
    return status == ScriptExitStatus::Success &&
           exit_code == 0 &&
           !error.has_error();
  }

  bool ScriptResult::failed() const noexcept
  {
    return status == ScriptExitStatus::Failed;
  }

  bool ScriptResult::was_cancelled() const noexcept
  {
    return status == ScriptExitStatus::Cancelled;
  }

  bool ScriptResult::timed_out() const noexcept
  {
    return status == ScriptExitStatus::Timeout;
  }

  bool ScriptResult::has_output() const noexcept
  {
    return !output.empty();
  }

  bool ScriptResult::has_error_output() const noexcept
  {
    return !error_output.empty();
  }

  bool ScriptResult::has_value() const noexcept
  {
    return !value.is_undefined();
  }

  const char *to_string(ScriptExitStatus status) noexcept
  {
    switch (status)
    {
    case ScriptExitStatus::Success:
      return "success";
    case ScriptExitStatus::Failed:
      return "failed";
    case ScriptExitStatus::Cancelled:
      return "cancelled";
    case ScriptExitStatus::Timeout:
      return "timeout";
    }

    return "failed";
  }

  ScriptExitStatus to_script_exit_status(
      BindingExitStatus status) noexcept
  {
    switch (status)
    {
    case BindingExitStatus::Success:
      return ScriptExitStatus::Success;
    case BindingExitStatus::Failed:
      return ScriptExitStatus::Failed;
    case BindingExitStatus::Cancelled:
      return ScriptExitStatus::Cancelled;
    case BindingExitStatus::Timeout:
      return ScriptExitStatus::Timeout;
    }

    return ScriptExitStatus::Failed;
  }

  BindingExitStatus to_binding_exit_status(
      ScriptExitStatus status) noexcept
  {
    switch (status)
    {
    case ScriptExitStatus::Success:
      return BindingExitStatus::Success;
    case ScriptExitStatus::Failed:
      return BindingExitStatus::Failed;
    case ScriptExitStatus::Cancelled:
      return BindingExitStatus::Cancelled;
    case ScriptExitStatus::Timeout:
      return BindingExitStatus::Timeout;
    }

    return BindingExitStatus::Failed;
  }

} // namespace kordex::bindings
