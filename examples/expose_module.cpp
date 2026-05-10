/**
 *
 *  @file expose_module.cpp
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

#include <iostream>
#include <kordex/bindings/Bindings.hpp>

int main()
{
  auto engine_result = kordex::bindings::create_engine();
  if (!engine_result)
  {
    std::cerr << "failed to create engine: "
              << engine_result.error().message()
              << '\n';
    return 1;
  }

  auto engine = std::move(engine_result.value());

  auto init_result = engine.initialize();
  if (!init_result.succeeded())
  {
    std::cerr << "failed to initialize engine: "
              << init_result.error.message()
              << '\n';
    return init_result.exit_code;
  }

  auto module = kordex::bindings::NativeModule::create("math");

  auto set_version_error = module.set_value(
      "version",
      kordex::bindings::Value::string("0.1.0"));

  if (set_version_error)
  {
    std::cerr << "failed to set module value: "
              << set_version_error.message()
              << '\n';
    return 1;
  }

  kordex::bindings::NativeFunctionInfo add_info;
  add_info.name = "add";
  add_info.module_name = "math";
  add_info.description = "Add two numbers";
  add_info.min_args = 2;
  add_info.max_args = 2;

  auto add = kordex::bindings::NativeFunction::create(
      add_info,
      [](const kordex::bindings::NativeFunctionArguments &args)
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

  auto add_function_error = module.add_function(add);
  if (add_function_error)
  {
    std::cerr << "failed to add module function: "
              << add_function_error.message()
              << '\n';
    return 1;
  }

  auto register_error = engine.register_native_module(module);
  if (register_error)
  {
    std::cerr << "failed to register native module: "
              << register_error.message()
              << '\n';
    return 1;
  }

  auto imported = engine.import_module("math");
  if (!imported)
  {
    std::cerr << "failed to import module: "
              << imported.error().message()
              << '\n';
    return 1;
  }

  auto version = imported.value().export_value("version");
  if (!version)
  {
    std::cerr << "failed to read module version: "
              << version.error().message()
              << '\n';
    return 1;
  }

  auto version_text = version.value().as_string();
  if (!version_text)
  {
    std::cerr << "failed to convert module version: "
              << version_text.error().message()
              << '\n';
    return 1;
  }

  kordex::bindings::FunctionArguments args{
      kordex::bindings::Value::number(20.0),
      kordex::bindings::Value::number(22.0)};

  auto call_result = imported.value().call("add", args);
  if (!call_result)
  {
    std::cerr << "failed to call module function: "
              << call_result.error().message()
              << '\n';
    return 1;
  }

  auto number = call_result.value().as_number();
  if (!number)
  {
    std::cerr << "failed to read function result: "
              << number.error().message()
              << '\n';
    return 1;
  }

  std::cout << "module        = " << imported.value().name() << '\n';
  std::cout << "module kind   = "
            << kordex::bindings::to_string(imported.value().kind())
            << '\n';
  std::cout << "math.version  = " << version_text.value() << '\n';
  std::cout << "math.add(...) = " << number.value() << '\n';

  (void)engine.shutdown();

  return 0;
}
