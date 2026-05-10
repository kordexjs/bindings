/**
 *
 *  @file Error.hpp
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

#ifndef KORDEX_BINDINGS_ERROR_HPP
#define KORDEX_BINDINGS_ERROR_HPP

#include <string>
#include <utility>

#include <vix/error/Error.hpp>
#include <vix/error/ErrorCategory.hpp>
#include <vix/error/ErrorCode.hpp>
#include <vix/error/Exception.hpp>

namespace kordex::bindings
{
  /**
   * @brief Kordex bindings error alias.
   *
   * Bindings uses the shared Vix structured error model.
   */
  using Error = vix::error::Error;

  /**
   * @brief Kordex bindings exception alias.
   */
  using Exception = vix::error::Exception;

  /**
   * @brief Kordex bindings error code alias.
   */
  using ErrorCode = vix::error::ErrorCode;

  /**
   * @brief Kordex bindings error category alias.
   */
  using ErrorCategory = vix::error::ErrorCategory;

  /**
   * @enum BindingErrorCode
   * @brief Binding-specific semantic error codes.
   */
  enum class BindingErrorCode
  {
    None = 0,
    InvalidArgument,
    InvalidState,
    InvalidConfig,
    EngineUnavailable,
    EngineInitializationFailed,
    ContextUnavailable,
    ScriptCompileFailed,
    ScriptExecutionFailed,
    ValueConversionFailed,
    FunctionCallFailed,
    ModuleNotFound,
    ModuleRegistrationFailed,
    RuntimeBridgeFailed,
    PermissionDenied,
    Timeout,
    Cancelled,
    NotImplemented,
    InternalError
  };

  /**
   * @brief Return the default Kordex bindings error category.
   */
  [[nodiscard]] constexpr ErrorCategory binding_error_category() noexcept
  {
    return ErrorCategory("kordex.bindings");
  }

  /**
   * @brief Convert a BindingErrorCode to a shared ErrorCode.
   */
  [[nodiscard]] constexpr ErrorCode to_error_code(BindingErrorCode code) noexcept
  {
    switch (code)
    {
    case BindingErrorCode::None:
      return ErrorCode::Ok;

    case BindingErrorCode::InvalidArgument:
      return ErrorCode::InvalidArgument;

    case BindingErrorCode::InvalidState:
      return ErrorCode::InvalidState;

    case BindingErrorCode::InvalidConfig:
      return ErrorCode::ConfigError;

    case BindingErrorCode::EngineUnavailable:
    case BindingErrorCode::ContextUnavailable:
      return ErrorCode::NotFound;

    case BindingErrorCode::EngineInitializationFailed:
    case BindingErrorCode::ScriptCompileFailed:
    case BindingErrorCode::ScriptExecutionFailed:
    case BindingErrorCode::FunctionCallFailed:
    case BindingErrorCode::ModuleRegistrationFailed:
    case BindingErrorCode::RuntimeBridgeFailed:
      return ErrorCode::InternalError;

    case BindingErrorCode::ValueConversionFailed:
      return ErrorCode::InvalidArgument;

    case BindingErrorCode::ModuleNotFound:
      return ErrorCode::NotFound;

    case BindingErrorCode::PermissionDenied:
      return ErrorCode::PermissionDenied;

    case BindingErrorCode::Timeout:
      return ErrorCode::Timeout;

    case BindingErrorCode::Cancelled:
      return ErrorCode::Cancelled;

    case BindingErrorCode::NotImplemented:
      return ErrorCode::NotImplemented;

    case BindingErrorCode::InternalError:
      return ErrorCode::InternalError;
    }

    return ErrorCode::Unknown;
  }

  /**
   * @brief Convert a BindingErrorCode to a stable string.
   */
  [[nodiscard]] constexpr const char *to_string(BindingErrorCode code) noexcept
  {
    switch (code)
    {
    case BindingErrorCode::None:
      return "none";
    case BindingErrorCode::InvalidArgument:
      return "invalid_argument";
    case BindingErrorCode::InvalidState:
      return "invalid_state";
    case BindingErrorCode::InvalidConfig:
      return "invalid_config";
    case BindingErrorCode::EngineUnavailable:
      return "engine_unavailable";
    case BindingErrorCode::EngineInitializationFailed:
      return "engine_initialization_failed";
    case BindingErrorCode::ContextUnavailable:
      return "context_unavailable";
    case BindingErrorCode::ScriptCompileFailed:
      return "script_compile_failed";
    case BindingErrorCode::ScriptExecutionFailed:
      return "script_execution_failed";
    case BindingErrorCode::ValueConversionFailed:
      return "value_conversion_failed";
    case BindingErrorCode::FunctionCallFailed:
      return "function_call_failed";
    case BindingErrorCode::ModuleNotFound:
      return "module_not_found";
    case BindingErrorCode::ModuleRegistrationFailed:
      return "module_registration_failed";
    case BindingErrorCode::RuntimeBridgeFailed:
      return "runtime_bridge_failed";
    case BindingErrorCode::PermissionDenied:
      return "permission_denied";
    case BindingErrorCode::Timeout:
      return "timeout";
    case BindingErrorCode::Cancelled:
      return "cancelled";
    case BindingErrorCode::NotImplemented:
      return "not_implemented";
    case BindingErrorCode::InternalError:
      return "internal_error";
    }

    return "unknown";
  }

  /**
   * @brief Build a structured bindings error.
   */
  [[nodiscard]] inline Error make_binding_error(
      BindingErrorCode code,
      std::string message)
  {
    return Error(
        to_error_code(code),
        binding_error_category(),
        std::move(message));
  }

  /**
   * @brief Return a success error object.
   */
  [[nodiscard]] inline Error ok() noexcept
  {
    return {};
  }

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_ERROR_HPP
