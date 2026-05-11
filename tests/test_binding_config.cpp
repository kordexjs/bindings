/**
 *
 *  @file test_binding_config.cpp
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

#include <kordex/bindings/BindingConfig.hpp>

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

  [[nodiscard]] bool test_config_from_default_options()
  {
    auto result = kordex::bindings::BindingConfig::from_options(
        kordex::bindings::BindingOptions::defaults());

    if (!expect_true(
            static_cast<bool>(result),
            "default options should create a valid config"))
    {
      return false;
    }

    const auto &config = result.value();

    return expect_true(
               config.backend == kordex::bindings::EngineBackend::Native,
               "default config backend should be native") &&
           expect_true(
               config.module_policy == kordex::bindings::ModulePolicy::Full,
               "default config module policy should be full") &&
           expect_true(
               config.allow_native_modules,
               "default config should allow native modules") &&
           expect_true(
               config.allow_native_functions,
               "default config should allow native functions") &&
           expect_true(
               config.allow_runtime_bridge,
               "default config should allow runtime bridge") &&
           expect_true(
               std::string_view(config.engine_name) == "native",
               "default config engine name should be native");
  }

  [[nodiscard]] bool test_config_from_development_options()
  {
    auto result = kordex::bindings::BindingConfig::from_options(
        kordex::bindings::BindingOptions::development());

    if (!expect_true(
            static_cast<bool>(result),
            "development options should create a valid config"))
    {
      return false;
    }

    const auto &config = result.value();

    return expect_true(
               config.debug,
               "development config should enable debug") &&
           expect_true(
               config.source_maps,
               "development config should enable source maps") &&
           expect_true(
               std::string_view(config.environment) == "development",
               "development environment should match") &&
           expect_true(
               std::string_view(config.engine_name) == "native-dev",
               "development engine name should match");
  }

  [[nodiscard]] bool test_config_from_production_options()
  {
    auto result = kordex::bindings::BindingConfig::from_options(
        kordex::bindings::BindingOptions::production());

    if (!expect_true(
            static_cast<bool>(result),
            "production options should create a valid config"))
    {
      return false;
    }

    const auto &config = result.value();

    return expect_true(
               !config.debug,
               "production config should disable debug") &&
           expect_true(
               !config.source_maps,
               "production config should disable source maps") &&
           expect_true(
               std::string_view(config.environment) == "production",
               "production environment should match") &&
           expect_true(
               std::string_view(config.engine_name) == "native",
               "production engine name should match");
  }

  [[nodiscard]] bool test_config_helpers()
  {
    kordex::bindings::BindingConfig config;

    const bool initially_empty =
        !config.has_stack_limit() &&
        !config.has_heap_limit() &&
        !config.has_module_paths();

    config.max_stack_size = 1024;
    config.max_heap_size = 2048;
    config.module_paths.push_back("modules");
    config.engine_name = "native-test";

    return expect_true(
               initially_empty,
               "config should initially have no limits or paths") &&
           expect_true(
               config.has_stack_limit(),
               "config stack limit should be detected") &&
           expect_true(
               config.has_heap_limit(),
               "config heap limit should be detected") &&
           expect_true(
               config.has_module_paths(),
               "config module paths should be detected") &&
           expect_true(
               config.has_engine_name(),
               "config engine name should be detected");
  }

  [[nodiscard]] bool test_parse_engine_backend()
  {
    auto native_backend = kordex::bindings::parse_engine_backend("native");
    auto quickjs_backend = kordex::bindings::parse_engine_backend("quickjs");
    auto v8_backend = kordex::bindings::parse_engine_backend("v8");
    auto invalid_backend = kordex::bindings::parse_engine_backend("unknown");

    return expect_true(
               native_backend &&
                   native_backend.value() ==
                       kordex::bindings::EngineBackend::Native,
               "native backend should parse") &&
           expect_true(
               quickjs_backend &&
                   quickjs_backend.value() ==
                       kordex::bindings::EngineBackend::QuickJS,
               "quickjs backend should parse") &&
           expect_true(
               v8_backend &&
                   v8_backend.value() ==
                       kordex::bindings::EngineBackend::V8,
               "v8 backend should parse") &&
           expect_true(
               !invalid_backend,
               "invalid backend should fail");
  }

  [[nodiscard]] bool test_parse_module_policy()
  {
    auto disabled_policy = kordex::bindings::parse_module_policy("disabled");
    auto builtin_policy = kordex::bindings::parse_module_policy("builtin_only");
    auto full_policy = kordex::bindings::parse_module_policy("full");
    auto invalid_policy = kordex::bindings::parse_module_policy("unknown");

    return expect_true(
               disabled_policy &&
                   disabled_policy.value() ==
                       kordex::bindings::ModulePolicy::Disabled,
               "disabled policy should parse") &&
           expect_true(
               builtin_policy &&
                   builtin_policy.value() ==
                       kordex::bindings::ModulePolicy::BuiltinOnly,
               "builtin policy should parse") &&
           expect_true(
               full_policy &&
                   full_policy.value() ==
                       kordex::bindings::ModulePolicy::Full,
               "full policy should parse") &&
           expect_true(
               !invalid_policy,
               "invalid policy should fail");
  }

  [[nodiscard]] bool test_invalid_config_disabled_modules()
  {
    kordex::bindings::BindingOptions options;
    options.module_policy = kordex::bindings::ModulePolicy::Disabled;
    options.allow_native_modules = true;

    auto result = kordex::bindings::BindingConfig::from_options(options);

    return expect_true(
        !result,
        "disabled module policy should reject native modules");
  }

  [[nodiscard]] bool test_invalid_config_native_functions()
  {
    kordex::bindings::BindingOptions options;
    options.allow_native_modules = true;
    options.allow_native_functions = false;

    auto result = kordex::bindings::BindingConfig::from_options(options);

    return expect_true(
        !result,
        "native modules should require native functions");
  }
} // namespace

int main()
{
  const bool ok =
      test_config_from_default_options() &&
      test_config_from_development_options() &&
      test_config_from_production_options() &&
      test_config_helpers() &&
      test_parse_engine_backend() &&
      test_parse_module_policy() &&
      test_invalid_config_disabled_modules() &&
      test_invalid_config_native_functions();

  return ok ? 0 : 1;
}
