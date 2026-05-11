/**
 *
 *  @file TypeScriptLoader.hpp
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

#ifndef KORDEX_BINDINGS_TYPESCRIPT_LOADER_HPP
#define KORDEX_BINDINGS_TYPESCRIPT_LOADER_HPP

#include <cstddef>
#include <string>
#include <vector>

#include <kordex/bindings/Error.hpp>
#include <kordex/bindings/Result.hpp>
#include <kordex/bindings/Script.hpp>

namespace kordex::bindings
{
  /**
   * @enum TypeScriptDiagnosticLevel
   * @brief Severity level for TypeScript loader diagnostics.
   */
  enum class TypeScriptDiagnosticLevel
  {
    Info,
    Warning,
    Error
  };

  /**
   * @struct TypeScriptDiagnostic
   * @brief Diagnostic produced while checking or transforming TypeScript.
   */
  struct TypeScriptDiagnostic
  {
    TypeScriptDiagnosticLevel level{TypeScriptDiagnosticLevel::Info};

    std::string message{};
    std::string path{};

    std::size_t line{0};
    std::size_t column{0};

    [[nodiscard]] bool has_message() const noexcept;
    [[nodiscard]] bool has_location() const noexcept;
    [[nodiscard]] bool is_error() const noexcept;
    [[nodiscard]] bool is_warning() const noexcept;
  };

  /**
   * @struct TypeScriptLoaderOptions
   * @brief Options controlling TypeScript loading and transpilation.
   */
  struct TypeScriptLoaderOptions
  {
    /**
     * @brief Whether basic checks should be performed before transpilation.
     */
    bool check{true};

    /**
     * @brief Whether type syntax should be stripped.
     */
    bool strip_types{true};

    /**
     * @brief Whether unsupported syntax should be reported as warnings.
     */
    bool warn_unsupported{true};

    /**
     * @brief Whether the transformed script should be marked JavaScript.
     */
    bool emit_javascript{true};
  };

  /**
   * @struct TypeScriptLoadResult
   * @brief Result of transforming TypeScript to JavaScript.
   */
  struct TypeScriptLoadResult
  {
    Script script{};
    std::vector<TypeScriptDiagnostic> diagnostics{};
    bool transformed{false};

    [[nodiscard]] bool has_diagnostics() const noexcept;
    [[nodiscard]] bool has_errors() const noexcept;
    [[nodiscard]] bool has_warnings() const noexcept;
  };

  /**
   * @class TypeScriptLoader
   * @brief Minimal TypeScript loading/checking/transpilation layer.
   *
   * This is an MVP transformer. It strips common TypeScript-only syntax
   * before sending code to the JavaScript engine. A full TypeScript compiler
   * can later replace this implementation behind the same API.
   */
  class TypeScriptLoader
  {
  public:
    explicit TypeScriptLoader(
        TypeScriptLoaderOptions options = {});

    [[nodiscard]] const TypeScriptLoaderOptions &options() const noexcept;

    /**
     * @brief Return true if the script is TypeScript.
     */
    [[nodiscard]] static bool is_typescript(
        const Script &script) noexcept;

    /**
     * @brief Load a script from disk and transform it if it is TypeScript.
     */
    [[nodiscard]] Result<TypeScriptLoadResult> load(
        const std::string &path) const;

    /**
     * @brief Check a TypeScript script.
     */
    [[nodiscard]] std::vector<TypeScriptDiagnostic> check(
        const Script &script) const;

    /**
     * @brief Transform TypeScript to JavaScript.
     */
    [[nodiscard]] Result<TypeScriptLoadResult> transpile(
        Script script) const;

    /**
     * @brief Transform TypeScript source text to JavaScript text.
     */
    [[nodiscard]] Result<std::string> transpile_source(
        const std::string &source,
        const std::string &path = {}) const;

  private:
    [[nodiscard]] Error validate_script(
        const Script &script) const;

    TypeScriptLoaderOptions options_{};
  };

  /**
   * @brief Convert TypeScriptDiagnosticLevel to a stable string.
   */
  [[nodiscard]] const char *to_string(
      TypeScriptDiagnosticLevel level) noexcept;

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_TYPESCRIPT_LOADER_HPP
