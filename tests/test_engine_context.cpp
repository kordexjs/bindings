/**
 *
 *  @file test_engine_context.cpp
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
#include <kordex/bindings/EngineContext.hpp>

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

  [[nodiscard]] bool test_context_info_helpers()
  {
    kordex::bindings::EngineContextInfo info;
    info.name = "main";
    info.id = "ctx-1";

    return expect_true(
               info.has_name(),
               "context info should have name") &&
           expect_true(
               info.has_id(),
               "context info should have id");
  }

  [[nodiscard]] bool test_default_context()
  {
    kordex::bindings::EngineContext context;

    return expect_true(
               !context.initialized(),
               "default context should not be initialized") &&
           expect_true(
               std::string_view(context.name()) == "default",
               "default context name should be default") &&
           expect_true(
               context.global_count() == 0,
               "default context should have no globals") &&
           expect_true(
               context.global_function_count() == 0,
               "default context should have no global functions") &&
           expect_true(
               context.registry().empty(),
               "default context registry should be empty") &&
           expect_true(
               std::string_view(context.globals().name()) == "global",
               "global object name should be global");
  }

  [[nodiscard]] bool test_create_context()
  {
    auto context_result = kordex::bindings::EngineContext::create();

    if (!expect_true(
            result_ok(context_result),
            "context creation should succeed"))
    {
      return false;
    }

    auto context = context_result.value();

    return expect_true(
               !context.initialized(),
               "created context should not initialize automatically") &&
           expect_true(
               std::string_view(context.name()) == "default",
               "created context name should be default") &&
           expect_true(
               context.config().backend ==
                   kordex::bindings::EngineBackend::Native,
               "created context backend should be native");
  }

  [[nodiscard]] bool test_create_context_from_options()
  {
    auto options = kordex::bindings::BindingOptions::development();

    auto context_result = kordex::bindings::EngineContext::create(options);

    if (!expect_true(
            result_ok(context_result),
            "context creation from options should succeed"))
    {
      return false;
    }

    auto context = context_result.value();

    return expect_true(
               context.config().debug,
               "development context should enable debug") &&
           expect_true(
               context.config().source_maps,
               "development context should enable source maps") &&
           expect_true(
               std::string_view(context.config().engine_name) ==
                   "native-dev",
               "development context engine name should match");
  }

  [[nodiscard]] bool test_create_context_from_config()
  {
    auto config_result = kordex::bindings::BindingConfig::from_options(
        kordex::bindings::BindingOptions::test());

    if (!expect_true(
            result_ok(config_result),
            "test config should be valid"))
    {
      return false;
    }

    auto context_result = kordex::bindings::EngineContext::create(
        config_result.value());

    if (!expect_true(
            result_ok(context_result),
            "context creation from config should succeed"))
    {
      return false;
    }

    auto context = context_result.value();

    return expect_true(
               context.config().has_stack_limit(),
               "test context should have stack limit") &&
           expect_true(
               context.config().has_heap_limit(),
               "test context should have heap limit") &&
           expect_true(
               std::string_view(context.config().engine_name) ==
                   "native-test",
               "test context engine name should match");
  }

  [[nodiscard]] bool test_initialize_and_shutdown()
  {
    kordex::bindings::EngineContext context;

    const auto init_error = context.initialize();
    const bool initialized = context.initialized();

    const auto second_init_error = context.initialize();

    const auto shutdown_error = context.shutdown();
    const bool stopped = !context.initialized();

    const auto second_shutdown_error = context.shutdown();

    return expect_true(
               !init_error,
               "context initialize should succeed") &&
           expect_true(
               initialized,
               "context should be initialized") &&
           expect_true(
               !second_init_error,
               "second initialize should be idempotent") &&
           expect_true(
               !shutdown_error,
               "context shutdown should succeed") &&
           expect_true(
               stopped,
               "context should be stopped after shutdown") &&
           expect_true(
               !second_shutdown_error,
               "second shutdown should be idempotent");
  }

  [[nodiscard]] bool test_initialize_invalid_context_name()
  {
    kordex::bindings::EngineContext context;
    context.set_name("");

    const auto error = context.initialize();

    return expect_true(
               error.has_error(),
               "initializing context with empty name should fail") &&
           expect_true(
               error.code() ==
                   kordex::bindings::ErrorCode::ConfigError,
               "empty context name should map to config error") &&
           expect_true(
               !context.initialized(),
               "invalid context should not be initialized");
  }

  [[nodiscard]] bool test_name_and_id_setters()
  {
    kordex::bindings::EngineContext context;

    context.set_name("main");
    context.set_id("ctx-main");

    return expect_true(
               std::string_view(context.name()) == "main",
               "context name should be updated") &&
           expect_true(
               std::string_view(context.id()) == "ctx-main",
               "context id should be updated") &&
           expect_true(
               context.info().has_name(),
               "context info should have name") &&
           expect_true(
               context.info().has_id(),
               "context info should have id");
  }

  [[nodiscard]] bool test_operations_require_initialization()
  {
    kordex::bindings::EngineContext context;

    const auto set_error = context.set_global(
        "answer",
        kordex::bindings::Value::number(42.0));

    auto get_result = context.global("answer");

    auto function = kordex::bindings::Function::create(
        "answer",
        [](const kordex::bindings::FunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::number(42.0);
        });

    const auto function_error = context.add_global_function(function);
    auto import_result = context.import_module("missing");

    return expect_true(
               set_error.has_error(),
               "set global before initialize should fail") &&
           expect_true(
               !get_result,
               "get global before initialize should fail") &&
           expect_true(
               function_error.has_error(),
               "add function before initialize should fail") &&
           expect_true(
               !import_result,
               "import before initialize should fail") &&
           expect_true(
               set_error.code() ==
                   kordex::bindings::ErrorCode::NotFound,
               "uninitialized context should map to not found");
  }

  [[nodiscard]] bool test_global_values()
  {
    kordex::bindings::EngineContext context;

    const auto init_error = context.initialize();

    const auto set_error = context.set_global(
        "answer",
        kordex::bindings::Value::number(42.0));

    auto value = context.global("answer");

    if (!expect_true(
            !init_error && !set_error,
            "context init and set global should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(value),
            "getting global value should succeed"))
    {
      return false;
    }

    auto number = value.value().as_number();

    if (!expect_true(
            result_ok(number),
            "global value should convert to number"))
    {
      return false;
    }

    return expect_true(
               context.has_global("answer"),
               "context should have global") &&
           expect_true(
               context.global_count() == 1,
               "global count should be one") &&
           expect_true(
               number.value() == 42.0,
               "global value should be 42");
  }

  [[nodiscard]] bool test_missing_global()
  {
    kordex::bindings::EngineContext context;

    const auto init_error = context.initialize();
    auto value = context.global("missing");

    return expect_true(
               !init_error,
               "context initialize should succeed") &&
           expect_true(
               !value,
               "missing global should fail") &&
           expect_true(
               value.error().code() ==
                   kordex::bindings::ErrorCode::InvalidArgument,
               "missing global should map to invalid argument");
  }

  [[nodiscard]] bool test_invalid_global_name()
  {
    kordex::bindings::EngineContext context;

    const auto init_error = context.initialize();

    const auto set_error = context.set_global(
        "",
        kordex::bindings::Value::number(1.0));

    auto get_result = context.global("");
    const auto remove_error = context.remove_global("");

    return expect_true(
               !init_error,
               "context initialize should succeed") &&
           expect_true(
               set_error.has_error(),
               "set global with empty name should fail") &&
           expect_true(
               !get_result,
               "get global with empty name should fail") &&
           expect_true(
               remove_error.has_error(),
               "remove global with empty name should fail") &&
           expect_true(
               set_error.code() ==
                   kordex::bindings::ErrorCode::InvalidArgument,
               "empty global name should map to invalid argument");
  }

  [[nodiscard]] bool test_remove_global()
  {
    kordex::bindings::EngineContext context;

    const auto init_error = context.initialize();

    const auto set_error = context.set_global(
        "enabled",
        kordex::bindings::Value::boolean(true));

    const auto remove_error = context.remove_global("enabled");
    auto removed_value = context.global("enabled");

    return expect_true(
               !init_error && !set_error,
               "context init and set global should succeed") &&
           expect_true(
               !remove_error,
               "remove global should succeed") &&
           expect_true(
               !context.has_global("enabled"),
               "removed global should not exist") &&
           expect_true(
               context.global_count() == 0,
               "global count should be zero after remove") &&
           expect_true(
               !removed_value,
               "removed global lookup should fail");
  }

  [[nodiscard]] bool test_global_functions()
  {
    kordex::bindings::EngineContext context;

    const auto init_error = context.initialize();

    auto function = kordex::bindings::Function::create(
        "answer",
        [](const kordex::bindings::FunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::number(42.0);
        });

    const auto add_error = context.add_global_function(function);
    auto stored_function = context.global_function("answer");
    auto call_result = context.call_global_function("answer");

    if (!expect_true(
            !init_error && !add_error,
            "context init and add global function should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(stored_function),
            "stored global function should be found"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(call_result),
            "global function call should succeed"))
    {
      return false;
    }

    auto number = call_result.value().as_number();

    if (!expect_true(
            result_ok(number),
            "global function result should convert to number"))
    {
      return false;
    }

    return expect_true(
               context.has_global_function("answer"),
               "context should have global function") &&
           expect_true(
               context.global_function_count() == 1,
               "global function count should be one") &&
           expect_true(
               number.value() == 42.0,
               "global function should return 42");
  }

  [[nodiscard]] bool test_set_global_function_explicit_name()
  {
    kordex::bindings::EngineContext context;

    const auto init_error = context.initialize();

    auto function = kordex::bindings::Function::create(
        "internal_answer",
        [](const kordex::bindings::FunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::number(42.0);
        });

    const auto set_error = context.set_global_function(
        "answer",
        function);

    return expect_true(
               !init_error && !set_error,
               "setting global function with explicit name should succeed") &&
           expect_true(
               context.has_global_function("answer"),
               "context should store explicit function name") &&
           expect_true(
               !context.has_global_function("internal_answer"),
               "context should not store internal function name automatically");
  }

  [[nodiscard]] bool test_missing_global_function()
  {
    kordex::bindings::EngineContext context;

    const auto init_error = context.initialize();

    auto function = context.global_function("missing");
    auto call_result = context.call_global_function("missing");
    const auto remove_error = context.remove_global_function("missing");

    return expect_true(
               !init_error,
               "context initialize should succeed") &&
           expect_true(
               !function,
               "missing global function lookup should fail") &&
           expect_true(
               !call_result,
               "missing global function call should fail") &&
           expect_true(
               remove_error.has_error(),
               "missing global function remove should fail") &&
           expect_true(
               function.error().code() ==
                   kordex::bindings::ErrorCode::InternalError,
               "missing global function should map to internal error");
  }

  [[nodiscard]] bool test_native_functions_disabled()
  {
    auto options = kordex::bindings::BindingOptions::defaults();
    options.allow_native_modules = false;
    options.allow_native_functions = false;

    auto config_result = kordex::bindings::BindingConfig::from_options(options);
    if (!expect_true(
            result_ok(config_result),
            "config with native functions disabled should be valid"))
    {
      return false;
    }

    kordex::bindings::EngineContext context(config_result.value());

    const auto init_error = context.initialize();

    auto function = kordex::bindings::Function::create(
        "blocked",
        [](const kordex::bindings::FunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::undefined();
        });

    const auto set_error = context.add_global_function(function);

    return expect_true(
               !init_error,
               "context initialize should succeed") &&
           expect_true(
               set_error.has_error(),
               "adding global function should fail when native functions disabled") &&
           expect_true(
               set_error.code() ==
                   kordex::bindings::ErrorCode::PermissionDenied,
               "disabled native functions should map to permission denied");
  }

  [[nodiscard]] bool test_remove_global_function()
  {
    kordex::bindings::EngineContext context;

    const auto init_error = context.initialize();

    auto function = kordex::bindings::Function::create(
        "hello",
        [](const kordex::bindings::FunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::string("hello");
        });

    const auto add_error = context.add_global_function(function);
    const auto remove_error = context.remove_global_function("hello");

    return expect_true(
               !init_error && !add_error,
               "context init and add global function should succeed") &&
           expect_true(
               !remove_error,
               "remove global function should succeed") &&
           expect_true(
               !context.has_global_function("hello"),
               "removed global function should not exist") &&
           expect_true(
               context.global_function_count() == 0,
               "global function count should be zero after remove");
  }

  [[nodiscard]] bool test_register_and_import_module()
  {
    kordex::bindings::EngineContext context;

    const auto init_error = context.initialize();

    auto module = kordex::bindings::Module::create(
        "main",
        kordex::bindings::ModuleKind::Script);

    const auto register_error = context.register_module(module);
    auto imported_module = context.import_module("main");

    if (!expect_true(
            !init_error && !register_error,
            "context init and module registration should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(imported_module),
            "module import should succeed"))
    {
      return false;
    }

    return expect_true(
               context.registry().has_module("main"),
               "context registry should have module") &&
           expect_true(
               std::string_view(imported_module.value().name()) == "main",
               "imported module name should match");
  }

  [[nodiscard]] bool test_register_native_module()
  {
    kordex::bindings::EngineContext context;

    const auto init_error = context.initialize();

    auto native_module = kordex::bindings::NativeModule::create("console");

    const auto value_error = native_module.set_value(
        "version",
        kordex::bindings::Value::string("0.1.0"));

    const auto register_error = context.register_native_module(native_module);
    auto imported_module = context.import_module("console");

    if (!expect_true(
            !init_error && !value_error && !register_error,
            "context init and native module registration should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(imported_module),
            "native module import should succeed"))
    {
      return false;
    }

    return expect_true(
               imported_module.value().kind() ==
                   kordex::bindings::ModuleKind::Native,
               "imported native module should have native kind") &&
           expect_true(
               imported_module.value().has_export("version"),
               "imported native module should expose value");
  }

  [[nodiscard]] bool test_native_modules_disabled()
  {
    auto options = kordex::bindings::BindingOptions::defaults();
    options.allow_native_modules = false;

    auto config_result = kordex::bindings::BindingConfig::from_options(options);
    if (!expect_true(
            result_ok(config_result),
            "config with native modules disabled should be valid"))
    {
      return false;
    }

    kordex::bindings::EngineContext context(config_result.value());

    const auto init_error = context.initialize();

    auto native_module = kordex::bindings::NativeModule::create("console");
    const auto register_error = context.register_native_module(native_module);

    return expect_true(
               !init_error,
               "context initialize should succeed") &&
           expect_true(
               register_error.has_error(),
               "register native module should fail when disabled") &&
           expect_true(
               register_error.code() ==
                   kordex::bindings::ErrorCode::PermissionDenied,
               "disabled native modules should map to permission denied");
  }

  [[nodiscard]] bool test_module_policy_builtin_only()
  {
    auto options = kordex::bindings::BindingOptions::defaults();
    options.module_policy = kordex::bindings::ModulePolicy::BuiltinOnly;

    auto config_result = kordex::bindings::BindingConfig::from_options(options);
    if (!expect_true(
            result_ok(config_result),
            "builtin only config should be valid"))
    {
      return false;
    }

    kordex::bindings::EngineContext context(config_result.value());

    const auto init_error = context.initialize();

    auto script_module = kordex::bindings::Module::create(
        "script",
        kordex::bindings::ModuleKind::Script);

    auto builtin_module = kordex::bindings::Module::create(
        "builtin",
        kordex::bindings::ModuleKind::Builtin);

    const auto script_error = context.register_module(script_module);
    const auto builtin_error = context.register_module(builtin_module);

    return expect_true(
               !init_error,
               "context initialize should succeed") &&
           expect_true(
               script_error.has_error(),
               "script module should be rejected in builtin only mode") &&
           expect_true(
               !builtin_error,
               "builtin module should be accepted in builtin only mode") &&
           expect_true(
               script_error.code() ==
                   kordex::bindings::ErrorCode::PermissionDenied,
               "script rejection should map to permission denied");
  }

  [[nodiscard]] bool test_run_script()
  {
    kordex::bindings::EngineContext context;

    const auto init_error = context.initialize();

    auto script = kordex::bindings::Script::from_source(
        "console.log('hello');",
        "hello.js");

    auto result = context.run_script(script);

    if (!expect_true(
            !init_error,
            "context initialize should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result.succeeded(),
            "context run script should succeed"))
    {
      return false;
    }

    auto value = result.value.as_string();

    if (!expect_true(
            result_ok(value),
            "run script value should convert to string"))
    {
      return false;
    }

    return expect_true(
               value.value() == "console.log('hello');",
               "run script should return source placeholder") &&
           expect_true(
               std::string_view(result.output) ==
                   "script executed by native placeholder",
               "run script output should match");
  }

  [[nodiscard]] bool test_run_script_requires_initialization()
  {
    kordex::bindings::EngineContext context;

    auto script = kordex::bindings::Script::from_source(
        "console.log('hello');",
        "hello.js");

    auto result = context.run_script(script);

    return expect_true(
               result.failed(),
               "run script before initialize should fail") &&
           expect_true(
               result.error.has_error(),
               "run script failure should contain error") &&
           expect_true(
               result.error.code() ==
                   kordex::bindings::ErrorCode::NotFound,
               "uninitialized run script should map to not found");
  }

  [[nodiscard]] bool test_eval()
  {
    kordex::bindings::EngineContext context;

    const auto init_error = context.initialize();

    auto result = context.eval(
        "1 + 1",
        "eval.js");

    if (!expect_true(
            !init_error,
            "context initialize should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result.succeeded(),
            "context eval should succeed"))
    {
      return false;
    }

    auto value = result.value.as_string();

    if (!expect_true(
            result_ok(value),
            "eval value should convert to string"))
    {
      return false;
    }

    return expect_true(
        value.value() == "1 + 1",
        "eval placeholder should return source");
  }

  [[nodiscard]] bool test_global_names_and_clear()
  {
    kordex::bindings::EngineContext context;

    const auto init_error = context.initialize();

    const auto global_error = context.set_global(
        "a",
        kordex::bindings::Value::number(1.0));

    auto function = kordex::bindings::Function::create(
        "b",
        [](const kordex::bindings::FunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::undefined();
        });

    const auto function_error = context.add_global_function(function);

    auto module = kordex::bindings::Module::create(
        "c",
        kordex::bindings::ModuleKind::Script);

    const auto module_error = context.register_module(module);

    const auto global_names = context.global_names();
    const auto function_names = context.global_function_names();

    const bool names_ok =
        global_names.size() == 1 &&
        global_names[0] == "a" &&
        function_names.size() == 1 &&
        function_names[0] == "b";

    context.clear();

    return expect_true(
               !init_error &&
                   !global_error &&
                   !function_error &&
                   !module_error,
               "context setup should succeed") &&
           expect_true(
               names_ok,
               "context names should match") &&
           expect_true(
               context.global_count() == 0,
               "global count should be zero after clear") &&
           expect_true(
               context.global_function_count() == 0,
               "global function count should be zero after clear") &&
           expect_true(
               context.registry().empty(),
               "registry should be empty after clear");
  }

  [[nodiscard]] bool test_shutdown_preserves_data_but_blocks_operations()
  {
    kordex::bindings::EngineContext context;

    const auto init_error = context.initialize();

    const auto set_error = context.set_global(
        "answer",
        kordex::bindings::Value::number(42.0));

    const auto shutdown_error = context.shutdown();

    auto value = context.global("answer");

    return expect_true(
               !init_error && !set_error && !shutdown_error,
               "context init, set, and shutdown should succeed") &&
           expect_true(
               !context.initialized(),
               "context should not be initialized after shutdown") &&
           expect_true(
               context.has_global("answer"),
               "shutdown should preserve data") &&
           expect_true(
               !value,
               "global access after shutdown should fail") &&
           expect_true(
               value.error().code() ==
                   kordex::bindings::ErrorCode::NotFound,
               "global access after shutdown should map to not found");
  }

} // namespace

int main()
{
  const bool ok =
      test_context_info_helpers() &&
      test_default_context() &&
      test_create_context() &&
      test_create_context_from_options() &&
      test_create_context_from_config() &&
      test_initialize_and_shutdown() &&
      test_initialize_invalid_context_name() &&
      test_name_and_id_setters() &&
      test_operations_require_initialization() &&
      test_global_values() &&
      test_missing_global() &&
      test_invalid_global_name() &&
      test_remove_global() &&
      test_global_functions() &&
      test_set_global_function_explicit_name() &&
      test_missing_global_function() &&
      test_native_functions_disabled() &&
      test_remove_global_function() &&
      test_register_and_import_module() &&
      test_register_native_module() &&
      test_native_modules_disabled() &&
      test_module_policy_builtin_only() &&
      test_run_script() &&
      test_run_script_requires_initialization() &&
      test_eval() &&
      test_global_names_and_clear() &&
      test_shutdown_preserves_data_but_blocks_operations();

  return ok ? 0 : 1;
}
