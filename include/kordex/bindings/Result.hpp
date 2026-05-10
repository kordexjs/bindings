/**
 *
 *  @file Result.hpp
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

#ifndef KORDEX_BINDINGS_RESULT_HPP
#define KORDEX_BINDINGS_RESULT_HPP

#include <string>

#include <vix/error/Result.hpp>
#include <kordex/bindings/Error.hpp>

namespace kordex::bindings
{
  /**
   * @brief Kordex bindings result type.
   *
   * This is a thin alias over the shared Vix Result<T> type.
   */
  template <typename T>
  using Result = vix::error::Result<T>;

  /**
   * @brief Result type for operations returning only success/failure.
   */
  using BoolResult = Result<bool>;

  /**
   * @brief Result type for string-producing binding operations.
   */
  using StringResult = Result<std::string>;

  /**
   * @brief Result type for integer-producing binding operations.
   */
  using IntResult = Result<int>;

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_RESULT_HPP
