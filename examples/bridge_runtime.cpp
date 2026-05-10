/**
 *
 *  @file bridge_runtime.cpp
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
  auto bridge_result = kordex::bindings::create_runtime_bridge();
  if (!bridge_result)
  {
    std::cerr << "failed to create runtime bridge: "
              << bridge_result.error().message()
              << '\n';
    return 1;
  }

  auto bridge = std::move(bridge_result.value());

  auto bridge_init = bridge.initialize();
  if (!bridge_init.succeeded())
  {
    std::cerr << "failed to initialize runtime bridge: "
              << bridge_init.error.message()
              << '\n';
    return bridge_init.exit_code;
  }

  auto context_result = kordex::bindings::create_context();
  if (!context_result)
  {
    std::cerr << "failed to create engine context: "
              << context_result.error().message()
              << '\n';
    return 1;
  }

  auto context = std::move(context_result.value());

  const auto context_init = context.initialize();
  if (context_init)
  {
    std::cerr << "failed to initialize engine context: "
              << context_init.message()
              << '\n';
    return 1;
  }

  const auto install_error = bridge.install_into(context);
  if (install_error)
  {
    std::cerr << "failed to install runtime bridge: "
              << install_error.message()
              << '\n';
    return 1;
  }

  auto state = context.global("kordex_runtime_state");
  if (!state)
  {
    std::cerr << "failed to read runtime state: "
              << state.error().message()
              << '\n';
    return 1;
  }

  auto running = context.global("kordex_runtime_running");
  if (!running)
  {
    std::cerr << "failed to read runtime running flag: "
              << running.error().message()
              << '\n';
    return 1;
  }

  auto state_text = state.value().as_string();
  if (!state_text)
  {
    std::cerr << "failed to convert runtime state: "
              << state_text.error().message()
              << '\n';
    return 1;
  }

  auto running_bool = running.value().as_boolean();
  if (!running_bool)
  {
    std::cerr << "failed to convert runtime running flag: "
              << running_bool.error().message()
              << '\n';
    return 1;
  }

  std::cout << "bridge name     = " << bridge.name() << '\n';
  std::cout << "bridge attached = "
            << (bridge.attached() ? "yes" : "no")
            << '\n';
  std::cout << "bridge owns rt  = "
            << (bridge.owns_runtime() ? "yes" : "no")
            << '\n';
  std::cout << "runtime state   = " << state_text.value() << '\n';
  std::cout << "runtime running = "
            << (running_bool.value() ? "yes" : "no")
            << '\n';

  auto eval_result = bridge.eval(
      "console.log('Hello from the runtime bridge');",
      "bridge.js");

  if (!eval_result.succeeded())
  {
    std::cerr << "bridge eval failed: "
              << eval_result.error.message()
              << '\n';
    return eval_result.exit_code;
  }

  auto value = eval_result.value.as_string();
  if (!value)
  {
    std::cerr << "failed to read bridge eval value: "
              << value.error().message()
              << '\n';
    return 1;
  }

  std::cout << "eval output     = " << eval_result.output << '\n';
  std::cout << "eval value      = " << value.value() << '\n';

  (void)context.shutdown();
  (void)bridge.shutdown();

  return 0;
}
