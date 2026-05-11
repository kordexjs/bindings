/**
 *
 *  @file test_quickjs_errors.cpp
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

#include <cassert>
#include <iostream>
#include <string>
#include <utility>

#include <kordex/bindings/BindingOptions.hpp>
#include <kordex/bindings/Engine.hpp>

int main()
{
  using namespace kordex::bindings;

#if defined(KORDEX_BINDINGS_ENABLE_QUICKJS) && KORDEX_BINDINGS_ENABLE_QUICKJS

  BindingOptions options;
  options.backend = EngineBackend::QuickJS;
  options.engine_name = "quickjs-errors-test";
  options.debug = true;
  options.diagnostics = true;

  auto engine_result = Engine::create(options);
  assert(engine_result);

  auto engine = std::move(engine_result.value());

  auto init = engine.initialize();
  assert(init.succeeded());

  auto error_result = engine.eval(
      "function fail() { throw new Error('boom'); } fail();",
      "error.js");

  assert(error_result.failed());
  assert(error_result.error.has_error());

  const std::string error_message(error_result.error.message());
  assert(error_message.find("Error") != std::string::npos);
  assert(error_message.find("boom") != std::string::npos);

  auto reference_error = engine.eval(
      "missingVariable + 1",
      "reference-error.js");

  assert(reference_error.failed());
  assert(reference_error.error.has_error());

  const std::string reference_message(reference_error.error.message());
  assert(reference_message.find("ReferenceError") != std::string::npos ||
         reference_message.find("missingVariable") != std::string::npos);

  auto thrown_string = engine.eval(
      "throw 'plain boom'",
      "throw-string.js");

  assert(thrown_string.failed());
  assert(thrown_string.error.has_error());

  const std::string thrown_message(thrown_string.error.message());
  assert(thrown_message.find("plain boom") != std::string::npos);

  auto shutdown = engine.shutdown();
  assert(shutdown.succeeded());

  std::cout << "QuickJS error reporting test passed\n";

#else

  std::cout << "QuickJS backend is disabled, skipping error reporting test\n";

#endif

  return 0;
}
