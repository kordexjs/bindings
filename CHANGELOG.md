# Changelog

All notable changes to `kordex::bindings` will be documented in this file.

The format follows a simple release-oriented structure.

## [0.1.0] - 2026-01-01

### Added

- Added initial `kordex::bindings` module.
- Added public umbrella header:

```cpp
#include <kordex/bindings/Bindings.hpp>
```

- Added version API:
  - `version()`
  - `version_major()`
  - `version_minor()`
  - `version_patch()`
- Added shared bindings error model:
  - `BindingErrorCode`
  - `make_binding_error()`
  - `ok()`
- Added bindings result types:
  - `Result<T>`
  - `BindingResult`
  - `ScriptResult`
- Added bindings configuration:
  - `BindingOptions`
  - `BindingConfig`
  - `EngineBackend`
  - `ModulePolicy`
- Added engine-independent value model:
  - `ValueType`
  - `Value`
  - support for `undefined`, `null`, `boolean`, `number`, and `string` values
- Added callable abstractions:
  - `Function`
  - `FunctionInfo`
  - `NativeFunction`
  - `NativeFunctionInfo`
- Added object representation:
  - `Object`
  - property storage
  - property lookup
  - property removal
  - key and value enumeration
- Added native module support:
  - `NativeModule`
  - `NativeModuleInfo`
  - native functions
  - constant values
  - conversion to object representation
- Added generic module support:
  - `Module`
  - `ModuleInfo`
  - `ModuleKind`
  - native module conversion
  - exported values
  - exported functions
- Added module registry:
  - `ModuleRegistry`
  - `ModuleRegistryOptions`
  - module registration
  - native module registration
  - module import
  - kind-based module filtering
- Added script representation:
  - `Script`
  - `ScriptInfo`
  - `ScriptType`
  - source-based scripts
  - runtime source file conversion
  - placeholder compile flow
  - placeholder execution flow
- Added engine context:
  - `EngineContext`
  - global values
  - global functions
  - module registry
  - script execution placeholder
  - source evaluation placeholder
- Added top-level engine facade:
  - `Engine`
  - `EngineInfo`
  - `EngineState`
  - lifecycle management
  - default context ownership
  - module registration
  - native module registration
  - global values and functions
  - script execution
- Added runtime bridge:
  - `RuntimeBridge`
  - `RuntimeBridgeInfo`
  - `RuntimeBridgeOptions`
  - bridge initialization and shutdown
  - runtime source loading
  - runtime script conversion
  - runtime module conversion
  - runtime result conversion
  - bridge installation into `EngineContext`
- Added public helper functions:
  - `package_name()`
  - `package_description()`
  - `native_engine_enabled()`
  - `quickjs_enabled()`
  - `v8_enabled()`
  - `default_backend()`
  - `create_engine()`
  - `create_context()`
  - `create_runtime_bridge()`
  - `run_source()`
  - `run_script()`
- Added examples:
  - `create_engine.cpp`
  - `run_script.cpp`
  - `expose_function.cpp`
  - `expose_module.cpp`
  - `bridge_runtime.cpp`
- Added tests for:
  - version API
  - error model
  - binding options
  - binding config
  - binding result
  - value model
  - functions
  - objects
  - native functions
  - native modules
  - modules
  - module registry
  - script result
  - scripts
  - engine context
  - engine
  - runtime bridge
- Added CMake package support:
  - `kordex::bindings`
  - standalone build
  - umbrella build support
  - install/export support
  - package config support

### Notes

This release uses a native placeholder backend.

JavaScript is not executed by a real engine yet.

Placeholder script execution validates the bindings architecture and returns the script source as a string value.

QuickJS and V8 integration are planned for future releases.

## Roadmap

### Planned

- Add QuickJS backend.
- Add real JavaScript compilation.
- Add real JavaScript execution.
- Add callable value support inside `Value`.
- Add native module exposure to the JavaScript backend.
- Add script-side module imports.
- Connect runtime module resolution to engine imports.
- Add builtin Kordex modules.
- Add better diagnostics and stack traces.
- Add TypeScript execution strategy.
