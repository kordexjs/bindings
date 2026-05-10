/**
 *
 *  @file test_module_registry.cpp
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
#include <kordex/bindings/ModuleRegistry.hpp>

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

  [[nodiscard]] bool test_empty_registry()
  {
    const kordex::bindings::ModuleRegistry registry;

    return expect_true(
               registry.empty(),
               "new registry should be empty") &&
           expect_true(
               registry.size() == 0,
               "new registry size should be zero") &&
           expect_true(
               registry.names().empty(),
               "new registry names should be empty") &&
           expect_true(
               registry.modules().empty(),
               "new registry module map should be empty");
  }

  [[nodiscard]] bool test_register_and_get_module()
  {
    kordex::bindings::ModuleRegistry registry;

    auto module = kordex::bindings::Module::create(
        "main",
        kordex::bindings::ModuleKind::Script);

    const auto register_error = registry.register_module(module);
    auto found_module = registry.module("main");

    if (!expect_true(
            !register_error,
            "registering module should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(found_module),
            "getting module should succeed"))
    {
      return false;
    }

    return expect_true(
               registry.has_module("main"),
               "registry should have module") &&
           expect_true(
               registry.size() == 1,
               "registry size should be one") &&
           expect_true(
               std::string_view(found_module.value().name()) == "main",
               "registered module name should match");
  }

  [[nodiscard]] bool test_register_module_with_explicit_name()
  {
    kordex::bindings::ModuleRegistry registry;

    auto module = kordex::bindings::Module::create(
        "internal",
        kordex::bindings::ModuleKind::Script);

    const auto register_error = registry.register_module(
        "alias",
        module);

    return expect_true(
               !register_error,
               "registering module with explicit name should succeed") &&
           expect_true(
               registry.has_module("alias"),
               "registry should store explicit name") &&
           expect_true(
               !registry.has_module("internal"),
               "registry should not store internal name automatically");
  }

  [[nodiscard]] bool test_register_native_module()
  {
    kordex::bindings::ModuleRegistry registry;

    auto native_module = kordex::bindings::NativeModule::create("console");

    const auto value_error = native_module.set_value(
        "version",
        kordex::bindings::Value::string("0.1.0"));

    const auto register_error = registry.register_native_module(native_module);
    auto found_module = registry.module("console");

    if (!expect_true(
            !value_error && !register_error,
            "native module setup and registration should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(found_module),
            "getting native module should succeed"))
    {
      return false;
    }

    return expect_true(
               found_module.value().kind() ==
                   kordex::bindings::ModuleKind::Native,
               "registered native module should have native kind") &&
           expect_true(
               found_module.value().loaded(),
               "registered native module should be loaded") &&
           expect_true(
               found_module.value().has_export("version"),
               "registered native module should expose values");
  }

  [[nodiscard]] bool test_import_module()
  {
    kordex::bindings::ModuleRegistry registry;

    auto module = kordex::bindings::Module::create(
        "config",
        kordex::bindings::ModuleKind::Script);

    const auto register_error = registry.register_module(module);
    auto imported_module = registry.import_module("config");

    if (!expect_true(
            !register_error,
            "registering importable module should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(imported_module),
            "importing module should succeed"))
    {
      return false;
    }

    return expect_true(
        std::string_view(imported_module.value().name()) == "config",
        "imported module name should match");
  }

  [[nodiscard]] bool test_import_not_importable_module()
  {
    kordex::bindings::ModuleRegistry registry;

    kordex::bindings::ModuleInfo info;
    info.name = "private";
    info.kind = kordex::bindings::ModuleKind::Script;
    info.importable = false;

    auto module = kordex::bindings::Module::create(info);

    const auto register_error = registry.register_module(module);
    auto imported_module = registry.import_module("private");

    return expect_true(
               !register_error,
               "registering non-importable module should succeed") &&
           expect_true(
               !imported_module,
               "importing non-importable module should fail") &&
           expect_true(
               imported_module.error().code() ==
                   kordex::bindings::ErrorCode::PermissionDenied,
               "non-importable module should map to permission denied");
  }

  [[nodiscard]] bool test_missing_module()
  {
    const kordex::bindings::ModuleRegistry registry;

    auto found_module = registry.module("missing");
    auto imported_module = registry.import_module("missing");
    const auto remove_error = const_cast<kordex::bindings::ModuleRegistry &>(registry)
                                  .remove_module("missing");

    return expect_true(
               !found_module,
               "missing module lookup should fail") &&
           expect_true(
               !imported_module,
               "missing module import should fail") &&
           expect_true(
               remove_error.has_error(),
               "missing module remove should fail") &&
           expect_true(
               found_module.error().code() ==
                   kordex::bindings::ErrorCode::NotFound,
               "missing module lookup should map to not found");
  }

  [[nodiscard]] bool test_invalid_name()
  {
    kordex::bindings::ModuleRegistry registry;

    auto module = kordex::bindings::Module::create(
        "main",
        kordex::bindings::ModuleKind::Script);

    const auto register_error = registry.register_module("", module);
    auto found_module = registry.module("");
    auto imported_module = registry.import_module("");
    const auto remove_error = registry.remove_module("");

    return expect_true(
               register_error.has_error(),
               "registering empty module name should fail") &&
           expect_true(
               !found_module,
               "looking up empty module name should fail") &&
           expect_true(
               !imported_module,
               "importing empty module name should fail") &&
           expect_true(
               remove_error.has_error(),
               "removing empty module name should fail") &&
           expect_true(
               register_error.code() ==
                   kordex::bindings::ErrorCode::InvalidArgument,
               "empty module name should map to invalid argument");
  }

  [[nodiscard]] bool test_invalid_module()
  {
    kordex::bindings::ModuleRegistry registry;

    const kordex::bindings::Module module;

    const auto register_error = registry.register_module(module);

    return expect_true(
               register_error.has_error(),
               "registering invalid module should fail") &&
           expect_true(
               register_error.code() ==
                   kordex::bindings::ErrorCode::InternalError,
               "invalid module should map to internal error");
  }

  [[nodiscard]] bool test_overwrite_policy()
  {
    kordex::bindings::ModuleRegistryOptions options;
    options.allow_overwrite = false;

    kordex::bindings::ModuleRegistry registry(options);

    auto first_module = kordex::bindings::Module::create(
        "main",
        kordex::bindings::ModuleKind::Script);

    auto second_module = kordex::bindings::Module::create(
        "main",
        kordex::bindings::ModuleKind::Script);

    const auto first_error = registry.register_module(first_module);
    const auto second_error = registry.register_module(second_module);

    return expect_true(
               !first_error,
               "first registration should succeed") &&
           expect_true(
               second_error.has_error(),
               "second registration should fail when overwrite is disabled") &&
           expect_true(
               second_error.code() ==
                   kordex::bindings::ErrorCode::InternalError,
               "overwrite failure should map to internal error");
  }

  [[nodiscard]] bool test_module_kind_policy()
  {
    kordex::bindings::ModuleRegistryOptions options;
    options.allow_native_modules = false;
    options.allow_script_modules = false;
    options.allow_builtin_modules = false;

    kordex::bindings::ModuleRegistry registry(options);

    auto native_module = kordex::bindings::Module::create(
        "native",
        kordex::bindings::ModuleKind::Native);

    auto script_module = kordex::bindings::Module::create(
        "script",
        kordex::bindings::ModuleKind::Script);

    auto builtin_module = kordex::bindings::Module::create(
        "builtin",
        kordex::bindings::ModuleKind::Builtin);

    const auto native_error = registry.register_module(native_module);
    const auto script_error = registry.register_module(script_module);
    const auto builtin_error = registry.register_module(builtin_module);

    return expect_true(
               native_error.has_error(),
               "native module should be rejected") &&
           expect_true(
               script_error.has_error(),
               "script module should be rejected") &&
           expect_true(
               builtin_error.has_error(),
               "builtin module should be rejected") &&
           expect_true(
               native_error.code() ==
                   kordex::bindings::ErrorCode::PermissionDenied,
               "native rejection should map to permission denied") &&
           expect_true(
               script_error.code() ==
                   kordex::bindings::ErrorCode::PermissionDenied,
               "script rejection should map to permission denied") &&
           expect_true(
               builtin_error.code() ==
                   kordex::bindings::ErrorCode::PermissionDenied,
               "builtin rejection should map to permission denied");
  }

  [[nodiscard]] bool test_names_and_names_by_kind()
  {
    kordex::bindings::ModuleRegistry registry;

    auto native_module = kordex::bindings::Module::create(
        "native",
        kordex::bindings::ModuleKind::Native);

    auto script_module = kordex::bindings::Module::create(
        "script",
        kordex::bindings::ModuleKind::Script);

    auto builtin_module = kordex::bindings::Module::create(
        "builtin",
        kordex::bindings::ModuleKind::Builtin);

    const auto native_error = registry.register_module(native_module);
    const auto script_error = registry.register_module(script_module);
    const auto builtin_error = registry.register_module(builtin_module);

    const auto names = registry.names();
    const auto native_names = registry.names_by_kind(
        kordex::bindings::ModuleKind::Native);
    const auto script_names = registry.names_by_kind(
        kordex::bindings::ModuleKind::Script);
    const auto builtin_names = registry.names_by_kind(
        kordex::bindings::ModuleKind::Builtin);

    return expect_true(
               !native_error && !script_error && !builtin_error,
               "registering modules should succeed") &&
           expect_true(
               names.size() == 3,
               "registry names size should be three") &&
           expect_true(
               names[0] == "builtin" &&
                   names[1] == "native" &&
                   names[2] == "script",
               "registry names should be sorted by map order") &&
           expect_true(
               native_names.size() == 1 && native_names[0] == "native",
               "native names should match") &&
           expect_true(
               script_names.size() == 1 && script_names[0] == "script",
               "script names should match") &&
           expect_true(
               builtin_names.size() == 1 && builtin_names[0] == "builtin",
               "builtin names should match");
  }

  [[nodiscard]] bool test_remove_and_clear()
  {
    kordex::bindings::ModuleRegistry registry;

    auto first_module = kordex::bindings::Module::create(
        "a",
        kordex::bindings::ModuleKind::Script);

    auto second_module = kordex::bindings::Module::create(
        "b",
        kordex::bindings::ModuleKind::Script);

    const auto first_error = registry.register_module(first_module);
    const auto second_error = registry.register_module(second_module);

    const auto remove_error = registry.remove_module("a");

    const bool after_remove =
        !registry.has_module("a") &&
        registry.has_module("b") &&
        registry.size() == 1;

    registry.clear();

    return expect_true(
               !first_error && !second_error,
               "registering modules should succeed") &&
           expect_true(
               !remove_error,
               "removing module should succeed") &&
           expect_true(
               after_remove,
               "registry should contain only b after remove") &&
           expect_true(
               registry.empty(),
               "registry should be empty after clear") &&
           expect_true(
               registry.size() == 0,
               "registry size should be zero after clear");
  }

  [[nodiscard]] bool test_set_options()
  {
    kordex::bindings::ModuleRegistry registry;

    kordex::bindings::ModuleRegistryOptions options;
    options.allow_script_modules = false;

    registry.set_options(options);

    auto module = kordex::bindings::Module::create(
        "script",
        kordex::bindings::ModuleKind::Script);

    const auto register_error = registry.register_module(module);

    return expect_true(
               !registry.options().allow_script_modules,
               "registry options should be updated") &&
           expect_true(
               register_error.has_error(),
               "script registration should fail after options update") &&
           expect_true(
               register_error.code() ==
                   kordex::bindings::ErrorCode::PermissionDenied,
               "script registration failure should map to permission denied");
  }

} // namespace

int main()
{
  const bool ok =
      test_empty_registry() &&
      test_register_and_get_module() &&
      test_register_module_with_explicit_name() &&
      test_register_native_module() &&
      test_import_module() &&
      test_import_not_importable_module() &&
      test_missing_module() &&
      test_invalid_name() &&
      test_invalid_module() &&
      test_overwrite_policy() &&
      test_module_kind_policy() &&
      test_names_and_names_by_kind() &&
      test_remove_and_clear() &&
      test_set_options();

  return ok ? 0 : 1;
}
