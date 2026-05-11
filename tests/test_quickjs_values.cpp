/**
 *
 *  @file test_quickjs_values.cpp
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
#include <utility>

#include <kordex/bindings/BindingOptions.hpp>
#include <kordex/bindings/Engine.hpp>

namespace
{
#if defined(KORDEX_BINDINGS_ENABLE_QUICKJS) && KORDEX_BINDINGS_ENABLE_QUICKJS

  kordex::bindings::Engine make_engine()
  {
    using namespace kordex::bindings;

    BindingOptions options;
    options.backend = EngineBackend::QuickJS;
    options.engine_name = "quickjs-values-test";
    options.debug = true;
    options.diagnostics = true;

    auto engine_result = Engine::create(options);
    assert(engine_result);

    auto engine = std::move(engine_result.value());

    auto init = engine.initialize();
    assert(init.succeeded());

    return engine;
  }

#endif
} // namespace

int main()
{
  using namespace kordex::bindings;

#if defined(KORDEX_BINDINGS_ENABLE_QUICKJS) && KORDEX_BINDINGS_ENABLE_QUICKJS

  auto engine = make_engine();

  {
    auto result = engine.eval("undefined", "undefined.js");
    assert(result.succeeded());
    assert(result.value.is_undefined());
  }

  {
    auto result = engine.eval("null", "null.js");
    assert(result.succeeded());
    assert(result.value.is_null());
  }

  {
    auto result = engine.eval("true", "boolean.js");
    assert(result.succeeded());
    assert(result.value.is_boolean());

    auto value = result.value.as_boolean();
    assert(value);
    assert(value.value() == true);
  }

  {
    auto result = engine.eval("21 * 2", "number.js");
    assert(result.succeeded());
    assert(result.value.is_number());

    auto value = result.value.as_number();
    assert(value);
    assert(value.value() == 42.0);
  }

  {
    auto result = engine.eval("'hello ' + 'kordex'", "string.js");
    assert(result.succeeded());
    assert(result.value.is_string());

    auto value = result.value.as_string();
    assert(value);
    assert(value.value() == "hello kordex");
  }

  {
    auto result = engine.eval("({ name: 'kordex', ok: true })", "object.js");
    assert(result.succeeded());
    assert(result.value.is_string());

    auto value = result.value.as_string();
    assert(value);
    assert(value.value().find("kordex") != std::string::npos);
  }

  {
    auto result = engine.eval("[1, 2, 3]", "array.js");
    assert(result.succeeded());
    assert(result.value.is_string());

    auto value = result.value.as_string();
    assert(value);
    assert(value.value().find("1") != std::string::npos);
  }

  {
    auto result = engine.eval("throw new Error('boom')", "error.js");
    assert(result.failed());
    assert(result.error.has_error());
  }

  auto shutdown = engine.shutdown();
  assert(shutdown.succeeded());

  std::cout << "QuickJS value conversion test passed\n";

#else

  std::cout << "QuickJS backend is disabled, skipping value conversion test\n";

#endif

  return 0;
}
