/**
 *
 *  @file NativeBackendDriver.hpp
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

#ifndef KORDEX_BINDINGS_BACKEND_NATIVE_BACKEND_DRIVER_HPP
#define KORDEX_BINDINGS_BACKEND_NATIVE_BACKEND_DRIVER_HPP

#include <kordex/bindings/backend/EngineBackendDriver.hpp>

namespace kordex::bindings
{
  /**
   * @class NativeBackendDriver
   * @brief Built-in placeholder backend.
   *
   * NativeBackendDriver preserves the current placeholder execution behavior.
   * It exists so Engine can use the same backend-driver path before QuickJS
   * or V8 is connected.
   */
  class NativeBackendDriver final : public EngineBackendDriver
  {
  public:
    [[nodiscard]] BindingResult initialize(
        EngineContext &context) override;

    [[nodiscard]] BindingResult shutdown(
        EngineContext &context) override;

    [[nodiscard]] ScriptResult run_script(
        EngineContext &context,
        Script script) override;

    [[nodiscard]] ScriptResult eval(
        EngineContext &context,
        std::string source,
        std::string name) override;
  };

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_BACKEND_NATIVE_BACKEND_DRIVER_HPP
