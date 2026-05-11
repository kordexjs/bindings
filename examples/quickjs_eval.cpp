#include <iostream>

#include <kordex/bindings/BindingOptions.hpp>
#include <kordex/bindings/Engine.hpp>

int main()
{
  using namespace kordex::bindings;

  BindingOptions options;
  options.backend = EngineBackend::QuickJS;
  options.engine_name = "quickjs";

  auto engine_result = Engine::create(options);
  if (!engine_result)
  {
    std::cerr << engine_result.error().message() << '\n';
    return 1;
  }

  auto engine = std::move(engine_result.value());

  auto init = engine.initialize();
  if (!init.succeeded())
  {
    std::cerr << init.error.message() << '\n';
    return init.exit_code;
  }

  auto result = engine.eval("40 + 2", "eval.js");
  if (!result.succeeded())
  {
    std::cerr << result.error.message() << '\n';
    return result.exit_code;
  }

  std::cout << result.value.display() << '\n';

  auto shutdown = engine.shutdown();
  if (!shutdown.succeeded())
  {
    std::cerr << shutdown.error.message() << '\n';
    return shutdown.exit_code;
  }

  return 0;
}
