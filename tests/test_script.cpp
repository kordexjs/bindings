/**
 *
 *  @file test_script.cpp
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
#include <kordex/bindings/Script.hpp>

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

  [[nodiscard]] bool test_script_type_strings()
  {
    return expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::ScriptType::Unknown)) == "unknown",
               "unknown script type string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::ScriptType::JavaScript)) == "javascript",
               "javascript script type string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::ScriptType::TypeScript)) == "typescript",
               "typescript script type string should match") &&
           expect_true(
               std::string_view(kordex::bindings::to_string(
                   kordex::bindings::ScriptType::Json)) == "json",
               "json script type string should match");
  }

  [[nodiscard]] bool test_detect_script_type()
  {
    return expect_true(
               kordex::bindings::detect_script_type("main.js") ==
                   kordex::bindings::ScriptType::JavaScript,
               "js file should be detected as javascript") &&
           expect_true(
               kordex::bindings::detect_script_type("main.mjs") ==
                   kordex::bindings::ScriptType::JavaScript,
               "mjs file should be detected as javascript") &&
           expect_true(
               kordex::bindings::detect_script_type("main.cjs") ==
                   kordex::bindings::ScriptType::JavaScript,
               "cjs file should be detected as javascript") &&
           expect_true(
               kordex::bindings::detect_script_type("main.ts") ==
                   kordex::bindings::ScriptType::TypeScript,
               "ts file should be detected as typescript") &&
           expect_true(
               kordex::bindings::detect_script_type("main.mts") ==
                   kordex::bindings::ScriptType::TypeScript,
               "mts file should be detected as typescript") &&
           expect_true(
               kordex::bindings::detect_script_type("main.cts") ==
                   kordex::bindings::ScriptType::TypeScript,
               "cts file should be detected as typescript") &&
           expect_true(
               kordex::bindings::detect_script_type("package.json") ==
                   kordex::bindings::ScriptType::Json,
               "json file should be detected as json") &&
           expect_true(
               kordex::bindings::detect_script_type("README.md") ==
                   kordex::bindings::ScriptType::Unknown,
               "unknown extension should be unknown");
  }

  [[nodiscard]] bool test_from_runtime_source_type()
  {
    return expect_true(
               kordex::bindings::from_runtime_source_type(
                   kordex::runtime::SourceType::JavaScript) ==
                   kordex::bindings::ScriptType::JavaScript,
               "runtime javascript should convert to script javascript") &&
           expect_true(
               kordex::bindings::from_runtime_source_type(
                   kordex::runtime::SourceType::TypeScript) ==
                   kordex::bindings::ScriptType::TypeScript,
               "runtime typescript should convert to script typescript") &&
           expect_true(
               kordex::bindings::from_runtime_source_type(
                   kordex::runtime::SourceType::Json) ==
                   kordex::bindings::ScriptType::Json,
               "runtime json should convert to script json") &&
           expect_true(
               kordex::bindings::from_runtime_source_type(
                   kordex::runtime::SourceType::Unknown) ==
                   kordex::bindings::ScriptType::Unknown,
               "runtime unknown should convert to script unknown");
  }

  [[nodiscard]] bool test_script_info_helpers()
  {
    kordex::bindings::ScriptInfo info;
    info.path = "src/main.js";
    info.name = "main.js";
    info.type = kordex::bindings::ScriptType::JavaScript;

    return expect_true(
               info.has_path(),
               "script info should have path") &&
           expect_true(
               info.has_name(),
               "script info should have name") &&
           expect_true(
               info.executable(),
               "javascript script info should be executable");
  }

  [[nodiscard]] bool test_default_script()
  {
    const kordex::bindings::Script script;

    return expect_true(
               !script.valid(),
               "default script should not be valid") &&
           expect_true(
               script.empty(),
               "default script should be empty") &&
           expect_true(
               script.size() == 0,
               "default script size should be zero") &&
           expect_true(
               script.type() == kordex::bindings::ScriptType::Unknown,
               "default script type should be unknown") &&
           expect_true(
               !script.loaded_from_file(),
               "default script should not be loaded from file") &&
           expect_true(
               !script.compiled(),
               "default script should not be compiled") &&
           expect_true(
               !script.executed(),
               "default script should not be executed") &&
           expect_true(
               !script.executable(),
               "default script should not be executable");
  }

  [[nodiscard]] bool test_from_source()
  {
    auto script = kordex::bindings::Script::from_source(
        "console.log('hello');",
        "hello.js");

    return expect_true(
               script.valid(),
               "script from source should be valid") &&
           expect_true(
               !script.empty(),
               "script from source should not be empty") &&
           expect_true(
               script.size() == std::string_view("console.log('hello');").size(),
               "script size should match source size") &&
           expect_true(
               std::string_view(script.name()) == "hello.js",
               "script name should match") &&
           expect_true(
               script.type() == kordex::bindings::ScriptType::JavaScript,
               "script from source should default to javascript") &&
           expect_true(
               script.executable(),
               "script from source should be executable") &&
           expect_true(
               !script.loaded_from_file(),
               "script from source should not be loaded from file");
  }

  [[nodiscard]] bool test_from_source_with_info()
  {
    kordex::bindings::ScriptInfo info;
    info.path = "src/main.ts";
    info.type = kordex::bindings::ScriptType::Unknown;
    info.loaded_from_file = false;

    auto script = kordex::bindings::Script::from_source(
        "const answer: number = 42;",
        info);

    return expect_true(
               script.valid(),
               "script from source with info should be valid") &&
           expect_true(
               std::string_view(script.path()) == "src/main.ts",
               "script path should match") &&
           expect_true(
               std::string_view(script.name()) == "main.ts",
               "script name should be derived from path") &&
           expect_true(
               script.type() == kordex::bindings::ScriptType::TypeScript,
               "script type should be detected from path") &&
           expect_true(
               script.executable(),
               "typescript script should be executable");
  }

  [[nodiscard]] bool test_setters_reset_state()
  {
    auto script = kordex::bindings::Script::from_source(
        "console.log('hello');",
        "hello.js");

    script.mark_compiled();
    script.mark_executed();

    const bool initially_compiled_and_executed =
        script.compiled() && script.executed();

    script.set_source("console.log('updated');");

    const bool source_reset_state =
        !script.compiled() && !script.executed();

    script.set_type(kordex::bindings::ScriptType::TypeScript);

    return expect_true(
               initially_compiled_and_executed,
               "script should be manually marked compiled and executed") &&
           expect_true(
               source_reset_state,
               "set_source should reset compiled and executed state") &&
           expect_true(
               script.type() == kordex::bindings::ScriptType::TypeScript,
               "set_type should update script type") &&
           expect_true(
               !script.compiled(),
               "set_type should reset compiled state") &&
           expect_true(
               !script.executed(),
               "set_type should reset executed state");
  }

  [[nodiscard]] bool test_set_path_and_name()
  {
    kordex::bindings::Script script;

    script.set_source("console.log('hello');");
    script.set_path("app/index.mjs");

    const bool path_and_name_ok =
        std::string_view(script.path()) == "app/index.mjs" &&
        std::string_view(script.name()) == "index.mjs" &&
        script.type() == kordex::bindings::ScriptType::JavaScript;

    script.set_name("custom-name");

    return expect_true(
               path_and_name_ok,
               "set_path should update path, name, and type") &&
           expect_true(
               std::string_view(script.name()) == "custom-name",
               "set_name should update script name");
  }

  [[nodiscard]] bool test_validate_success()
  {
    auto script = kordex::bindings::Script::from_source(
        "console.log('hello');",
        "hello.js");

    const auto error = script.validate();

    return expect_true(
        !error,
        "valid script should pass validation");
  }

  [[nodiscard]] bool test_validate_empty_source()
  {
    kordex::bindings::ScriptInfo info;
    info.name = "empty.js";
    info.type = kordex::bindings::ScriptType::JavaScript;

    const kordex::bindings::Script script("", info);

    const auto error = script.validate();

    return expect_true(
               error.has_error(),
               "empty script source should fail validation") &&
           expect_true(
               error.code() ==
                   kordex::bindings::ErrorCode::InvalidArgument,
               "empty source validation should map to invalid argument");
  }

  [[nodiscard]] bool test_validate_unknown_type()
  {
    kordex::bindings::ScriptInfo info;
    info.name = "unknown";
    info.type = kordex::bindings::ScriptType::Unknown;

    const kordex::bindings::Script script("content", info);

    const auto error = script.validate();

    return expect_true(
               error.has_error(),
               "unknown script type should fail validation") &&
           expect_true(
               error.code() ==
                   kordex::bindings::ErrorCode::InvalidArgument,
               "unknown type validation should map to invalid argument");
  }

  [[nodiscard]] bool test_compile_success()
  {
    auto script = kordex::bindings::Script::from_source(
        "console.log('hello');",
        "hello.js");

    auto result = script.compile();

    return expect_true(
               result.succeeded(),
               "compile should succeed") &&
           expect_true(
               script.compiled(),
               "compile should mark script as compiled") &&
           expect_true(
               !script.executed(),
               "compile should not mark script as executed") &&
           expect_true(
               std::string_view(result.output) ==
                   "script compiled successfully",
               "compile output should match");
  }

  [[nodiscard]] bool test_compile_failure()
  {
    kordex::bindings::ScriptInfo info;
    info.name = "empty.js";
    info.type = kordex::bindings::ScriptType::JavaScript;

    kordex::bindings::Script script("", info);

    auto result = script.compile();

    return expect_true(
               result.failed(),
               "compile should fail for invalid script") &&
           expect_true(
               !script.compiled(),
               "failed compile should not mark script as compiled") &&
           expect_true(
               result.error.has_error(),
               "failed compile should contain error") &&
           expect_true(
               result.error.code() ==
                   kordex::bindings::ErrorCode::InternalError,
               "compile failure should map to internal error");
  }

  [[nodiscard]] bool test_run_success()
  {
    auto script = kordex::bindings::Script::from_source(
        "console.log('hello');",
        "hello.js");

    auto result = script.run();

    if (!expect_true(
            result.succeeded(),
            "run should succeed"))
    {
      return false;
    }

    auto value = result.value.as_string();

    if (!expect_true(
            result_ok(value),
            "run result should convert to string"))
    {
      return false;
    }

    return expect_true(
               script.compiled(),
               "run should compile script if needed") &&
           expect_true(
               script.executed(),
               "run should mark script as executed") &&
           expect_true(
               result.has_value(),
               "run result should contain value") &&
           expect_true(
               value.value() == "console.log('hello');",
               "run placeholder should return source as value") &&
           expect_true(
               std::string_view(result.output) ==
                   "script executed by native placeholder",
               "run output should match");
  }

  [[nodiscard]] bool test_run_json_fails()
  {
    kordex::bindings::ScriptInfo info;
    info.name = "package.json";
    info.type = kordex::bindings::ScriptType::Json;

    kordex::bindings::Script script("{\"name\":\"kordex\"}", info);

    auto result = script.run();

    return expect_true(
               result.failed(),
               "json script run should fail") &&
           expect_true(
               !script.executed(),
               "json script should not be marked executed") &&
           expect_true(
               result.error.has_error(),
               "json script failure should contain error") &&
           expect_true(
               result.error.code() ==
                   kordex::bindings::ErrorCode::InternalError,
               "json script execution failure should map to internal error");
  }

  [[nodiscard]] bool test_from_runtime_source_file()
  {
    kordex::runtime::SourceFile source_file;
    source_file.original_path = "input.ts";
    source_file.path = "src/input.ts";
    source_file.type = kordex::runtime::SourceType::TypeScript;
    source_file.content = "const value = 42;";

    auto result = kordex::bindings::Script::from_source_file(source_file);

    if (!expect_true(
            result_ok(result),
            "script from runtime source file should succeed"))
    {
      return false;
    }

    const auto &script = result.value();

    return expect_true(
               script.valid(),
               "script from source file should be valid") &&
           expect_true(
               script.loaded_from_file(),
               "script from source file should be marked loaded from file") &&
           expect_true(
               std::string_view(script.path()) == "src/input.ts",
               "script source file path should match") &&
           expect_true(
               std::string_view(script.name()) == "input.ts",
               "script source file name should match") &&
           expect_true(
               script.type() == kordex::bindings::ScriptType::TypeScript,
               "script source file type should match");
  }

  [[nodiscard]] bool test_from_invalid_runtime_source_file()
  {
    kordex::runtime::SourceFile source_file;
    source_file.original_path = "README.md";
    source_file.path = "README.md";
    source_file.type = kordex::runtime::SourceType::Unknown;
    source_file.content = "hello";

    auto result = kordex::bindings::Script::from_source_file(source_file);

    return expect_true(
               !result,
               "invalid runtime source file should fail") &&
           expect_true(
               result.error().code() ==
                   kordex::bindings::ErrorCode::InvalidArgument,
               "invalid runtime source file should map to invalid argument");
  }
} // namespace

int main()
{
  const bool ok =
      test_script_type_strings() &&
      test_detect_script_type() &&
      test_from_runtime_source_type() &&
      test_script_info_helpers() &&
      test_default_script() &&
      test_from_source() &&
      test_from_source_with_info() &&
      test_setters_reset_state() &&
      test_set_path_and_name() &&
      test_validate_success() &&
      test_validate_empty_source() &&
      test_validate_unknown_type() &&
      test_compile_success() &&
      test_compile_failure() &&
      test_run_success() &&
      test_run_json_fails() &&
      test_from_runtime_source_file() &&
      test_from_invalid_runtime_source_file();

  return ok ? 0 : 1;
}
