/**
 *
 *  @file test_quickjs_backend.cpp
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

#include <kordex/bindings/BindingOptions.hpp>
#include <kordex/bindings/Engine.hpp>
#include <kordex/bindings/Value.hpp>

int main()
{
  using namespace kordex::bindings;

#if defined(KORDEX_BINDINGS_ENABLE_QUICKJS) && KORDEX_BINDINGS_ENABLE_QUICKJS

  BindingOptions options;
  options.backend = EngineBackend::QuickJS;
  options.engine_name = "quickjs-test";
  options.debug = true;
  options.diagnostics = true;

  auto engine_result = Engine::create(options);
  assert(engine_result);

  auto engine = std::move(engine_result.value());

  auto init_result = engine.initialize();
  assert(init_result.succeeded());
  assert(engine.running());

  auto result = engine.eval("1 + 2", "eval.js");
  assert(result.succeeded());
  assert(result.has_value());
  assert(result.value.is_number());

  auto number = result.value.as_number();
  assert(number);
  assert(number.value() == 3.0);

  auto string_result = engine.eval("'hello ' + 'kordex'", "string.js");
  assert(string_result.succeeded());
  assert(string_result.value.is_string());

  auto text = string_result.value.as_string();
  assert(text);
  assert(text.value() == "hello kordex");

  auto bool_result = engine.eval("10 > 2", "bool.js");
  assert(bool_result.succeeded());
  assert(bool_result.value.is_boolean());

  auto boolean = bool_result.value.as_boolean();
  assert(boolean);
  assert(boolean.value() == true);

  auto shutdown_result = engine.shutdown();
  assert(shutdown_result.succeeded());
  assert(!engine.running());

  std::cout << "QuickJS eval test passed\n";

#else

  std::cout << "QuickJS backend is disabled, skipping test\n";

#endif

  return 0;
}
