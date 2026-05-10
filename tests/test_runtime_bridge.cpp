/**
 *
 *  @file test_runtime_bridge.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Gaspard Kirira.
 *  All rights reserved.
 *  https://github.com/kordexjs/bindings
 *
 *  Use of this source code is governed by a MIT license
 *  that can be found in the LICENSE file.
 *
 *  Kordex Bindings
 *
 */

#include <string_view>
#include <kordex/bindings/RuntimeBridge.hpp>

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

  [[nodiscard]] bool test_runtime_bridge_info_helpers()
  {
    kordex::bindings::RuntimeBridgeInfo info;
    info.name = "bridge";
    info.attached = true;
    info.initialized = true;

    return expect_true(
               info.has_name(),
               "runtime bridge info should have name") &&
           expect_true(
               info.attached,
               "runtime bridge info should be attached") &&
           expect_true(
               info.initialized,
               "runtime bridge info should be initialized");
  }

  [[nodiscard]] bool test_default_bridge()
  {
    kordex::bindings::RuntimeBridge bridge;

    return expect_true(
               bridge.attached(),
               "default bridge should be attached") &&
           expect_true(
               !bridge.initialized(),
               "default bridge should not be initialized") &&
           expect_true(
               bridge.owns_runtime(),
               "default bridge should own runtime") &&
           expect_true(
               bridge.options().owns_runtime,
               "default bridge options should report ownership") &&
           expect_true(
               std::string_view(bridge.name()) == "runtime-bridge",
               "default bridge name should match");
  }

  [[nodiscard]] bool test_create_bridge()
  {
    auto result = kordex::bindings::RuntimeBridge::create();

    if (!expect_true(
            result_ok(result),
            "runtime bridge creation should succeed"))
    {
      return false;
    }

    auto &bridge = result.value();

    return expect_true(
               bridge.attached(),
               "created bridge should be attached") &&
           expect_true(
               !bridge.initialized(),
               "created bridge should not be initialized") &&
           expect_true(
               bridge.owns_runtime(),
               "created bridge should own runtime") &&
           expect_true(
               std::string_view(bridge.name()) == "runtime-bridge",
               "created bridge name should match");
  }

  [[nodiscard]] bool test_create_bridge_from_runtime_options()
  {
    auto runtime_options = kordex::runtime::RuntimeOptions::test();

    auto result = kordex::bindings::RuntimeBridge::create(runtime_options);

    if (!expect_true(
            result_ok(result),
            "runtime bridge creation from runtime options should succeed"))
    {
      return false;
    }

    auto &bridge = result.value();

    return expect_true(
               bridge.attached(),
               "bridge from runtime options should be attached") &&
           expect_true(
               bridge.owns_runtime(),
               "bridge from runtime options should own runtime") &&
           expect_true(
               !bridge.initialized(),
               "bridge from runtime options should not initialize automatically");
  }

  [[nodiscard]] bool test_attach_existing_runtime()
  {
    kordex::runtime::Runtime runtime;

    auto result = kordex::bindings::RuntimeBridge::attach(runtime);

    if (!expect_true(
            result_ok(result),
            "runtime bridge attach should succeed"))
    {
      return false;
    }

    auto &bridge = result.value();

    return expect_true(
               bridge.attached(),
               "attached bridge should be attached") &&
           expect_true(
               !bridge.owns_runtime(),
               "attached bridge should not own runtime") &&
           expect_true(
               !bridge.options().owns_runtime,
               "attached bridge options should report non-ownership") &&
           expect_true(
               !bridge.initialized(),
               "attached bridge should not initialize automatically");
  }

  [[nodiscard]] bool test_initialize_and_shutdown()
  {
    kordex::bindings::RuntimeBridge bridge;

    auto init_result = bridge.initialize();

    const bool initialized = bridge.initialized();

    auto second_init_result = bridge.initialize();

    auto shutdown_result = bridge.shutdown();

    const bool stopped = !bridge.initialized();

    auto second_shutdown_result = bridge.shutdown();

    return expect_true(
               init_result.succeeded(),
               "runtime bridge initialize should succeed") &&
           expect_true(
               initialized,
               "runtime bridge should be initialized") &&
           expect_true(
               second_init_result.succeeded(),
               "second initialize should be idempotent") &&
           expect_true(
               shutdown_result.succeeded(),
               "runtime bridge shutdown should succeed") &&
           expect_true(
               stopped,
               "runtime bridge should be stopped after shutdown") &&
           expect_true(
               second_shutdown_result.succeeded(),
               "second shutdown should be idempotent");
  }

  [[nodiscard]] bool test_initialize_invalid_name()
  {
    kordex::bindings::RuntimeBridge bridge;
    bridge.set_name("");

    auto result = bridge.initialize();

    return expect_true(
               result.failed(),
               "initializing bridge with empty name should fail") &&
           expect_true(
               result.error.has_error(),
               "failed bridge initialization should contain error") &&
           expect_true(
               result.error.code() ==
                   kordex::bindings::ErrorCode::InternalError,
               "bridge initialization failure should map to internal error") &&
           expect_true(
               !bridge.initialized(),
               "invalid bridge should not be initialized");
  }

  [[nodiscard]] bool test_set_name_and_options()
  {
    kordex::bindings::RuntimeBridge bridge;

    bridge.set_name("main-bridge");

    kordex::bindings::RuntimeBridgeOptions options;
    options.auto_start_runtime = true;
    options.allow_source_loading = false;
    options.allow_module_resolution = false;
    options.allow_manifest_loading = false;
    options.allow_tasks = false;

    bridge.set_options(options);

    return expect_true(
               std::string_view(bridge.name()) == "main-bridge",
               "bridge name should be updated") &&
           expect_true(
               bridge.options().auto_start_runtime,
               "bridge auto start option should be updated") &&
           expect_true(
               !bridge.options().allow_source_loading,
               "bridge source loading option should be updated") &&
           expect_true(
               !bridge.options().allow_module_resolution,
               "bridge module resolution option should be updated") &&
           expect_true(
               !bridge.options().allow_manifest_loading,
               "bridge manifest loading option should be updated") &&
           expect_true(
               !bridge.options().allow_tasks,
               "bridge task option should be updated") &&
           expect_true(
               bridge.options().owns_runtime == bridge.owns_runtime(),
               "bridge ownership option should remain consistent");
  }

  [[nodiscard]] bool test_operations_require_initialization()
  {
    kordex::bindings::RuntimeBridge bridge;

    auto source = bridge.load_source("main.js");
    auto script = bridge.load_script("main.js");
    auto module = bridge.resolve_module("./main.js");
    auto manifest = bridge.load_manifest("package.json");

    return expect_true(
               !source,
               "load_source before initialize should fail") &&
           expect_true(
               !script,
               "load_script before initialize should fail") &&
           expect_true(
               !module,
               "resolve_module before initialize should fail") &&
           expect_true(
               !manifest,
               "load_manifest before initialize should fail") &&
           expect_true(
               source.error().code() ==
                   kordex::bindings::ErrorCode::InternalError,
               "uninitialized bridge should map to internal error");
  }

  [[nodiscard]] bool test_eval_success()
  {
    kordex::bindings::RuntimeBridge bridge;

    auto init_result = bridge.initialize();

    auto result = bridge.eval(
        "1 + 1",
        "eval.js");

    if (!expect_true(
            init_result.succeeded(),
            "bridge initialize should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result.succeeded(),
            "bridge eval should succeed"))
    {
      return false;
    }

    auto value = result.value.as_string();

    if (!expect_true(
            result_ok(value),
            "bridge eval result should convert to string"))
    {
      return false;
    }

    return expect_true(
               value.value() == "1 + 1",
               "bridge eval placeholder should return source") &&
           expect_true(
               std::string_view(result.output) ==
                   "script executed by native placeholder",
               "bridge eval output should match");
  }

  [[nodiscard]] bool test_run_script_success()
  {
    kordex::bindings::RuntimeBridge bridge;

    auto init_result = bridge.initialize();

    auto script = kordex::bindings::Script::from_source(
        "console.log('hello');",
        "hello.js");

    auto result = bridge.run_script(script);

    if (!expect_true(
            init_result.succeeded(),
            "bridge initialize should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result.succeeded(),
            "bridge run_script should succeed"))
    {
      return false;
    }

    auto value = result.value.as_string();

    if (!expect_true(
            result_ok(value),
            "bridge run_script result should convert to string"))
    {
      return false;
    }

    return expect_true(
        value.value() == "console.log('hello');",
        "bridge run_script placeholder should return source");
  }

  [[nodiscard]] bool test_run_script_requires_initialization()
  {
    kordex::bindings::RuntimeBridge bridge;

    auto script = kordex::bindings::Script::from_source(
        "console.log('hello');",
        "hello.js");

    auto result = bridge.run_script(script);

    return expect_true(
               result.failed(),
               "run_script before initialize should fail") &&
           expect_true(
               result.error.has_error(),
               "run_script before initialize should contain error") &&
           expect_true(
               result.error.code() ==
                   kordex::bindings::ErrorCode::InternalError,
               "run_script before initialize should map to internal error");
  }

  [[nodiscard]] bool test_permission_options()
  {
    kordex::bindings::RuntimeBridge bridge;

    kordex::bindings::RuntimeBridgeOptions options;
    options.allow_source_loading = false;
    options.allow_module_resolution = false;
    options.allow_manifest_loading = false;

    bridge.set_options(options);

    auto init_result = bridge.initialize();

    auto source = bridge.load_source("main.js");
    auto script = bridge.load_script("main.js");
    auto module = bridge.resolve_module("./main.js");
    auto manifest = bridge.load_manifest("package.json");
    auto run_file_result = bridge.run_file("main.js");

    return expect_true(
               init_result.succeeded(),
               "bridge initialize should succeed") &&
           expect_true(
               !source,
               "load_source should fail when disabled") &&
           expect_true(
               !script,
               "load_script should fail when source loading disabled") &&
           expect_true(
               !module,
               "resolve_module should fail when disabled") &&
           expect_true(
               !manifest,
               "load_manifest should fail when disabled") &&
           expect_true(
               run_file_result.failed(),
               "run_file should fail when source loading disabled") &&
           expect_true(
               source.error().code() ==
                   kordex::bindings::ErrorCode::PermissionDenied,
               "disabled source loading should map to permission denied") &&
           expect_true(
               module.error().code() ==
                   kordex::bindings::ErrorCode::PermissionDenied,
               "disabled module resolution should map to permission denied") &&
           expect_true(
               manifest.error().code() ==
                   kordex::bindings::ErrorCode::PermissionDenied,
               "disabled manifest loading should map to permission denied") &&
           expect_true(
               run_file_result.error.code() ==
                   kordex::bindings::ErrorCode::PermissionDenied,
               "disabled run_file should map to permission denied");
  }

  [[nodiscard]] bool test_runtime_state_values()
  {
    kordex::bindings::RuntimeBridge bridge;

    const auto state_value = bridge.runtime_state_value();
    const auto running_value = bridge.runtime_running_value();

    auto state_text = state_value.as_string();
    auto running = running_value.as_boolean();

    if (!expect_true(
            result_ok(state_text),
            "runtime state value should be string"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(running),
            "runtime running value should be boolean"))
    {
      return false;
    }

    return expect_true(
               !state_text.value().empty(),
               "runtime state string should not be empty") &&
           expect_true(
               !running.value(),
               "default runtime should not be running");
  }

  [[nodiscard]] bool test_install_into_context()
  {
    kordex::bindings::RuntimeBridge bridge;

    auto bridge_init = bridge.initialize();

    kordex::bindings::EngineContext context;
    const auto context_init = context.initialize();

    const auto install_error = bridge.install_into(context);

    auto state_value = context.global("kordex_runtime_state");
    auto running_value = context.global("kordex_runtime_running");

    if (!expect_true(
            bridge_init.succeeded() &&
                !context_init &&
                !install_error,
            "bridge and context setup should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(state_value) && result_ok(running_value),
            "runtime bridge globals should be installed"))
    {
      return false;
    }

    return expect_true(
               context.has_global("kordex_runtime_state"),
               "context should contain runtime state global") &&
           expect_true(
               context.has_global("kordex_runtime_running"),
               "context should contain runtime running global") &&
           expect_true(
               state_value.value().is_string(),
               "runtime state global should be string") &&
           expect_true(
               running_value.value().is_boolean(),
               "runtime running global should be boolean");
  }

  [[nodiscard]] bool test_install_into_uninitialized_context()
  {
    kordex::bindings::RuntimeBridge bridge;

    auto bridge_init = bridge.initialize();

    kordex::bindings::EngineContext context;

    const auto install_error = bridge.install_into(context);

    return expect_true(
               bridge_init.succeeded(),
               "bridge initialize should succeed") &&
           expect_true(
               install_error.has_error(),
               "installing into uninitialized context should fail") &&
           expect_true(
               install_error.code() ==
                   kordex::bindings::ErrorCode::NotFound,
               "uninitialized context should map to not found");
  }

  [[nodiscard]] bool test_from_runtime_success_result()
  {
    auto runtime_result = kordex::runtime::RuntimeResult::success(
        "runtime output");

    auto result = kordex::bindings::RuntimeBridge::from_runtime_result(
        runtime_result);

    return expect_true(
               result.succeeded(),
               "runtime success should convert to script success") &&
           expect_true(
               result.exit_code == 0,
               "converted success exit code should be zero") &&
           expect_true(
               !result.error.has_error(),
               "converted success should not contain error") &&
           expect_true(
               result.value.is_string(),
               "converted success value should be string") &&
           expect_true(
               std::string_view(result.output) == "runtime output",
               "converted success output should match");
  }

  [[nodiscard]] bool test_from_runtime_failure_result()
  {
    auto runtime_result = kordex::runtime::RuntimeResult::failure(
        kordex::runtime::make_runtime_error(
            kordex::runtime::RuntimeErrorCode::InternalError,
            "runtime failed"),
        7);

    auto result = kordex::bindings::RuntimeBridge::from_runtime_result(
        runtime_result);

    return expect_true(
               result.failed(),
               "runtime failure should convert to script failure") &&
           expect_true(
               result.exit_code == 7,
               "converted failure exit code should match") &&
           expect_true(
               result.error.has_error(),
               "converted failure should contain error") &&
           expect_true(
               result.error.code() ==
                   kordex::bindings::ErrorCode::InternalError,
               "converted failure should map to internal error") &&
           expect_true(
               std::string_view(result.error.message()) ==
                   "runtime failed",
               "converted failure message should match");
  }

  [[nodiscard]] bool test_from_runtime_cancelled_result()
  {
    auto runtime_result = kordex::runtime::RuntimeResult::cancelled(
        "runtime cancelled");

    auto result = kordex::bindings::RuntimeBridge::from_runtime_result(
        runtime_result);

    return expect_true(
               result.was_cancelled(),
               "runtime cancelled should convert to script cancelled") &&
           expect_true(
               result.exit_code == 130,
               "converted cancelled exit code should be 130") &&
           expect_true(
               result.error.has_error(),
               "converted cancelled should contain error") &&
           expect_true(
               result.error.code() ==
                   kordex::bindings::ErrorCode::Cancelled,
               "converted cancelled should map to cancelled");
  }

  [[nodiscard]] bool test_from_runtime_timeout_result()
  {
    auto runtime_result = kordex::runtime::RuntimeResult::timeout(
        "runtime timed out");

    auto result = kordex::bindings::RuntimeBridge::from_runtime_result(
        runtime_result);

    return expect_true(
               result.timed_out(),
               "runtime timeout should convert to script timeout") &&
           expect_true(
               result.exit_code == 124,
               "converted timeout exit code should be 124") &&
           expect_true(
               result.error.has_error(),
               "converted timeout should contain error") &&
           expect_true(
               result.error.code() ==
                   kordex::bindings::ErrorCode::Timeout,
               "converted timeout should map to timeout");
  }

  [[nodiscard]] bool test_script_from_source_file()
  {
    kordex::runtime::SourceFile source_file;
    source_file.original_path = "main.js";
    source_file.path = "src/main.js";
    source_file.type = kordex::runtime::SourceType::JavaScript;
    source_file.content = "console.log('hello');";

    auto result = kordex::bindings::RuntimeBridge::script_from_source_file(
        source_file);

    if (!expect_true(
            result_ok(result),
            "script_from_source_file should succeed"))
    {
      return false;
    }

    const auto &script = result.value();

    return expect_true(
               script.valid(),
               "converted script should be valid") &&
           expect_true(
               script.loaded_from_file(),
               "converted script should be loaded from file") &&
           expect_true(
               std::string_view(script.path()) == "src/main.js",
               "converted script path should match") &&
           expect_true(
               script.type() == kordex::bindings::ScriptType::JavaScript,
               "converted script type should match");
  }

  [[nodiscard]] bool test_invalid_script_from_source_file()
  {
    kordex::runtime::SourceFile source_file;
    source_file.original_path = "README.md";
    source_file.path = "README.md";
    source_file.type = kordex::runtime::SourceType::Unknown;
    source_file.content = "hello";

    auto result = kordex::bindings::RuntimeBridge::script_from_source_file(
        source_file);

    return expect_true(
               !result,
               "invalid source file conversion should fail") &&
           expect_true(
               result.error().code() ==
                   kordex::bindings::ErrorCode::InvalidArgument,
               "invalid source file conversion should map to invalid argument");
  }

  [[nodiscard]] bool test_module_from_resolved_module()
  {
    auto module_id = kordex::runtime::ModuleId::parse("kordex:fs");

    if (!expect_true(
            !(!module_id),
            "builtin module id should parse"))
    {
      return false;
    }

    kordex::runtime::ResolvedModule resolved_module;
    resolved_module.id = module_id.value();
    resolved_module.path = "kordex:fs";
    resolved_module.found = true;

    auto module = kordex::bindings::RuntimeBridge::module_from_resolved_module(
        resolved_module);

    auto specifier = module.export_value("specifier");
    auto path = module.export_value("path");
    auto found = module.export_value("found");
    auto builtin = module.export_value("builtin");

    if (!expect_true(
            result_ok(specifier) &&
                result_ok(path) &&
                result_ok(found) &&
                result_ok(builtin),
            "converted module should expose metadata"))
    {
      return false;
    }

    auto specifier_text = specifier.value().as_string();
    auto path_text = path.value().as_string();
    auto found_bool = found.value().as_boolean();
    auto builtin_bool = builtin.value().as_boolean();

    if (!expect_true(
            result_ok(specifier_text) &&
                result_ok(path_text) &&
                result_ok(found_bool) &&
                result_ok(builtin_bool),
            "converted module metadata should have expected value types"))
    {
      return false;
    }

    return expect_true(
               module.valid(),
               "converted module should be valid") &&
           expect_true(
               module.kind() == kordex::bindings::ModuleKind::Builtin,
               "converted builtin module kind should be builtin") &&
           expect_true(
               module.loaded(),
               "converted module should be loaded when resolved found") &&
           expect_true(
               specifier_text.value() == "kordex:fs",
               "converted module specifier should match") &&
           expect_true(
               path_text.value() == "kordex:fs",
               "converted module path should match") &&
           expect_true(
               found_bool.value(),
               "converted module found value should be true") &&
           expect_true(
               builtin_bool.value(),
               "converted module builtin value should be true");
  }

} // namespace

int main()
{
  const bool ok =
      test_runtime_bridge_info_helpers() &&
      test_default_bridge() &&
      test_create_bridge() &&
      test_create_bridge_from_runtime_options() &&
      test_attach_existing_runtime() &&
      test_initialize_and_shutdown() &&
      test_initialize_invalid_name() &&
      test_set_name_and_options() &&
      test_operations_require_initialization() &&
      test_eval_success() &&
      test_run_script_success() &&
      test_run_script_requires_initialization() &&
      test_permission_options() &&
      test_runtime_state_values() &&
      test_install_into_context() &&
      test_install_into_uninitialized_context() &&
      test_from_runtime_success_result() &&
      test_from_runtime_failure_result() &&
      test_from_runtime_cancelled_result() &&
      test_from_runtime_timeout_result() &&
      test_script_from_source_file() &&
      test_invalid_script_from_source_file() &&
      test_module_from_resolved_module();

  return ok ? 0 : 1;
}
