/**
 *
 *  @file ScriptResult.hpp
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

#ifndef KORDEX_BINDINGS_SCRIPT_RESULT_HPP
#define KORDEX_BINDINGS_SCRIPT_RESULT_HPP

#include <string>

#include <kordex/bindings/BindingResult.hpp>
#include <kordex/bindings/Error.hpp>
#include <kordex/bindings/Result.hpp>
#include <kordex/bindings/Value.hpp>

namespace kordex::bindings
{
  /**
   * @enum ScriptExitStatus
   * @brief High-level status for script operations.
   */
  enum class ScriptExitStatus
  {
    Success,
    Failed,
    Cancelled,
    Timeout
  };

  /**
   * @struct ScriptResult
   * @brief Result produced by script compilation or execution.
   *
   * ScriptResult stores:
   * - final script status
   * - process-style exit code
   * - structured error
   * - returned script value
   * - stdout/stderr text
   */
  struct ScriptResult
  {
    /**
     * @brief Final script status.
     */
    ScriptExitStatus status{ScriptExitStatus::Success};

    /**
     * @brief Process-style exit code.
     *
     * By convention:
     * - 0 means success
     * - non-zero means failure
     */
    int exit_code{0};

    /**
     * @brief Optional structured error.
     */
    Error error{};

    /**
     * @brief Returned script value.
     */
    Value value{Value::undefined()};

    /**
     * @brief Optional human-readable output.
     */
    std::string output{};

    /**
     * @brief Optional human-readable stderr output.
     */
    std::string error_output{};

    /**
     * @brief Build a successful script result.
     */
    [[nodiscard]] static ScriptResult success(
        Value value = Value::undefined(),
        std::string output = {});

    /**
     * @brief Build a failed script result.
     */
    [[nodiscard]] static ScriptResult failure(
        Error error,
        int exit_code = 1);

    /**
     * @brief Build a cancelled script result.
     */
    [[nodiscard]] static ScriptResult cancelled(
        std::string message = "script operation was cancelled");

    /**
     * @brief Build a timeout script result.
     */
    [[nodiscard]] static ScriptResult timeout(
        std::string message = "script operation timed out");

    /**
     * @brief Build a ScriptResult from a BindingResult.
     */
    [[nodiscard]] static ScriptResult from_binding_result(
        const BindingResult &result);

    /**
     * @brief Convert this ScriptResult to a BindingResult.
     */
    [[nodiscard]] BindingResult to_binding_result() const;

    /**
     * @brief Return true when the script operation succeeded.
     */
    [[nodiscard]] bool succeeded() const noexcept;

    /**
     * @brief Return true when the script operation failed.
     */
    [[nodiscard]] bool failed() const noexcept;

    /**
     * @brief Return true when the script operation was cancelled.
     */
    [[nodiscard]] bool was_cancelled() const noexcept;

    /**
     * @brief Return true when the script operation timed out.
     */
    [[nodiscard]] bool timed_out() const noexcept;

    /**
     * @brief Return true if stdout output is available.
     */
    [[nodiscard]] bool has_output() const noexcept;

    /**
     * @brief Return true if stderr output is available.
     */
    [[nodiscard]] bool has_error_output() const noexcept;

    /**
     * @brief Return true if the returned value is not undefined.
     */
    [[nodiscard]] bool has_value() const noexcept;
  };

  /**
   * @brief Result type used by operations returning ScriptResult.
   */
  using ScriptExecutionResult = Result<ScriptResult>;

  /**
   * @brief Convert ScriptExitStatus to a stable string.
   */
  [[nodiscard]] const char *to_string(ScriptExitStatus status) noexcept;

  /**
   * @brief Convert BindingExitStatus to ScriptExitStatus.
   */
  [[nodiscard]] ScriptExitStatus to_script_exit_status(
      BindingExitStatus status) noexcept;

  /**
   * @brief Convert ScriptExitStatus to BindingExitStatus.
   */
  [[nodiscard]] BindingExitStatus to_binding_exit_status(
      ScriptExitStatus status) noexcept;

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_SCRIPT_RESULT_HPP
