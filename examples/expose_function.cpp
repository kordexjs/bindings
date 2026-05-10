/**
 *
 *  @file expose_function.cpp
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

  auto add = kordex::bindings::Function::create(
      "add",
      [](const kordex::bindings::FunctionArguments &args)
          -> kordex::bindings::Result<kordex::bindings::Value>
      {
        if (args.size() != 2)
        {
          return kordex::bindings::make_binding_error(
              kordex::bindings::BindingErrorCode::InvalidArgument,
              "add expects exactly two arguments");
        }

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

  auto add_error = engine.add_global_function(add);
  if (add_error)
  {
    std::cerr << "failed to expose function: "
              << add_error.message()
              << '\n';
    return 1;
  }

  kordex::bindings::FunctionArguments args{
      kordex::bindings::Value::number(20.0),
      kordex::bindings::Value::number(22.0)};

  auto call_result = engine.call_global_function("add", args);
  if (!call_result)
  {
    std::cerr << "failed to call function: "
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

  std::cout << "add(20, 22) = "
            << number.value()
            << '\n';

  (void)engine.shutdown();

  return 0;
}
