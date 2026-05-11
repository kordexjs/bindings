/**
 *
 *  @file create_engine.cpp
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

  std::cout << "engine name    = " << engine.name() << '\n';
  std::cout << "engine backend = "
            << kordex::bindings::to_string(engine.backend())
            << '\n';
  std::cout << "engine state   = "
            << kordex::bindings::to_string(engine.state())
            << '\n';

  auto shutdown_result = engine.shutdown();
  if (!shutdown_result.succeeded())
  {
    std::cerr << "failed to shutdown engine: "
              << shutdown_result.error.message()
              << '\n';
    return shutdown_result.exit_code;
  }

  return 0;
}
