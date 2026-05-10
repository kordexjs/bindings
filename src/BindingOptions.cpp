/**
 *
 *  @file BindingOptions.cpp
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

#include <kordex/bindings/BindingOptions.hpp>

namespace kordex::bindings
{
  BindingOptions BindingOptions::defaults()
  {
    return BindingOptions{};
  }

  BindingOptions BindingOptions::development()
  {
    BindingOptions options;
    options.backend = EngineBackend::Native;
    options.module_policy = ModulePolicy::Full;
    options.allow_native_modules = true;
    options.allow_native_functions = true;
    options.allow_runtime_bridge = true;
    options.diagnostics = true;
    options.debug = true;
    options.source_maps = true;
    options.max_stack_size = 0;
    options.max_heap_size = 0;
    options.engine_name = "native-dev";
    return options;
  }

  BindingOptions BindingOptions::production()
  {
    BindingOptions options;
    options.backend = EngineBackend::Native;
    options.module_policy = ModulePolicy::Full;
    options.allow_native_modules = true;
    options.allow_native_functions = true;
    options.allow_runtime_bridge = true;
    options.diagnostics = true;
    options.debug = false;
    options.source_maps = false;
    options.max_stack_size = 0;
    options.max_heap_size = 0;
    options.engine_name = "native";
    return options;
  }

  BindingOptions BindingOptions::test()
  {
    BindingOptions options;
    options.backend = EngineBackend::Native;
    options.module_policy = ModulePolicy::Full;
    options.allow_native_modules = true;
    options.allow_native_functions = true;
    options.allow_runtime_bridge = true;
    options.diagnostics = true;
    options.debug = true;
    options.source_maps = false;
    options.max_stack_size = 1024 * 1024;
    options.max_heap_size = 16 * 1024 * 1024;
    options.engine_name = "native-test";
    return options;
  }

  bool BindingOptions::has_stack_limit() const noexcept
  {
    return max_stack_size > 0;
  }

  bool BindingOptions::has_heap_limit() const noexcept
  {
    return max_heap_size > 0;
  }

  bool BindingOptions::has_module_paths() const noexcept
  {
    return !module_paths.empty();
  }

  bool BindingOptions::has_engine_name() const noexcept
  {
    return !engine_name.empty();
  }

  const char *to_string(EngineBackend backend) noexcept
  {
    switch (backend)
    {
    case EngineBackend::Native:
      return "native";
    case EngineBackend::QuickJS:
      return "quickjs";
    case EngineBackend::V8:
      return "v8";
    }

    return "native";
  }

  const char *to_string(ModulePolicy policy) noexcept
  {
    switch (policy)
    {
    case ModulePolicy::Disabled:
      return "disabled";
    case ModulePolicy::BuiltinOnly:
      return "builtin_only";
    case ModulePolicy::Full:
      return "full";
    }

    return "full";
  }

} // namespace kordex::bindings
