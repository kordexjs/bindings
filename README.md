# Kordex Bindings

Native bindings layer for Kordex.

Kordex Bindings connects the Kordex runtime to a real JavaScript engine.
It provides the engine abstraction, script execution API, native module bridge, value conversion layer, module loading, TypeScript loading, and the bridge used to expose C++ functions to JavaScript.

## Role

`kordex-bindings` is the execution bridge between:

- `kordex-runtime`
- JavaScript engine backends
- `kordex-std`
- native C++ modules
- user scripts

It is responsible for turning a Kordex script into something executable by the selected JavaScript backend.

## Features

- Engine abstraction
- QuickJS backend support
- Native placeholder backend
- Script loading and execution
- JavaScript `eval()`
- TypeScript loading and basic transpilation
- Static import loading
- Relative import resolution
- JSON module loading
- Built-in module imports such as `kordex:path`
- Native module registry
- Native function bridge
- C++ value conversion to JavaScript values
- JavaScript arguments conversion to C++ values
- Module cache
- Runtime bridge foundation
- Source-map-ready script pipeline

## Public headers

```txt
include/kordex/bindings/
├── BindingConfig.hpp
├── BindingOptions.hpp
├── BindingResult.hpp
├── Bindings.hpp
├── Engine.hpp
├── EngineContext.hpp
├── Error.hpp
├── Function.hpp
├── Module.hpp
├── ModuleLoader.hpp
├── ModuleRegistry.hpp
├── NativeFunction.hpp
├── NativeModule.hpp
├── Object.hpp
├── Result.hpp
├── RuntimeBridge.hpp
├── Script.hpp
├── ScriptResult.hpp
├── TypeScriptLoader.hpp
├── Value.hpp
├── ValueType.hpp
├── Version.hpp
└── backend/
    ├── BackendDriver.hpp
    └── QuickJsBackendDriver.hpp
```

## Engine backends

Kordex Bindings supports multiple backend modes.

```cpp
enum class EngineBackend
{
  Native,
  QuickJS,
  V8
};
```

Current practical backend:

- QuickJS

The native backend remains useful as a minimal placeholder or test backend.

## Build options

The backend is selected through CMake options:

- `KORDEX_BINDINGS_ENABLE_NATIVE_ENGINE`
- `KORDEX_BINDINGS_ENABLE_QUICKJS`
- `KORDEX_BINDINGS_ENABLE_V8`

Example with QuickJS:

```bash
cmake -S . -B build-quickjs -G Ninja \
  -DKORDEX_BINDINGS_ENABLE_QUICKJS=ON \
  -DKORDEX_BINDINGS_ENABLE_NATIVE_ENGINE=OFF

cmake --build build-quickjs
```

Only one real JavaScript backend should be enabled at a time.

## Basic usage

Create an engine:

```cpp
#include <kordex/bindings/Bindings.hpp>

int main()
{
  auto engine_result = kordex::bindings::create_engine();

  if (!engine_result)
  {
    return 1;
  }

  auto engine = std::move(engine_result.value());

  auto init = engine.initialize();
  if (!init.succeeded())
  {
    return init.exit_code;
  }

  auto result = engine.eval("1 + 2", "main.js");

  engine.shutdown();

  return result.exit_code;
}
```

## Evaluate JavaScript

```cpp
auto result = engine.eval(
    "const x = 40 + 2; x;",
    "main.js");

if (result.succeeded())
{
  std::cout << result.value.display() << "\n";
}
```

Expected value:
```
42
```
## Script loading

Scripts can be loaded from source or from disk.

```cpp
auto script = kordex::bindings::Script::load("main.js");

if (!script)
{
  return 1;
}

auto result = engine.run_script(script.value());
```

Supported script types:

- `.js`
- `.mjs`
- `.cjs`
- `.ts`
- `.mts`
- `.cts`
- `.json`

## TypeScript support

Kordex Bindings includes a minimal TypeScript loader.

It supports:

- `.ts`
- `.mts`
- `.cts`
- simple type stripping
- basic syntax checks
- transformation to JavaScript before execution

Example:

```typescript
const name: string = "Kordex";

function hello(value: string): string {
  return "Hello " + value;
}

hello(name);
```

The TypeScript loader transforms this into JavaScript before sending it to QuickJS.

## Module loading

Kordex Bindings includes a static module loader.

It supports:

- relative imports
- extension resolution
- directory index.js
- JSON imports
- TypeScript imports
- built-in module imports
- module cache

Example:

```javascript
import { message } from "./lib/message.js";

message;
```

Extension-less imports are supported:

```javascript
import { message } from "./lib/message";
```

Directory imports are supported:

```javascript
import { name } from "./pkg";
```

This resolves:
```
./pkg/index.js
```

## JSON modules

JSON imports are supported:

```javascript
import user from "./user.json";

user.name;
```

The JSON module is converted internally to:

```javascript
exports.default = JSON.parse(...);
```

## Built-in modules

Built-in Kordex modules use the `kordex:` prefix.

Example:

```javascript
import { join } from "kordex:path";

join("/tmp", "kordex", "app");
```

Built-in modules are resolved from the engine context module registry.

The standard modules are installed by `kordex-std` or by the CLI before script execution.

## Native modules

A native module is a C++ module exposed to JavaScript.

```cpp
kordex::bindings::NativeModule module("example");

module.set_function(
    kordex::bindings::NativeFunction::create(
        "hello",
        [](const kordex::bindings::NativeFunctionArguments &args)
            -> kordex::bindings::Result<kordex::bindings::Value>
        {
          (void)args;
          return kordex::bindings::Value::string("Hello from C++");
        }));
```

Then JavaScript can import and call it when installed:

```javascript
import { hello } from "kordex:example";

hello();
```

## Native function bridge

Kordex Bindings exposes native C++ functions to JavaScript through an internal bridge.

The generated JavaScript module calls:

```javascript
__kordex_call_native(moduleName, functionName, ...args)
```

The QuickJS backend then:

1. Converts JavaScript arguments to `kordex::bindings::Value`
2. Looks up the native module from `EngineContext`
3. Calls the native function
4. Converts the C++ result back to a QuickJS value

Supported value types:

- `undefined`
- `null`
- `boolean`
- `number`
- `string`

Object and function support can be expanded later.

## Value system

`Value` is the engine-independent value transported across the bindings layer.

```cpp
Value::undefined();
Value::null();
Value::boolean(true);
Value::number(42.0);
Value::string("Kordex");
```

Example:

```cpp
auto value = kordex::bindings::Value::string("hello");

if (value.is_string())
{
  auto text = value.as_string();
}
```

## Module registry

`ModuleRegistry` stores modules visible to the bindings layer.

It supports:

- registering modules
- registering native modules
- importing modules
- removing modules
- listing modules by name
- listing modules by kind

Example:

```cpp
kordex::bindings::ModuleRegistry registry;

auto error = registry.register_native_module(native_module);

if (error)
{
  return 1;
}

auto module = registry.import_module("fs");
```

## Engine context

`EngineContext` owns:

- binding configuration
- initialization state
- module registry
- runtime bridge state

It is passed to backend drivers during initialization and execution.

```cpp
kordex::bindings::EngineContext context(config);

auto error = context.initialize();
```

## Binding options

Use `BindingOptions` to configure an engine before creation.

```cpp
kordex::bindings::BindingOptions options;

options.backend = kordex::bindings::EngineBackend::QuickJS;
options.module_policy = kordex::bindings::ModulePolicy::Full;
options.allow_native_modules = true;
options.allow_native_functions = true;
options.allow_runtime_bridge = true;
options.diagnostics = true;
options.debug = true;
options.source_maps = true;
```

Then:

```cpp
auto engine = kordex::bindings::Engine::create(options);
```

## Binding config

`BindingConfig` is the normalized configuration used internally.

It is created from options:

```cpp
auto config = kordex::bindings::BindingConfig::from_options(options);
```

It validates:

- engine name
- environment
- module policy
- native module permissions
- native function permissions

## Error handling

Kordex Bindings uses `Result<T>` and structured errors instead of exceptions for recoverable failures.

```cpp
auto result = engine.eval("throw new Error('boom')", "error.js");

if (result.failed())
{
  std::cerr << result.error.message() << "\n";
}
```

JavaScript errors are converted into `ScriptResult::failure(...)`.

Expected message:
```
Error: boom
```

## ScriptResult

`ScriptResult` stores:

- execution status
- exit code
- structured error
- returned value
- stdout text
- stderr text

```cpp
if (result.succeeded())
{
  std::cout << result.value.display() << "\n";
}

if (result.failed())
{
  std::cerr << result.error.message() << "\n";
}
```

## BindingResult

`BindingResult` is used for engine-level operations:

- initialization
- shutdown
- context creation
- backend setup

```cpp
auto init = engine.initialize();

if (!init.succeeded())
{
  return init.exit_code;
}
```

## ModuleLoader pipeline

The module loader pipeline is:

1. Script
2. -> TypeScriptLoader if `.ts`/`.mts`/`.cts`
3. -> ModuleResolver for relative imports
4. -> JSON module conversion
5. -> builtin module conversion
6. -> CommonJS-like bundle generation
7. -> QuickJS eval

## TypeScriptLoader pipeline

The TypeScript loader pipeline is:

1. TypeScript source
2. -> basic check
3. -> strip type-only syntax
4. -> emit JavaScript Script

This is intentionally an MVP. A full TypeScript compiler can replace this layer later without changing the public API.

## QuickJS backend

The QuickJS backend is responsible for:

- creating `JSRuntime`
- creating `JSContext`
- evaluating JavaScript
- collecting returned values
- formatting JavaScript exceptions
- exposing the native bridge
- converting values between QuickJS and Kordex

## Build from source

From the module directory:

```bash
vix build \
  --preset dev-ninja
```

With QuickJS enabled:

```bash
vix build \
  --preset dev-ninja \
  -- \
  -DKORDEX_BINDINGS_ENABLE_QUICKJS=ON \
  -DKORDEX_BINDINGS_ENABLE_NATIVE_ENGINE=OFF
```

With tests:

```bash
vix build \
  --preset dev-ninja \
  -- \
  -DKORDEX_BINDINGS_BUILD_TESTS=ON

vix tests -- --output-on-failure
```

## Examples

Evaluate JavaScript:

```bash
./build-ninja/examples/eval_script
```

Run a script:

```bash
./build-ninja/examples/run_script
```

Use native modules:

```bash
./build-ninja/examples/native_module
```

## Tests

The bindings tests should cover:

- engine creation
- engine initialization
- JavaScript evaluation
- JavaScript errors
- TypeScript loading
- module imports
- JSON modules
- native modules
- native functions
- value conversion
- module registry
- backend shutdown

Run:

```bash
ctest --test-dir build-ninja --output-on-failure
```

## Current limitations

- TypeScript support is MVP-level and does not replace the official TypeScript compiler
- Source maps are prepared at the build layer, but detailed line/column mappings are still future work
- Object and function value conversion is not fully exposed yet
- Package imports are not connected yet
- Native ES module execution is not used directly yet
- The current module loader emits a CommonJS-like bundle for QuickJS evaluation

## Module role in Kordex

`kordex-bindings` is not the CLI and not the standard library.

It is the bridge that makes this possible:

```bash
kordex run main.ts
```

Internally:

1. CLI
2. -> Runtime options
3. -> Bindings engine
4. -> Module loader
5. -> TypeScript loader
6. -> QuickJS backend
7. -> Native std modules
8. -> ScriptResult

## License

MIT License.
