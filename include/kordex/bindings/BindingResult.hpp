/**
 *
 *  @file BindingResult.hpp
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

#ifndef KORDEX_BINDINGS_BINDING_RESULT_HPP
#define KORDEX_BINDINGS_BINDING_RESULT_HPP

#include <string>

#include <kordex/bindings/Error.hpp>
#include <kordex/bindings/Result.hpp>

namespace kordex::bindings
{
  /**
   * @enum BindingExitStatus
   * @brief High-level status for a bindings operation.
   */
  enum class BindingExitStatus
  {
    Success,
    Failed,
    Cancelled,
    Timeout
  };

  /**
   * @struct BindingResult
   * @brief Result produced by high-level bindings operations.
   *
   * BindingResult is used for operations such as:
   * - engine initialization
   * - context creation
   * - script compilation
   * - script execution
   * - native function calls
   * - runtime bridge calls
   */
  struct BindingResult
  {
    /**
     * @brief Final bindings status.
     */
    BindingExitStatus status{BindingExitStatus::Success};

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
     * @brief Optional human-readable output.
     */
    std::string output{};

    /**
     * @brief Optional human-readable stderr output.
     */
    std::string error_output{};

    /**
     * @brief Build a successful result.
     */
    [[nodiscard]] static BindingResult success(
        std::string output = {});

    /**
     * @brief Build a failed result from an error.
     */
    [[nodiscard]] static BindingResult failure(
        Error error,
        int exit_code = 1);

    /**
     * @brief Build a cancelled result.
     */
    [[nodiscard]] static BindingResult cancelled(
        std::string message = "bindings operation was cancelled");

    /**
     * @brief Build a timeout result.
     */
    [[nodiscard]] static BindingResult timeout(
        std::string message = "bindings operation timed out");

    /**
     * @brief Return true when the bindings operation succeeded.
     */
    [[nodiscard]] bool succeeded() const noexcept;

    /**
     * @brief Return true when the bindings operation failed.
     */
    [[nodiscard]] bool failed() const noexcept;

    /**
     * @brief Return true when the bindings operation was cancelled.
     */
    [[nodiscard]] bool was_cancelled() const noexcept;

    /**
     * @brief Return true when the bindings operation timed out.
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
  };

  /**
   * @brief Result type used by operations returning BindingResult.
   */
  using BindingExecutionResult = Result<BindingResult>;

  /**
   * @brief Convert BindingExitStatus to a stable string.
   */
  [[nodiscard]] const char *to_string(BindingExitStatus status) noexcept;

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_BINDING_RESULT_HPP
