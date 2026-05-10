/**
 *
 *  @file test_binding_options.cpp
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
#include <kordex/bindings/BindingOptions.hpp>

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

  [[nodiscard]] bool test_default_options()
  {
    const auto options = kordex::bindings::BindingOptions::defaults();

    return expect_true(
               options.backend == kordex::bindings::EngineBackend::Native,
               "default backend should be native") &&
           expect_true(
               options.module_policy == kordex::bindings::ModulePolicy::Full,
               "default module policy should be full") &&
           expect_true(
               options.allow_native_modules,
               "native modules should be allowed by default") &&
           expect_true(
               options.allow_native_functions,
               "native functions should be allowed by default") &&
           expect_true(
               options.allow_runtime_bridge,
               "runtime bridge should be allowed by default") &&
           expect_true(
               options.diagnostics,
               "diagnostics should be enabled by default") &&
           expect_true(
               !options.debug,
               "debug should be disabled by default") &&
           expect_true(
               !options.source_maps,
               "source maps should be disabled by default");
  }

  [[nodiscard]] bool test_development_options()
  {
    const auto options = kordex::bindings::BindingOptions::development();

    return expect_true(
               options.debug,
               "development debug should be enabled") &&
           expect_true(
               options.source_maps,
               "development source maps should be enabled") &&
           expect_true(
               options.has_engine_name(),
               "development should have engine name") &&
           expect_true(
               std::string_view(options.engine_name) == "native-dev",
               "development engine name should match");
  }

  [[nodiscard]] bool test_production_options()
  {
    const auto options = kordex::bindings::BindingOptions::production();

    return expect_true(
               !options.debug,
               "production debug should be disabled") &&
           expect_true(
               !options.source_maps,
               "production source maps should be disabled") &&
           expect_true(
               options.has_engine_name(),
               "production should have engine name") &&
           expect_true(
               std::string_view(options.engine_name) == "native",
               "production engine name should match");
  }

  [[nodiscard]] bool test_test_options()
  {
    const auto options = kordex::bindings::BindingOptions::test();

    return expect_true(
               options.debug,
               "test debug should be enabled") &&
           expect_true(
               options.has_stack_limit(),
               "test should have stack limit") &&
           expect_true(
               options.has_heap_limit(),
               "test should have heap limit") &&
           expect_true(
               std::string_view(options.engine_name) == "native-test",
               "test engine name should match");
  }

  [[nodiscard]] bool test_limit_helpers()
  {
    kordex::bindings::BindingOptions options;

    const bool initially_empty =
        !options.has_stack_limit() &&
        !options.has_heap_limit() &&
        !options.has_module_paths() &&
        !options.has_engine_name();

    options.max_stack_size = 1024;
    options.max_heap_size = 2048;
    options.module_paths.push_back("modules");
    options.engine_name = "custom";

    return expect_true(
               initially_empty,
               "options should initially have no limits or paths") &&
           expect_true(
               options.has_stack_limit(),
               "stack limit should be detected") &&
           expect_true(
               options.has_heap_limit(),
               "heap limit should be detected") &&
           expect_true(
               options.has_module_paths(),
               "module paths should be detected") &&
           expect_true(
               options.has_engine_name(),
               "engine name should be detected");
  }

  [[nodiscard]] bool test_backend_strings()
  {
    return expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::EngineBackend::Native)) == "native",
               "native backend string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::EngineBackend::QuickJS)) == "quickjs",
               "quickjs backend string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::EngineBackend::V8)) == "v8",
               "v8 backend string should match");
  }

  [[nodiscard]] bool test_module_policy_strings()
  {
    return expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::ModulePolicy::Disabled)) == "disabled",
               "disabled policy string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::ModulePolicy::BuiltinOnly)) ==
                   "builtin_only",
               "builtin only policy string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::ModulePolicy::Full)) == "full",
               "full policy string should match");
  }
} // namespace

int main()
{
  const bool ok =
      test_default_options() &&
      test_development_options() &&
      test_production_options() &&
      test_test_options() &&
      test_limit_helpers() &&
      test_backend_strings() &&
      test_module_policy_strings();

  return ok ? 0 : 1;
}
