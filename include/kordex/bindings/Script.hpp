/**
 *
 *  @file Script.hpp
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

#ifndef KORDEX_BINDINGS_SCRIPT_HPP
#define KORDEX_BINDINGS_SCRIPT_HPP

#include <cstddef>
#include <string>
#include <string_view>

#include <kordex/runtime/SourceFile.hpp>

#include <kordex/bindings/Error.hpp>
#include <kordex/bindings/Result.hpp>
#include <kordex/bindings/ScriptResult.hpp>
#include <kordex/bindings/Value.hpp>

namespace kordex::bindings
{
  /**
   * @enum ScriptType
   * @brief Type of script handled by the bindings layer.
   */
  enum class ScriptType
  {
    Unknown,
    JavaScript,
    TypeScript,
    Json
  };

  /**
   * @struct ScriptInfo
   * @brief Metadata describing a script.
   */
  struct ScriptInfo
  {
    /**
     * @brief Original script path.
     */
    std::string path{};

    /**
     * @brief Optional script name.
     */
    std::string name{};

    /**
     * @brief Script type.
     */
    ScriptType type{ScriptType::Unknown};

    /**
     * @brief Whether the script was loaded from disk.
     */
    bool loaded_from_file{false};

    /**
     * @brief Whether the script was compiled.
     */
    bool compiled{false};

    /**
     * @brief Whether the script was executed.
     */
    bool executed{false};

    /**
     * @brief Return true if a script path is available.
     */
    [[nodiscard]] bool has_path() const noexcept;

    /**
     * @brief Return true if a script name is available.
     */
    [[nodiscard]] bool has_name() const noexcept;

    /**
     * @brief Return true if the script type is executable.
     */
    [[nodiscard]] bool executable() const noexcept;
  };

  /**
   * @class Script
   * @brief Engine-independent script representation.
   *
   * Script owns source text and metadata. It does not execute JavaScript by
   * itself. Execution is performed later by Engine or EngineContext.
   */
  class Script
  {
  public:
    /**
     * @brief Construct an empty invalid script.
     */
    Script() = default;

    /**
     * @brief Construct a script from source and metadata.
     */
    Script(std::string source, ScriptInfo info = {});

    /**
     * @brief Create a script from source code.
     */
    [[nodiscard]] static Script from_source(
        std::string source,
        std::string name = "anonymous");

    /**
     * @brief Create a script from source code and metadata.
     */
    [[nodiscard]] static Script from_source(
        std::string source,
        ScriptInfo info);

    /**
     * @brief Create a script from a runtime SourceFile.
     */
    [[nodiscard]] static Result<Script> from_source_file(
        const kordex::runtime::SourceFile &source_file);

    /**
     * @brief Load a script from disk using kordex::runtime::SourceFile.
     */
    [[nodiscard]] static Result<Script> load(
        const std::string &path);

    /**
     * @brief Return script metadata.
     */
    [[nodiscard]] const ScriptInfo &info() const noexcept;

    /**
     * @brief Return script source.
     */
    [[nodiscard]] const std::string &source() const noexcept;

    /**
     * @brief Return script path.
     */
    [[nodiscard]] const std::string &path() const noexcept;

    /**
     * @brief Return script name.
     */
    [[nodiscard]] const std::string &name() const noexcept;

    /**
     * @brief Return script type.
     */
    [[nodiscard]] ScriptType type() const noexcept;

    /**
     * @brief Return true if the script has non-empty source.
     */
    [[nodiscard]] bool valid() const noexcept;

    /**
     * @brief Return true if the script source is empty.
     */
    [[nodiscard]] bool empty() const noexcept;

    /**
     * @brief Return source size in bytes.
     */
    [[nodiscard]] std::size_t size() const noexcept;

    /**
     * @brief Return true if the script was loaded from disk.
     */
    [[nodiscard]] bool loaded_from_file() const noexcept;

    /**
     * @brief Return true if the script was compiled.
     */
    [[nodiscard]] bool compiled() const noexcept;

    /**
     * @brief Return true if the script was executed.
     */
    [[nodiscard]] bool executed() const noexcept;

    /**
     * @brief Return true if the script can be executed.
     */
    [[nodiscard]] bool executable() const noexcept;

    /**
     * @brief Update script source.
     */
    void set_source(std::string source);

    /**
     * @brief Update script path.
     */
    void set_path(std::string path);

    /**
     * @brief Update script name.
     */
    void set_name(std::string name);

    /**
     * @brief Update script type.
     */
    void set_type(ScriptType type) noexcept;

    /**
     * @brief Mark this script as compiled.
     */
    void mark_compiled() noexcept;

    /**
     * @brief Mark this script as not compiled.
     */
    void mark_uncompiled() noexcept;

    /**
     * @brief Mark this script as executed.
     */
    void mark_executed() noexcept;

    /**
     * @brief Mark this script as not executed.
     */
    void mark_unexecuted() noexcept;

    /**
     * @brief Validate script metadata and source.
     */
    [[nodiscard]] Error validate() const;

    /**
     * @brief Perform a lightweight compile validation.
     *
     * This does not invoke a JavaScript engine. It only validates source and
     * marks the script as compiled when valid.
     */
    [[nodiscard]] ScriptResult compile();

    /**
     * @brief Perform a lightweight execution placeholder.
     *
     * Real JavaScript execution is provided by Engine/EngineContext later.
     * For now this validates the script and returns the source as a value.
     */
    [[nodiscard]] ScriptResult run();

  private:
    std::string source_{};
    ScriptInfo info_{};
  };

  /**
   * @brief Detect script type from a path.
   */
  [[nodiscard]] ScriptType detect_script_type(
      std::string_view path) noexcept;

  /**
   * @brief Convert runtime source type to bindings script type.
   */
  [[nodiscard]] ScriptType from_runtime_source_type(
      kordex::runtime::SourceType type) noexcept;

  /**
   * @brief Convert ScriptType to a stable string.
   */
  [[nodiscard]] const char *to_string(ScriptType type) noexcept;

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_SCRIPT_HPP
