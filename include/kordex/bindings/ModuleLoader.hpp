/**
 *
 *  @file ModuleLoader.hpp
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

#ifndef KORDEX_BINDINGS_MODULE_LOADER_HPP
#define KORDEX_BINDINGS_MODULE_LOADER_HPP

#include <map>
#include <string>
#include <vector>

#include <kordex/runtime/ModuleResolver.hpp>

#include <kordex/bindings/Error.hpp>
#include <kordex/bindings/Result.hpp>
#include <kordex/bindings/Script.hpp>

namespace kordex::bindings
{
  /**
   * @struct ModuleLoaderOptions
   * @brief Options controlling script module loading.
   */
  struct ModuleLoaderOptions
  {
    bool allow_relative{true};
    bool allow_absolute{true};
    bool allow_json{true};
    bool allow_typescript{true};

    bool allow_packages{false};
    bool allow_builtins{false};

    bool cache_enabled{true};
  };

  /**
   * @struct ModuleLoaderReport
   * @brief Metadata produced by the module loader.
   */
  struct ModuleLoaderReport
  {
    std::string entry_path{};
    std::vector<std::string> loaded_modules{};
    bool bundled{false};
    bool used_cache{false};

    [[nodiscard]] bool has_loaded_modules() const noexcept;
  };

  /**
   * @struct ModuleLoaderResult
   * @brief Result of loading and transforming a module graph.
   */
  struct ModuleLoaderResult
  {
    Script script{};
    ModuleLoaderReport report{};
  };

  /**
   * @class ModuleLoader
   * @brief Minimal static import loader for QuickJS execution.
   *
   * ModuleLoader resolves relative JavaScript, TypeScript, and JSON modules
   * through kordex::runtime::ModuleResolver, then emits a small CommonJS-like
   * bundle that QuickJS can execute with JS_Eval.
   */
  class ModuleLoader
  {
  public:
    explicit ModuleLoader(
        ModuleLoaderOptions options = {});

    [[nodiscard]] const ModuleLoaderOptions &options() const noexcept;

    /**
     * @brief Load an entry script and transform its static imports if needed.
     */
    [[nodiscard]] Result<ModuleLoaderResult> load_entry(
        Script entry);

    /**
     * @brief Clear loaded module cache.
     */
    void clear_cache();

    /**
     * @brief Return number of cached modules.
     */
    [[nodiscard]] std::size_t cache_size() const noexcept;

    /**
     * @brief Return true if the source contains static import or export syntax.
     */
    [[nodiscard]] static bool has_module_syntax(
        const std::string &source);

  private:
    struct LoadedModule
    {
      std::string id{};
      std::string path{};
      Script script{};
      std::string transformed_source{};
      std::vector<std::string> dependencies{};
      bool json{false};
    };

    [[nodiscard]] Result<Script> normalize_entry(
        Script entry) const;

    [[nodiscard]] Result<LoadedModule> load_module_graph(
        const Script &script,
        const std::string &requested_id,
        ModuleLoaderReport &report);

    [[nodiscard]] Result<LoadedModule> load_dependency(
        const std::string &specifier,
        const LoadedModule &parent,
        ModuleLoaderReport &report);

    [[nodiscard]] Result<Script> load_script_from_path(
        const std::string &path) const;

    [[nodiscard]] Result<std::string> resolve_dependency_path(
        const std::string &specifier,
        const LoadedModule &parent) const;

    [[nodiscard]] Result<std::string> transform_module_source(
        const LoadedModule &module,
        const std::map<std::string, LoadedModule> &modules) const;

    [[nodiscard]] Result<std::string> build_bundle(
        const LoadedModule &entry,
        const std::map<std::string, LoadedModule> &modules) const;

    [[nodiscard]] std::string make_module_id(
        const std::string &path) const;

    [[nodiscard]] static std::string parent_directory(
        const std::string &path);

    [[nodiscard]] static std::string json_escape(
        const std::string &value);

    [[nodiscard]] static std::string js_string(
        const std::string &value);

    ModuleLoaderOptions options_{};
    std::map<std::string, LoadedModule> cache_{};
  };

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_MODULE_LOADER_HPP
