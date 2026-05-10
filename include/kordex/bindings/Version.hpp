/**
 *
 *  @file Version.hpp
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

#ifndef KORDEX_BINDINGS_VERSION_HPP
#define KORDEX_BINDINGS_VERSION_HPP

#include <string_view>

namespace kordex::bindings
{
  /**
   * @brief Kordex bindings semantic version numbers.
   */
  inline constexpr int KORDEX_BINDINGS_VERSION_MAJOR = 0;
  inline constexpr int KORDEX_BINDINGS_VERSION_MINOR = 1;
  inline constexpr int KORDEX_BINDINGS_VERSION_PATCH = 0;

  /**
   * @brief Kordex bindings version as a stable string.
   */
  inline constexpr std::string_view KORDEX_BINDINGS_VERSION = "0.1.0";

  /**
   * @brief Return the Kordex bindings major version.
   */
  [[nodiscard]] constexpr int version_major() noexcept
  {
    return KORDEX_BINDINGS_VERSION_MAJOR;
  }

  /**
   * @brief Return the Kordex bindings minor version.
   */
  [[nodiscard]] constexpr int version_minor() noexcept
  {
    return KORDEX_BINDINGS_VERSION_MINOR;
  }

  /**
   * @brief Return the Kordex bindings patch version.
   */
  [[nodiscard]] constexpr int version_patch() noexcept
  {
    return KORDEX_BINDINGS_VERSION_PATCH;
  }

  /**
   * @brief Return the Kordex bindings version string.
   */
  [[nodiscard]] constexpr std::string_view version() noexcept
  {
    return KORDEX_BINDINGS_VERSION;
  }

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_VERSION_HPP
