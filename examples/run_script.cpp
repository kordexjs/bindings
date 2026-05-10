/**
 *
 *  @file run_script.cpp
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
  auto result = kordex::bindings::run_source(
      "console.log('Hello from Kordex bindings');",
      "hello.js");

  if (!result.succeeded())
  {
    std::cerr << "script failed: "
              << result.error.message()
              << '\n';
    return result.exit_code;
  }

  std::cout << "status = "
            << kordex::bindings::to_string(result.status)
            << '\n';

  std::cout << "output = "
            << result.output
            << '\n';

  auto value = result.value.as_string();
  if (!value)
  {
    std::cerr << "failed to read script value: "
              << value.error().message()
              << '\n';
    return 1;
  }

  std::cout << "value  = "
            << value.value()
            << '\n';

  return 0;
}
