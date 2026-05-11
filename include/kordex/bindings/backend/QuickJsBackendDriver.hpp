/**
 *
 *  @file QuickJsBackendDriver.hpp
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

#ifndef KORDEX_BINDINGS_BACKEND_QUICKJS_BACKEND_DRIVER_HPP
#define KORDEX_BINDINGS_BACKEND_QUICKJS_BACKEND_DRIVER_HPP

#include <kordex/bindings/backend/EngineBackendDriver.hpp>

namespace kordex::bindings
{
  /**
   * @class QuickJsBackendDriver
   * @brief QuickJS-backed JavaScript engine driver.
   *
   * This driver owns a QuickJS runtime and context when QuickJS support is
   * enabled at build time.
   */
  class QuickJsBackendDriver final : public EngineBackendDriver
  {
  public:
    QuickJsBackendDriver();

    QuickJsBackendDriver(const QuickJsBackendDriver &) = delete;
    QuickJsBackendDriver &operator=(const QuickJsBackendDriver &) = delete;

    QuickJsBackendDriver(QuickJsBackendDriver &&) noexcept = delete;
    QuickJsBackendDriver &operator=(QuickJsBackendDriver &&) noexcept = delete;

    ~QuickJsBackendDriver() override;

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

  private:
    [[nodiscard]] bool initialized() const noexcept;

    void *runtime_{nullptr};
    void *context_{nullptr};
  };

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_BACKEND_QUICKJS_BACKEND_DRIVER_HPP
