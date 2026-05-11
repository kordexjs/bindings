/**
 *
 *  @file EngineBackendDriver.hpp
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

#ifndef KORDEX_BINDINGS_BACKEND_ENGINE_BACKEND_DRIVER_HPP
#define KORDEX_BINDINGS_BACKEND_ENGINE_BACKEND_DRIVER_HPP

#include <string>

#include <kordex/bindings/BindingResult.hpp>
#include <kordex/bindings/EngineContext.hpp>
#include <kordex/bindings/Script.hpp>
#include <kordex/bindings/ScriptResult.hpp>

namespace kordex::bindings
{
  /**
   * @class EngineBackendDriver
   * @brief Internal backend interface used by Engine.
   *
   * EngineBackendDriver isolates concrete JavaScript engines from the public
   * Engine API. Native, QuickJS, and V8 backends must implement this interface.
   */
  class EngineBackendDriver
  {
  public:
    virtual ~EngineBackendDriver() = default;

    EngineBackendDriver() = default;

    EngineBackendDriver(const EngineBackendDriver &) = delete;
    EngineBackendDriver &operator=(const EngineBackendDriver &) = delete;

    EngineBackendDriver(EngineBackendDriver &&) noexcept = delete;
    EngineBackendDriver &operator=(EngineBackendDriver &&) noexcept = delete;

    /**
     * @brief Initialize the backend for the provided context.
     */
    [[nodiscard]] virtual BindingResult initialize(
        EngineContext &context) = 0;

    /**
     * @brief Shutdown the backend for the provided context.
     */
    [[nodiscard]] virtual BindingResult shutdown(
        EngineContext &context) = 0;

    /**
     * @brief Execute a script through the backend.
     */
    [[nodiscard]] virtual ScriptResult run_script(
        EngineContext &context,
        Script script) = 0;

    /**
     * @brief Evaluate source code through the backend.
     */
    [[nodiscard]] virtual ScriptResult eval(
        EngineContext &context,
        std::string source,
        std::string name) = 0;
  };

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_BACKEND_ENGINE_BACKEND_DRIVER_HPP
