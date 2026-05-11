/**
 *
 *  @file BindingResult.cpp
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

#include <utility>

#include <kordex/bindings/BindingResult.hpp>

namespace kordex::bindings
{
  BindingResult BindingResult::success(
      std::string output_value)
  {
    BindingResult result;
    result.status = BindingExitStatus::Success;
    result.exit_code = 0;
    result.error = ok();
    result.output = std::move(output_value);
    return result;
  }

  BindingResult BindingResult::failure(
      Error error_value,
      int exit_code_value)
  {
    BindingResult result;
    result.status = BindingExitStatus::Failed;
    result.exit_code = exit_code_value == 0 ? 1 : exit_code_value;
    result.error = std::move(error_value);
    return result;
  }

  BindingResult BindingResult::cancelled(
      std::string message)
  {
    BindingResult result;
    result.status = BindingExitStatus::Cancelled;
    result.exit_code = 130;
    result.error = make_binding_error(
        BindingErrorCode::Cancelled,
        std::move(message));
    return result;
  }

  BindingResult BindingResult::timeout(
      std::string message)
  {
    BindingResult result;
    result.status = BindingExitStatus::Timeout;
    result.exit_code = 124;
    result.error = make_binding_error(
        BindingErrorCode::Timeout,
        std::move(message));
    return result;
  }

  bool BindingResult::succeeded() const noexcept
  {
    return status == BindingExitStatus::Success &&
           exit_code == 0 &&
           !error.has_error();
  }

  bool BindingResult::failed() const noexcept
  {
    return status == BindingExitStatus::Failed;
  }

  bool BindingResult::was_cancelled() const noexcept
  {
    return status == BindingExitStatus::Cancelled;
  }

  bool BindingResult::timed_out() const noexcept
  {
    return status == BindingExitStatus::Timeout;
  }

  bool BindingResult::has_output() const noexcept
  {
    return !output.empty();
  }

  bool BindingResult::has_error_output() const noexcept
  {
    return !error_output.empty();
  }

  const char *to_string(BindingExitStatus status) noexcept
  {
    switch (status)
    {
    case BindingExitStatus::Success:
      return "success";
    case BindingExitStatus::Failed:
      return "failed";
    case BindingExitStatus::Cancelled:
      return "cancelled";
    case BindingExitStatus::Timeout:
      return "timeout";
    }

    return "failed";
  }

} // namespace kordex::bindings
