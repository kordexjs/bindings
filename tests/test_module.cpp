/**
 *
 *  @file test_module.cpp
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

#include <iostream>
#include <string_view>
#include <utility>

#include <kordex/bindings/Module.hpp>

namespace
{
  [[nodiscard]] bool expect_true(
      bool condition,
      const char *message)
  {
    if (!condition)
    {
      ::std::cerr << "[test_module] failed: "
                  << (message ? message : "unknown assertion")
                  << '\n';

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

  [[nodiscard]] bool test_module_kind_strings()
  {
    return expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::ModuleKind::Unknown)) == "unknown",
               "unknown module kind string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::ModuleKind::Native)) == "native",
               "native module kind string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::ModuleKind::Script)) == "script",
               "script module kind string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::ModuleKind::Builtin)) == "builtin",
               "builtin module kind string should match");
  }

  [[nodiscard]] bool test_module_info_helpers()
  {
    kordex::bindings::ModuleInfo info;
    info.name = "main";
    info.path = "main.js";
    info.description = "Main module";

    return expect_true(
               info.has_name(),
               "module info should have name") &&
           expect_true(
               info.has_path(),
               "module info should have path") &&
           expect_true(
               info.has_description(),
               "module info should have description");
  }

  [[nodiscard]] bool test_empty_module()
  {
    const kordex::bindings::Module module;

    return expect_true(
               !module.valid(),
               "default module should not be valid") &&
           expect_true(
               !module.importable(),
               "default module should not be importable") &&
           expect_true(
               !module.loaded(),
               "default module should not be loaded") &&
           expect_true(
               module.empty(),
               "default module should be empty") &&
           expect_true(
               module.export_count() == 0,
               "default export count should be zero") &&
           expect_true(
               module.function_count() == 0,
               "default function count should be zero");
  }

  [[nodiscard]] bool test_create_module()
  {
    auto module = kordex::bindings::Module::create(
        "main",
        kordex::bindings::ModuleKind::Script);

    const bool initially_unloaded = !module.loaded();

    module.mark_loaded();

    const bool loaded = module.loaded();

    module.mark_unloaded();

    return expect_true(
               module.valid(),
               "created module should be valid") &&
           expect_true(
               module.importable(),
               "created module should be importable") &&
           expect_true(
               initially_unloaded,
               "created module should start unloaded") &&
           expect_true(
               loaded,
               "module should be marked loaded") &&
           expect_true(
               !module.loaded(),
               "module should be marked unloaded") &&
           expect_true(
               std::string_view(module.name()) == "main",
               "module name should match") &&
           expect_true(
               module.kind() == kordex::bindings::ModuleKind::Script,
               "module kind should match");
  }

  [[nodiscard]] bool test_create_module_from_info()
  {
    kordex::bindings::ModuleInfo info;
    info.name = "builtin:console";
    info.path = "kordex:console";
    info.kind = kordex::bindings::ModuleKind::Builtin;
    info.description = "Console builtin";
    info.loaded = true;

    auto module = kordex::bindings::Module::create(info);

    return expect_true(
               module.valid(),
               "module from info should be valid") &&
           expect_true(
               module.loaded(),
               "module from info should be loaded") &&
           expect_true(
               std::string_view(module.path()) == "kordex:console",
               "module path should match") &&
           expect_true(
               module.kind() == kordex::bindings::ModuleKind::Builtin,
               "module kind should match");
  }

  [[nodiscard]] bool test_set_and_get_export()
  {
    auto module = kordex::bindings::Module::create("config");

    const auto set_error = module.set_export(
        "version",
        kordex::bindings::Value::string("0.1.0"));

    auto value = module.export_value("version");

    if (!expect_true(
            !set_error,
            "setting module export should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(value),
            "getting module export should succeed"))
    {
      return false;
    }

    auto string_value = value.value().as_string();

    if (!expect_true(
            result_ok(string_value),
            "module export should convert to string"))
    {
      return false;
    }

    return expect_true(
               module.has_export("version"),
               "module should have export") &&
           expect_true(
               module.export_count() == 1,
               "module export count should be one") &&
           expect_true(
               string_value.value() == "0.1.0",
               "module export value should match");
  }

  [[nodiscard]] bool test_add_and_get_function()
  {
    auto module = kordex::bindings::Module::create("math");

    auto function = kordex::bindings::Function::create(
        "answer",
        [](const kordex::bindings::FunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::number(42.0);
        });

    const auto add_error = module.add_function(
        ::std::move(function));

    auto stored_function = module.function("answer");

    if (!expect_true(
            !add_error,
            "adding function should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(stored_function),
            "getting function should succeed"))
    {
      return false;
    }

    auto call_result = stored_function.value().call();

    if (!expect_true(
            result_ok(call_result),
            "stored function call should succeed"))
    {
      return false;
    }

    auto number = call_result.value().as_number();

    if (!expect_true(
            result_ok(number),
            "stored function result should convert to number"))
    {
      return false;
    }

    return expect_true(
               module.has_function("answer"),
               "module should have function") &&
           expect_true(
               module.function_count() == 1,
               "module function count should be one") &&
           expect_true(
               number.value() == 42.0,
               "stored function should return 42");
  }

  [[nodiscard]] bool test_set_function_with_explicit_name()
  {
    auto module = kordex::bindings::Module::create("math");

    auto function = kordex::bindings::Function::create(
        "internal_answer",
        [](const kordex::bindings::FunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::number(42.0);
        });

    const auto add_error = module.set_function(
        "answer",
        ::std::move(function));

    return expect_true(
               !add_error,
               "setting function with explicit name should succeed") &&
           expect_true(
               module.has_function("answer"),
               "module should store explicit function name") &&
           expect_true(
               !module.has_function("internal_answer"),
               "module should not store internal name automatically");
  }

  [[nodiscard]] bool test_call_function_by_name()
  {
    auto module = kordex::bindings::Module::create("math");

    kordex::bindings::FunctionInfo info;
    info.name = "add";
    info.min_args = 2;
    info.max_args = 2;

    auto function = kordex::bindings::Function::create(
        info,
        [](const kordex::bindings::FunctionArguments &args)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          auto left = args[0].as_number();
          if (!left)
          {
            return left.error();
          }

          auto right = args[1].as_number();
          if (!right)
          {
            return right.error();
          }

          return kordex::bindings::Value::number(
              left.value() + right.value());
        });

    const auto add_error = module.add_function(
        ::std::move(function));

    kordex::bindings::FunctionArguments args{
        kordex::bindings::Value::number(20.0),
        kordex::bindings::Value::number(22.0)};

    auto result = module.call("add", args);

    if (!expect_true(
            !add_error,
            "adding add function should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(result),
            "module function call should succeed"))
    {
      return false;
    }

    auto number = result.value().as_number();

    if (!expect_true(
            result_ok(number),
            "module function call result should convert to number"))
    {
      return false;
    }

    return expect_true(
        number.value() == 42.0,
        "module function call should return 42");
  }

  [[nodiscard]] bool test_missing_entries()
  {
    auto module = kordex::bindings::Module::create("empty");

    auto missing_export = module.export_value("missing_export");
    auto missing_function = module.function("missing_function");
    auto missing_call = module.call("missing_function");

    return expect_true(
               !missing_export,
               "missing export should fail") &&
           expect_true(
               !missing_function,
               "missing function should fail") &&
           expect_true(
               !missing_call,
               "missing call should fail") &&
           expect_true(
               missing_export.error().code() ==
                   kordex::bindings::ErrorCode::NotFound,
               "missing export should map to not found") &&
           expect_true(
               missing_function.error().code() ==
                   kordex::bindings::ErrorCode::NotFound,
               "missing function should map to not found");
  }

  [[nodiscard]] bool test_invalid_names()
  {
    auto module = kordex::bindings::Module::create("test");

    const auto set_export_error = module.set_export(
        "",
        kordex::bindings::Value::number(1.0));

    auto get_export = module.export_value("");
    const auto remove_export_error = module.remove_export("");

    auto function = kordex::bindings::Function::create(
        "valid",
        [](const kordex::bindings::FunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::undefined();
        });

    const auto set_function_error = module.set_function(
        "",
        ::std::move(function));
    auto get_function = module.function("");
    const auto remove_function_error = module.remove_function("");

    return expect_true(
               set_export_error.has_error(),
               "set export with empty name should fail") &&
           expect_true(
               !get_export,
               "get export with empty name should fail") &&
           expect_true(
               remove_export_error.has_error(),
               "remove export with empty name should fail") &&
           expect_true(
               set_function_error.has_error(),
               "set function with empty name should fail") &&
           expect_true(
               !get_function,
               "get function with empty name should fail") &&
           expect_true(
               remove_function_error.has_error(),
               "remove function with empty name should fail") &&
           expect_true(
               set_export_error.code() ==
                   kordex::bindings::ErrorCode::InvalidArgument,
               "empty export name should map to invalid argument") &&
           expect_true(
               set_function_error.code() ==
                   kordex::bindings::ErrorCode::InvalidArgument,
               "empty function name should map to invalid argument");
  }

  [[nodiscard]] bool test_remove_entries()
  {
    auto module = kordex::bindings::Module::create("test");

    const auto export_error = module.set_export(
        "version",
        kordex::bindings::Value::string("0.1.0"));

    auto function = kordex::bindings::Function::create(
        "hello",
        [](const kordex::bindings::FunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::string("hello");
        });

    const auto function_error = module.add_function(
        ::std::move(function));

    const auto remove_export_error = module.remove_export("version");
    const auto remove_function_error = module.remove_function("hello");

    return expect_true(
               !export_error && !function_error,
               "setup entries should succeed") &&
           expect_true(
               !remove_export_error,
               "remove export should succeed") &&
           expect_true(
               !remove_function_error,
               "remove function should succeed") &&
           expect_true(
               !module.has_export("version"),
               "removed export should be missing") &&
           expect_true(
               !module.has_function("hello"),
               "removed function should be missing") &&
           expect_true(
               module.empty(),
               "module should be empty after removals");
  }

  [[nodiscard]] bool test_names_and_clear()
  {
    auto module = kordex::bindings::Module::create("test");

    const auto export_error = module.set_export(
        "a",
        kordex::bindings::Value::number(1.0));

    auto function = kordex::bindings::Function::create(
        "b",
        [](const kordex::bindings::FunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::undefined();
        });

    const auto function_error = module.add_function(
        ::std::move(function));

    const auto export_names = module.export_names();
    const auto function_names = module.function_names();

    const bool names_ok =
        export_names.size() == 1 &&
        export_names[0] == "a" &&
        function_names.size() == 1 &&
        function_names[0] == "b";

    module.clear();

    return expect_true(
               !export_error && !function_error,
               "adding entries should succeed") &&
           expect_true(
               names_ok,
               "entry names should match") &&
           expect_true(
               module.empty(),
               "module should be empty after clear") &&
           expect_true(
               module.export_count() == 0,
               "export count should be zero after clear") &&
           expect_true(
               module.function_count() == 0,
               "function count should be zero after clear");
  }

  [[nodiscard]] bool test_from_native_module()
  {
    auto native_module = kordex::bindings::NativeModule::create("console");

    const auto value_error = native_module.set_value(
        "version",
        kordex::bindings::Value::string("0.1.0"));

    auto native_function = kordex::bindings::NativeFunction::create(
        "log",
        [](const kordex::bindings::NativeFunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::undefined();
        });

    const auto function_error = native_module.add_function(
        ::std::move(native_function));

    auto module = kordex::bindings::Module::from_native(native_module);

    return expect_true(
               !value_error && !function_error,
               "native module setup should succeed") &&
           expect_true(
               module.valid(),
               "module from native should be valid") &&
           expect_true(
               module.loaded(),
               "module from native should be loaded") &&
           expect_true(
               module.kind() == kordex::bindings::ModuleKind::Native,
               "module from native should have native kind") &&
           expect_true(
               module.has_export("version"),
               "module from native should contain value") &&
           expect_true(
               module.has_function("log"),
               "module from native should contain function");
  }

  [[nodiscard]] bool test_to_object()
  {
    auto module = kordex::bindings::Module::create("console");

    const auto export_error = module.set_export(
        "version",
        kordex::bindings::Value::string("0.1.0"));

    auto function = kordex::bindings::Function::create(
        "log",
        [](const kordex::bindings::FunctionArguments &)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          return kordex::bindings::Value::undefined();
        });

    const auto function_error = module.add_function(
        ::std::move(function));

    const auto object = module.to_object();

    auto version = object.get("version");
    auto log = object.get("log");

    if (!expect_true(
            !export_error && !function_error,
            "module setup should succeed"))
    {
      return false;
    }

    if (!expect_true(
            result_ok(version) && result_ok(log),
            "module object properties should exist"))
    {
      return false;
    }

    return expect_true(
               std::string_view(object.name()) == "console",
               "module object name should match") &&
           expect_true(
               object.has("version"),
               "module object should contain export") &&
           expect_true(
               object.has("log"),
               "module object should contain function placeholder") &&
           expect_true(
               std::string_view(log.value().display()) == "[function]",
               "function placeholder should match");
  }

} // namespace

int main()
{
  const bool ok =
      test_module_kind_strings() &&
      test_module_info_helpers() &&
      test_empty_module() &&
      test_create_module() &&
      test_create_module_from_info() &&
      test_set_and_get_export() &&
      test_add_and_get_function() &&
      test_set_function_with_explicit_name() &&
      test_call_function_by_name() &&
      test_missing_entries() &&
      test_invalid_names() &&
      test_remove_entries() &&
      test_names_and_clear() &&
      test_from_native_module() &&
      test_to_object();

  return ok ? 0 : 1;
}
