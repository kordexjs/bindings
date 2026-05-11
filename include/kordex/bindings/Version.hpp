/**
 *
 *  @file Version.hpp
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

#ifndef KORDEX_BINDINGS_VERSION_HPP
#define KORDEX_BINDINGS_VERSION_HPP

#include <string_view>

namespace kordex::bindings
{
  /**
   * @brief Kordex bindings semantic version numbers.
   */
  inline constexpr int version_major_value = 0;
  inline constexpr int version_minor_value = 1;
  inline constexpr int version_patch_value = 0;

  /**
   * @brief Kordex bindings version as a stable string.
   */
  inline constexpr std::string_view version_string = "0.1.0";

  /**
   * @brief Return the Kordex bindings major version.
   */
  [[nodiscard]] constexpr int version_major() noexcept
  {
    return version_major_value;
  }

  /**
   * @brief Return the Kordex bindings minor version.
   */
  [[nodiscard]] constexpr int version_minor() noexcept
  {
    return version_minor_value;
  }

  /**
   * @brief Return the Kordex bindings patch version.
   */
  [[nodiscard]] constexpr int version_patch() noexcept
  {
    return version_patch_value;
  }

  /**
   * @brief Return the Kordex bindings version string.
   */
  [[nodiscard]] constexpr std::string_view version() noexcept
  {
    return version_string;
  }

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_VERSION_HPP
