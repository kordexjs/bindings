/**
 *
 *  @file NativeBackendDriver.cpp
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

#include <utility>

#include <kordex/bindings/backend/NativeBackendDriver.hpp>

namespace kordex::bindings
{
  BindingResult NativeBackendDriver::initialize(
      EngineContext &context)
  {
    (void)context;

    return BindingResult::success("native backend initialized");
  }

  BindingResult NativeBackendDriver::shutdown(
      EngineContext &context)
  {
    (void)context;

    return BindingResult::success("native backend stopped");
  }

  ScriptResult NativeBackendDriver::run_script(
      EngineContext &context,
      Script script)
  {
    if (!context.initialized())
    {
      return ScriptResult::failure(
          make_binding_error(
              BindingErrorCode::ContextUnavailable,
              "engine context is not initialized"),
          1);
    }

    return script.run();
  }

  ScriptResult NativeBackendDriver::eval(
      EngineContext &context,
      std::string source,
      std::string name)
  {
    Script script = Script::from_source(
        std::move(source),
        std::move(name));

    return run_script(context, std::move(script));
  }

} // namespace kordex::bindings
