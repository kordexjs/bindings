# Kordex Bindings

Native bindings layer for Kordex.

`kordex::bindings` connects the Kordex runtime to script execution. It provides the C++ abstraction layer for engines, contexts, values, functions, native modules, module registries, scripts, and runtime bridging.

The current implementation includes a native placeholder engine. It validates the architecture and public API before connecting a real JavaScript backend such as QuickJS or V8.

## Purpose

Kordex Bindings is responsible for:

- exposing C++ functions to scripts
- exposing native modules to scripts
- representing script values in C++
- managing engine contexts
- running scripts through a stable public API
- bridging `kordex::runtime` with `kordex::bindings`
- preparing the integration point for future JavaScript engines

The dependency direction is:

```txt
kordex::bindings
  -> kordex::runtime
  -> vix modules
```

`kordex::runtime` must never depend on `kordex::bindings`.

## Module role

The bindings module sits above runtime:

```
runtime
  loads files
  resolves modules
  manages process/task/timer/config/source files

bindings
  exposes runtime to script engines
  converts C++ values to script-facing values
  registers native modules
  executes scripts through Engine and EngineContext
```

## Features

- Engine abstraction
- Engine context
- Script representation
- Script result model
- Binding result model
- Generic value model
- Function wrapper
- Native function wrapper
- Object representation
- Native module support
- Module registry
- Runtime bridge
- Public umbrella header
- Examples
- Tests

## Current backend

The current backend is a native placeholder backend.

It does not execute JavaScript yet. Instead, it validates the full bindings architecture and returns predictable placeholder results.

Example:

```cpp
auto result = kordex::bindings::run_source(
    "console.log('Hello from Kordex bindings');",
    "hello.js");
```

The placeholder engine returns the source as a string value.

This makes the public API testable before QuickJS or V8 is added.

## Public headers

```
include/kordex/bindings/
├── Version.hpp
├── Error.hpp
├── Result.hpp
├── BindingOptions.hpp
├── BindingConfig.hpp
├── BindingResult.hpp
├── ValueType.hpp
├── Value.hpp
├── Function.hpp
├── Object.hpp
├── NativeFunction.hpp
├── NativeModule.hpp
├── Module.hpp
├── ModuleRegistry.hpp
├── ScriptResult.hpp
├── Script.hpp
├── EngineContext.hpp
├── Engine.hpp
├── RuntimeBridge.hpp
└── Bindings.hpp
```

## Main concepts

### Engine

`Engine` is the top-level bindings facade.

It owns:

- bindings configuration
- lifecycle state
- a default `EngineContext`

Example:

```cpp
auto engine_result = kordex::bindings::create_engine();
if (!engine_result)
{
  return 1;
}

auto engine = std::move(engine_result.value());

auto init_result = engine.initialize();
if (!init_result.succeeded())
{
  return init_result.exit_code;
}
```

### EngineContext

`EngineContext` owns:

- global values
- global functions
- module registry
- script evaluation state

Example:

```cpp
kordex::bindings::EngineContext context;

auto error = context.initialize();
if (error)
{
  return 1;
}

context.set_global(
    "answer",
    kordex::bindings::Value::number(42.0));
```

### Value

`Value` is the engine-independent value type used across the bindings boundary.

Supported primitive value types:

- `undefined`
- `null`
- `boolean`
- `number`
- `string`

Object, function, native function, and module are represented at the type level and will be expanded when the real engine backend is connected.

Example:

```cpp
auto value = kordex::bindings::Value::number(42.0);

auto number = value.as_number();
if (!number)
{
  return 1;
}
```

### Function

`Function` wraps a callable C++ callback.

Example:

```cpp
auto add = kordex::bindings::Function::create(
    "add",
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
```

### NativeFunction

`NativeFunction` is a C++ function intended to be exposed to scripts.

It stores:

- name
- module name
- description
- argument limits
- safety flag
- callback

### NativeModule

`NativeModule` groups native functions and constant values.

Example:

```cpp
auto module = kordex::bindings::NativeModule::create("math");

module.set_value(
    "version",
    kordex::bindings::Value::string("0.1.0"));

module.add_function(add_function);
```

### Module

`Module` is the generic module representation used by the bindings layer.

It can represent:

- native modules
- script modules
- builtin modules

### ModuleRegistry

`ModuleRegistry` stores modules visible to an engine context.

It supports:

- registering modules
- registering native modules
- importing modules
- filtering modules by kind
- permission policies

### Script

`Script` stores source code and metadata.

Supported script types:

- `javascript`
- `typescript`
- `json`

Only JavaScript and TypeScript are executable in the placeholder execution model.

Example:

```cpp
auto script = kordex::bindings::Script::from_source(
    "console.log('hello');",
    "hello.js");

auto result = script.run();
```

### RuntimeBridge

`RuntimeBridge` connects `kordex::runtime` to `kordex::bindings`.

It can:

- load source files through runtime
- convert runtime source files to scripts
- resolve runtime modules
- convert resolved modules to bindings modules
- install runtime state into an engine context
- run runtime-backed placeholder execution

Example:

```cpp
auto bridge_result = kordex::bindings::create_runtime_bridge();
if (!bridge_result)
{
  return 1;
}

auto bridge = std::move(bridge_result.value());

auto init_result = bridge.initialize();
if (!init_result.succeeded())
{
  return init_result.exit_code;
}
```

## Build

From the module directory:

```bash
vix build --build-target all -v
```

Or with CMake:

```bash
cmake -S . -B build-ninja -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DKORDEX_BINDINGS_BUILD_TESTS=ON \
  -DKORDEX_BINDINGS_BUILD_EXAMPLES=ON

cmake --build build-ninja
```

## Build options

```
KORDEX_BINDINGS_BUILD_TESTS=OFF
KORDEX_BINDINGS_BUILD_EXAMPLES=OFF
KORDEX_BINDINGS_ENABLE_WARNINGS=ON
KORDEX_BINDINGS_ENABLE_SANITIZERS=OFF

KORDEX_BINDINGS_ENABLE_NATIVE_ENGINE=ON
KORDEX_BINDINGS_ENABLE_QUICKJS=OFF
KORDEX_BINDINGS_ENABLE_V8=OFF

KORDEX_BINDINGS_FETCH_RUNTIME=ON
KORDEX_BINDINGS_FETCH_ERROR=ON
KORDEX_BINDINGS_FETCH_LOG=ON
KORDEX_BINDINGS_FETCH_JSON=ON
KORDEX_BINDINGS_FETCH_TESTS=ON

KORDEX_VIX_GIT_TAG=main
KORDEX_RUNTIME_GIT_TAG=main
```

Only one real JavaScript backend should be enabled at a time.

For now, the default backend is:

```
native
```

## Tests

Enable tests:

```bash
cmake -S . -B build-ninja -G Ninja \
  -DKORDEX_BINDINGS_BUILD_TESTS=ON

cmake --build build-ninja
ctest --test-dir build-ninja --output-on-failure
```

Test files:

```
tests/
├── test_version.cpp
├── test_error.cpp
├── test_binding_options.cpp
├── test_binding_config.cpp
├── test_binding_result.cpp
├── test_value.cpp
├── test_function.cpp
├── test_object.cpp
├── test_native_function.cpp
├── test_native_module.cpp
├── test_module.cpp
├── test_module_registry.cpp
├── test_script_result.cpp
├── test_script.cpp
├── test_engine_context.cpp
├── test_engine.cpp
└── test_runtime_bridge.cpp
```

## Examples

Build examples:

```bash
cmake -S . -B build-ninja -G Ninja \
  -DKORDEX_BINDINGS_BUILD_EXAMPLES=ON

cmake --build build-ninja
```

Examples:

```
examples/
├── create_engine.cpp
├── run_script.cpp
├── expose_function.cpp
├── expose_module.cpp
└── bridge_runtime.cpp
```

Run example binaries from the build directory.

## Public umbrella include

Use:

```cpp
#include <kordex/bindings/Bindings.hpp>
```

This includes the full public bindings API.

## Quick example

```cpp
#include <iostream>

#include <kordex/bindings/Bindings.hpp>

int main()
{
  auto result = kordex::bindings::run_source(
      "console.log('Hello from Kordex');",
      "hello.js");

  if (!result.succeeded())
  {
    std::cerr << result.error.message() << '\n';
    return result.exit_code;
  }

  auto value = result.value.as_string();
  if (!value)
  {
    std::cerr << value.error().message() << '\n';
    return 1;
  }

  std::cout << value.value() << '\n';

  return 0;
}
```

## Design rule

Bindings owns script-facing abstractions.

Runtime owns runtime behavior.

- `bindings` may depend on `runtime`
- `runtime` must not depend on `bindings`

This keeps Kordex modular and prevents circular dependencies.

## Roadmap

Next steps:

- connect QuickJS backend
- add real script compilation
- add real script execution
- represent callable values inside `Value`
- expose native modules to the JavaScript backend
- support module imports from scripts
- connect runtime module resolver to engine imports
- expose Kordex APIs as builtin modules
- add TypeScript transpilation strategy
- improve diagnostics and stack traces

## License

MIT License.
