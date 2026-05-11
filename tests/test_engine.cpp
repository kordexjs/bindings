/**
 *
 *  @file test_engine.cpp
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

#include <string_view>
#include <kordex/bindings/Engine.hpp>

namespace
{
  [[nodiscard]] bool expect_true(
      bool condition,
      const char *message)
  {
    if (!condition)
    {
      (void)message;
      return false;
    }

    return true;
  }

  template <typename T>
  [[nodiscard]] bool result_ok(
      const kordex::bindings::Result<T> &result)
  {
    return !(!result);
  }

  [[nodiscard]] bool test_engine_state_strings()
  {
    return expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::EngineState::Created)) == "created",
               "created state string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::EngineState::Initializing)) == "initializing",
               "initializing state string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::EngineState::Running)) == "running",
               "running state string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::EngineState::Stopping)) == "stopping",
               "stopping state string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::EngineState::Stopped)) == "stopped",
               "stopped state string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::EngineState::Failed)) == "failed",
               "failed state string should match");
  }

  [[nodiscard]] bool test_engine_info_helpers()
  {
    kordex::bindings::EngineInfo info;
    info.name = "native";
    info.state = kordex::bindings::EngineState::Running;
    info.initialized = true;

    const bool running_ok =
        info.has_name() &&
        info.running() &&
        !info.stopped() &&
        !info.failed();

    info.state = kordex::bindings::EngineState::Stopped;

    const bool stopped_ok =
        info.stopped() &&
        !info.running() &&
        !info.failed();

    info.state = kordex::bindings::EngineState::Failed;

    return expect_true(
               running_ok,
               "engine info should report running state") &&
           expect_true(
               stopped_ok,
               "engine info should report stopped state") &&
           expect_true(
               info.failed(),
               "engine info should report failed state");
  }

  [[nodiscard]] bool test_default_engine()
  {
    kordex::bindings::Engine engine;

    return expect_true(
               !engine.initialized(),
               "default engine should not be initialized") &&
           expect_true(
               !engine.running(),
               "default engine should not be running") &&
           expect_true(
               engine.state() == kordex::bindings::EngineState::Created,
               "default engine state should be created") &&
           expect_true(
               engine.backend() == kordex::bindings::EngineBackend::Native,
               "default engine backend should be native") &&
           expect_true(
               std::string_view(engine.name()) == "native",
               "default engine name should come from default config") &&
           expect_true(
               engine.module_count() == 0,
               "default engine should have no modules") &&
           expect_true(
               engine.global_count() == 0,
               "default engine should have no globals") &&
           expect_true(
               engine.global_function_count() == 0,
               "default engine should have no global functions");
  }

  [[nodiscard]] bool test_create_engine()
  {
    auto engine_result = kordex::bindings::Engine::create();

    if (!expect_true(
            result_ok(engine_result),
            "engine creation should succeed"))
    {
      return false;
    }

    auto engine = engine_result.value();

    return expect_true(
               !engine.initialized(),
               "created engine should not initialize automatically") &&
           expect_true(
               engine.state() == kordex::bindings::EngineState::Created,
               "created engine state should be created") &&
           expect_true(
               engine.backend() == kordex::bindings::EngineBackend::Native,
               "created engine backend should be native") &&
           expect_true(
               std::string_view(engine.name()) == "native",
               "created engine name should match config");
  }

  [[nodiscard]] bool test_create_engine_from_options()
  {
    auto options = kordex::bindings::BindingOptions::development();

    auto engine_result = kordex::bindings::Engine::create(options);

    if (!expect_true(
            result_ok(engine_result),
            "engine creation from options should succeed"))
    {
      return false;
    }

    auto engine = engine_result.value();

    return expect_true(
               engine.config().debug,
               "development engine should enable debug") &&
           expect_true(
               engine.config().source_maps,
               "development engine should enable source maps") &&
           expect_true(
               std::string_view(engine.name()) == "native-dev",
               "development engine name should match");
  }

  [[nodiscard]] bool test_create_engine_from_config()
  {
    auto config_result = kordex::bindings::BindingConfig::from_options(
        kordex::bindings::BindingOptions::test());

    if (!expect_true(
            result_ok(config_result),
            "test config should be valid"))
    {
      return false;
    }

    auto engine_result = kordex::bindings::Engine::create(
        config_result.value());

    if (!expect_true(
            result_ok(engine_result),
            "engine creation from config should succeed"))
    {
      return false;
    }

    auto engine = engine_result.value();

    return expect_true(
               engine.config().has_stack_limit(),
               "test engine should have stack limit") &&
           expect_true(
               engine.config().has_heap_limit(),
               "test engine should have heap limit") &&
           expect_true(
               std::string_view(engine.name()) == "native-test",
               "test engine name should match");
  }

  [[nodiscard]] bool test_initialize_and_shutdown()
  {
    kordex::bindings::Engine engine;

    auto init_result = engine.initialize();
    const bool initialized = engine.initialized();
    const bool running = engine.running();

    auto second_init_result = engine.initialize();

    auto shutdown_result = engine.shutdown();
    const bool stopped = !engine.initialized() &&
                         !engine.running() &&
                         engine.state() == kordex::bindings::EngineState::Stopped;

    auto second_shutdown_result = engine.shutdown();

    return expect_true(
               init_result.succeeded(),
               "engine initialize should succeed") &&
           expect_true(
               initialized,
               "engine should be initialized") &&
           expect_true(
               running,
               "engine should be running") &&
           expect_true(
               second_init_result.succeeded(),
               "second initialize should succeed") &&
           expect_true(
               shutdown_result.succeeded(),
               "engine shutdown should succeed") &&
           expect_true(
               stopped,
               "engine should be stopped after shutdown") &&
           expect_true(
               second_shutdown_result.succeeded(),
               "second shutdown should succeed");
  }

  [[nodiscard]] bool test_initialize_invalid_engine_name()
  {
    kordex::bindings::Engine engine;
    engine.set_name("");

    auto result = engine.initialize();

    return expect_true(
               result.failed(),
               "initializing engine with empty name should fail") &&
           expect_true(
               result.error.has_error(),
               "failed initialization should contain error") &&
           expect_true(
               result.error.code() ==
                   kordex::bindings::ErrorCode::InternalError,
               "engine initialization failure should map to internal error") &&
           expect_true(
               engine.state() == kordex::bindings::EngineState::Failed,
               "engine should be marked failed") &&
           expect_true(
               !engine.initialized(),
               "invalid engine should not be initialized");
  }

  [[nodiscard]] bool test_set_name()
  {
    kordex::bindings::Engine engine;

    engine.set_name("main-engine");

    return expect_true(
               std::string_view(engine.name()) == "main-engine",
               "engine name should be updated") &&
           expect_true(
               std::string_view(engine.context().name()) == "main-engine",
               "context name should follow engine name");
  }

  [[nodiscard]] bool test_operations_require_running()
  {
    kordex::bindings::Engine engine;

    const auto set_global_error = engine.set_global(
        "answer",
        kordex::bindings::Value::number(42.0));

    auto global_result = engine.global("answer");

    auto function = kordex::bindings::Function::create(
        "answer",
        [](const kordex::bindings::FunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::number(42.0);
        });

    const auto function_error = engine.add_global_function(function);
    auto import_result = engine.import_module("missing");

    return expect_true(
               set_global_error.has_error(),
               "set global before initialize should fail") &&
           expect_true(
               !global_result,
               "get global before initialize should fail") &&
           expect_true(
               function_error.has_error(),
               "add function before initialize should fail") &&
           expect_true(
               !import_result,
               "import before initialize should fail") &&
           expect_true(
               set_global_error.code() ==
                   kordex::bindings::ErrorCode::NotFound,
               "engine unavailable should map to not found");
  }

  [[nodiscard]] bool test_global_values()
  {
    kordex::bindings::Engine engine;

    auto init_result = engine.initialize();

    const auto set_error = engine.set_global(
        "answer",
        kordex::bindings::Value::number(42.0));

    auto value = engine.global("answer");

    if (!expect_true(
            init_result.succeeded() && !set_error,
            "engine initialize and set global should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(value),
            "getting engine global should succeed"))
    {
      return false;
    }

    auto number = value.value().as_number();

    if (!expect_true(
            result_ok(number),
            "engine global value should convert to number"))
    {
      return false;
    }

    return expect_true(
               engine.global_count() == 1,
               "engine global count should be one") &&
           expect_true(
               number.value() == 42.0,
               "engine global value should be 42");
  }

  [[nodiscard]] bool test_global_functions()
  {
    kordex::bindings::Engine engine;

    auto init_result = engine.initialize();

    auto function = kordex::bindings::Function::create(
        "answer",
        [](const kordex::bindings::FunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::number(42.0);
        });

    const auto add_error = engine.add_global_function(function);
    auto call_result = engine.call_global_function("answer");

    if (!expect_true(
            init_result.succeeded() && !add_error,
            "engine initialize and add global function should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(call_result),
            "engine global function call should succeed"))
    {
      return false;
    }

    auto number = call_result.value().as_number();

    if (!expect_true(
            result_ok(number),
            "engine global function result should convert to number"))
    {
      return false;
    }

    return expect_true(
               engine.global_function_count() == 1,
               "engine global function count should be one") &&
           expect_true(
               number.value() == 42.0,
               "engine global function should return 42");
  }

  [[nodiscard]] bool test_set_global_function_explicit_name()
  {
    kordex::bindings::Engine engine;

    auto init_result = engine.initialize();

    auto function = kordex::bindings::Function::create(
        "internal_answer",
        [](const kordex::bindings::FunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::number(42.0);
        });

    const auto set_error = engine.set_global_function(
        "answer",
        function);

    auto call_result = engine.call_global_function("answer");

    if (!expect_true(
            init_result.succeeded() && !set_error,
            "engine initialize and set global function should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(call_result),
            "explicit global function call should succeed"))
    {
      return false;
    }

    return expect_true(
        engine.global_function_count() == 1,
        "engine should have one global function");
  }

  [[nodiscard]] bool test_register_and_import_module()
  {
    kordex::bindings::Engine engine;

    auto init_result = engine.initialize();

    auto module = kordex::bindings::Module::create(
        "main",
        kordex::bindings::ModuleKind::Script);

    const auto register_error = engine.register_module(module);
    auto imported_module = engine.import_module("main");

    if (!expect_true(
            init_result.succeeded() && !register_error,
            "engine initialize and module registration should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(imported_module),
            "engine module import should succeed"))
    {
      return false;
    }

    return expect_true(
               engine.module_count() == 1,
               "engine module count should be one") &&
           expect_true(
               std::string_view(imported_module.value().name()) == "main",
               "imported module name should match");
  }

  [[nodiscard]] bool test_register_native_module()
  {
    kordex::bindings::Engine engine;

    auto init_result = engine.initialize();

    auto native_module = kordex::bindings::NativeModule::create("console");

    const auto value_error = native_module.set_value(
        "version",
        kordex::bindings::Value::string("0.1.0"));

    const auto register_error = engine.register_native_module(native_module);
    auto imported_module = engine.import_module("console");

    if (!expect_true(
            init_result.succeeded() && !value_error && !register_error,
            "engine initialize and native module registration should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(imported_module),
            "engine native module import should succeed"))
    {
      return false;
    }

    return expect_true(
               engine.module_count() == 1,
               "engine module count should be one") &&
           expect_true(
               imported_module.value().kind() ==
                   kordex::bindings::ModuleKind::Native,
               "imported module should be native") &&
           expect_true(
               imported_module.value().has_export("version"),
               "imported native module should expose value");
  }

  [[nodiscard]] bool test_run_script()
  {
    kordex::bindings::Engine engine;

    auto init_result = engine.initialize();

    auto script = kordex::bindings::Script::from_source(
        "console.log('hello');",
        "hello.js");

    auto result = engine.run_script(script);

    if (!expect_true(
            init_result.succeeded(),
            "engine initialize should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result.succeeded(),
            "engine run script should succeed"))
    {
      return false;
    }

    auto value = result.value.as_string();

    if (!expect_true(
            result_ok(value),
            "engine run script value should convert to string"))
    {
      return false;
    }

    return expect_true(
               value.value() == "console.log('hello');",
               "engine run script should return source placeholder") &&
           expect_true(
               std::string_view(result.output) ==
                   "script executed by native placeholder",
               "engine run script output should match");
  }

  [[nodiscard]] bool test_run_script_requires_running()
  {
    kordex::bindings::Engine engine;

    auto script = kordex::bindings::Script::from_source(
        "console.log('hello');",
        "hello.js");

    auto result = engine.run_script(script);

    return expect_true(
               result.failed(),
               "engine run script before initialize should fail") &&
           expect_true(
               result.error.has_error(),
               "engine run script failure should contain error") &&
           expect_true(
               result.error.code() ==
                   kordex::bindings::ErrorCode::NotFound,
               "engine unavailable should map to not found");
  }

  [[nodiscard]] bool test_eval()
  {
    kordex::bindings::Engine engine;

    auto init_result = engine.initialize();

    auto result = engine.eval(
        "1 + 1",
        "eval.js");

    if (!expect_true(
            init_result.succeeded(),
            "engine initialize should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result.succeeded(),
            "engine eval should succeed"))
    {
      return false;
    }

    auto value = result.value.as_string();

    if (!expect_true(
            result_ok(value),
            "engine eval value should convert to string"))
    {
      return false;
    }

    return expect_true(
        value.value() == "1 + 1",
        "engine eval placeholder should return source");
  }

  [[nodiscard]] bool test_clear()
  {
    kordex::bindings::Engine engine;

    auto init_result = engine.initialize();

    const auto global_error = engine.set_global(
        "a",
        kordex::bindings::Value::number(1.0));

    auto function = kordex::bindings::Function::create(
        "b",
        [](const kordex::bindings::FunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::undefined();
        });

    const auto function_error = engine.add_global_function(function);

    auto module = kordex::bindings::Module::create(
        "c",
        kordex::bindings::ModuleKind::Script);

    const auto module_error = engine.register_module(module);

    const bool setup_ok =
        init_result.succeeded() &&
        !global_error &&
        !function_error &&
        !module_error &&
        engine.global_count() == 1 &&
        engine.global_function_count() == 1 &&
        engine.module_count() == 1;

    engine.clear();

    return expect_true(
               setup_ok,
               "engine setup before clear should succeed") &&
           expect_true(
               engine.global_count() == 0,
               "engine globals should be empty after clear") &&
           expect_true(
               engine.global_function_count() == 0,
               "engine global functions should be empty after clear") &&
           expect_true(
               engine.module_count() == 0,
               "engine modules should be empty after clear");
  }

  [[nodiscard]] bool test_shutdown_blocks_operations_but_preserves_context_data()
  {
    kordex::bindings::Engine engine;

    auto init_result = engine.initialize();

    const auto set_error = engine.set_global(
        "answer",
        kordex::bindings::Value::number(42.0));

    auto shutdown_result = engine.shutdown();

    auto value = engine.global("answer");

    return expect_true(
               init_result.succeeded() &&
                   !set_error &&
                   shutdown_result.succeeded(),
               "engine init, set global, and shutdown should succeed") &&
           expect_true(
               !engine.initialized(),
               "engine should not be initialized after shutdown") &&
           expect_true(
               !engine.running(),
               "engine should not be running after shutdown") &&
           expect_true(
               engine.state() == kordex::bindings::EngineState::Stopped,
               "engine state should be stopped after shutdown") &&
           expect_true(
               !value,
               "engine global access after shutdown should fail") &&
           expect_true(
               value.error().code() ==
                   kordex::bindings::ErrorCode::NotFound,
               "engine access after shutdown should map to not found");
  }

  [[nodiscard]] bool test_reinitialize_resets_context()
  {
    kordex::bindings::Engine engine;

    auto init_result = engine.initialize();

    const auto set_error = engine.set_global(
        "answer",
        kordex::bindings::Value::number(42.0));

    auto shutdown_result = engine.shutdown();
    auto reinit_result = engine.initialize();

    auto value = engine.global("answer");

    return expect_true(
               init_result.succeeded() &&
                   !set_error &&
                   shutdown_result.succeeded() &&
                   reinit_result.succeeded(),
               "engine init, shutdown, and reinitialize should succeed") &&
           expect_true(
               engine.initialized(),
               "engine should be initialized after reinitialize") &&
           expect_true(
               engine.running(),
               "engine should be running after reinitialize") &&
           expect_true(
               engine.global_count() == 0,
               "reinitialize should reset default context") &&
           expect_true(
               !value,
               "old global should not exist after reinitialize");
  }

} // namespace

int main()
{
  const bool ok =
      test_engine_state_strings() &&
      test_engine_info_helpers() &&
      test_default_engine() &&
      test_create_engine() &&
      test_create_engine_from_options() &&
      test_create_engine_from_config() &&
      test_initialize_and_shutdown() &&
      test_initialize_invalid_engine_name() &&
      test_set_name() &&
      test_operations_require_running() &&
      test_global_values() &&
      test_global_functions() &&
      test_set_global_function_explicit_name() &&
      test_register_and_import_module() &&
      test_register_native_module() &&
      test_run_script() &&
      test_run_script_requires_running() &&
      test_eval() &&
      test_clear() &&
      test_shutdown_blocks_operations_but_preserves_context_data() &&
      test_reinitialize_resets_context();

  return ok ? 0 : 1;
}
