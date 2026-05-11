/**
 *
 *  @file QuickJsBackendDriver.cpp
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

#include <sstream>
#include <string>
#include <utility>

#include <kordex/bindings/backend/QuickJsBackendDriver.hpp>
#include <kordex/bindings/TypeScriptLoader.hpp>

#if defined(KORDEX_BINDINGS_ENABLE_QUICKJS) && KORDEX_BINDINGS_ENABLE_QUICKJS
#include <quickjs.h>
#endif

namespace kordex::bindings
{
#if defined(KORDEX_BINDINGS_ENABLE_QUICKJS) && KORDEX_BINDINGS_ENABLE_QUICKJS

  namespace
  {
    [[nodiscard]] JSRuntime *as_runtime(void *runtime) noexcept
    {
      return static_cast<JSRuntime *>(runtime);
    }

    [[nodiscard]] JSContext *as_context(void *context) noexcept
    {
      return static_cast<JSContext *>(context);
    }

    [[nodiscard]] std::string quickjs_cstring_to_string(
        JSContext *context,
        JSValue value)
    {
      const char *text = JS_ToCString(context, value);
      if (!text)
      {
        return {};
      }

      std::string result(text);
      JS_FreeCString(context, text);

      return result;
    }

    [[nodiscard]] std::string quickjs_property_string(
        JSContext *context,
        JSValue object,
        const char *property_name)
    {
      JSValue property = JS_GetPropertyStr(
          context,
          object,
          property_name);

      if (JS_IsException(property))
      {
        JSValue exception = JS_GetException(context);
        JS_FreeValue(context, exception);
        JS_FreeValue(context, property);
        return {};
      }

      if (JS_IsUndefined(property) || JS_IsNull(property))
      {
        JS_FreeValue(context, property);
        return {};
      }

      std::string result = quickjs_cstring_to_string(
          context,
          property);

      JS_FreeValue(context, property);

      return result;
    }

    [[nodiscard]] std::string quickjs_value_display(
        JSContext *context,
        JSValue value)
    {
      const char *text = JS_ToCString(context, value);
      if (!text)
      {
        JSValue exception = JS_GetException(context);
        JS_FreeValue(context, exception);
        return "JavaScript exception";
      }

      std::string result(text);
      JS_FreeCString(context, text);

      return result;
    }

    [[nodiscard]] std::string quickjs_exception_message(
        JSContext *context)
    {
      JSValue exception = JS_GetException(context);

      if (JS_IsUndefined(exception) || JS_IsNull(exception))
      {
        JS_FreeValue(context, exception);
        return "JavaScript exception";
      }

      const std::string name = quickjs_property_string(
          context,
          exception,
          "name");

      const std::string message = quickjs_property_string(
          context,
          exception,
          "message");

      const std::string stack = quickjs_property_string(
          context,
          exception,
          "stack");

      std::ostringstream stream;

      if (!name.empty() && !message.empty())
      {
        stream << name << ": " << message;
      }
      else if (!message.empty())
      {
        stream << message;
      }
      else
      {
        stream << quickjs_value_display(context, exception);
      }

      if (!stack.empty())
      {
        const std::string base = stream.str();

        if (stack.find(base) == std::string::npos)
        {
          stream << '\n'
                 << stack;
        }
        else
        {
          stream.str({});
          stream.clear();
          stream << stack;
        }
      }

      JS_FreeValue(context, exception);

      return stream.str().empty()
                 ? std::string("JavaScript exception")
                 : stream.str();
    }

    [[nodiscard]] Result<std::string> quickjs_value_to_string(
        JSContext *context,
        JSValue value)
    {
      const char *text = JS_ToCString(context, value);
      if (!text)
      {
        return make_binding_error(
            BindingErrorCode::ValueConversionFailed,
            quickjs_exception_message(context));
      }

      std::string result(text);
      JS_FreeCString(context, text);

      return result;
    }

    [[nodiscard]] Result<std::string> quickjs_object_to_json_or_string(
        JSContext *context,
        JSValue value)
    {
      JSValue json = JS_JSONStringify(
          context,
          value,
          JS_UNDEFINED,
          JS_UNDEFINED);

      if (!JS_IsException(json) && !JS_IsUndefined(json))
      {
        auto text = quickjs_value_to_string(context, json);
        JS_FreeValue(context, json);

        if (!text)
        {
          return text.error();
        }

        return text.value();
      }

      if (JS_IsException(json))
      {
        JS_FreeValue(context, json);

        JSValue exception = JS_GetException(context);
        JS_FreeValue(context, exception);
      }
      else
      {
        JS_FreeValue(context, json);
      }

      return quickjs_value_to_string(context, value);
    }

    [[nodiscard]] Result<Value> quickjs_value_to_binding_value(
        JSContext *context,
        JSValue value)
    {
      if (JS_IsUndefined(value))
      {
        return Value::undefined();
      }

      if (JS_IsNull(value))
      {
        return Value::null();
      }

      if (JS_IsBool(value))
      {
        return Value::boolean(JS_ToBool(context, value) != 0);
      }

      if (JS_IsNumber(value))
      {
        double number = 0.0;

        if (JS_ToFloat64(context, &number, value) < 0)
        {
          return make_binding_error(
              BindingErrorCode::ValueConversionFailed,
              quickjs_exception_message(context));
        }

        return Value::number(number);
      }

      if (JS_IsString(value))
      {
        auto text = quickjs_value_to_string(context, value);
        if (!text)
        {
          return text.error();
        }

        return Value::string(text.value());
      }

      if (JS_IsObject(value))
      {
        auto text = quickjs_object_to_json_or_string(context, value);
        if (!text)
        {
          return text.error();
        }

        return Value::string(text.value());
      }

      auto text = quickjs_value_to_string(context, value);
      if (!text)
      {
        return text.error();
      }

      return Value::string(text.value());
    }

    [[nodiscard]] ScriptResult make_quickjs_failure(
        JSContext *context)
    {
      return ScriptResult::failure(
          make_binding_error(
              BindingErrorCode::ScriptExecutionFailed,
              quickjs_exception_message(context)),
          1);
    }
  } // namespace

#endif

  QuickJsBackendDriver::QuickJsBackendDriver()
      : runtime_(nullptr),
        context_(nullptr)
  {
  }

  QuickJsBackendDriver::~QuickJsBackendDriver()
  {
#if defined(KORDEX_BINDINGS_ENABLE_QUICKJS) && KORDEX_BINDINGS_ENABLE_QUICKJS
    if (context_)
    {
      JS_FreeContext(as_context(context_));
      context_ = nullptr;
    }

    if (runtime_)
    {
      JS_FreeRuntime(as_runtime(runtime_));
      runtime_ = nullptr;
    }
#endif
  }

  BindingResult QuickJsBackendDriver::initialize(
      EngineContext &context)
  {
    (void)context;

#if defined(KORDEX_BINDINGS_ENABLE_QUICKJS) && KORDEX_BINDINGS_ENABLE_QUICKJS
    if (initialized())
    {
      return BindingResult::success("QuickJS backend already initialized");
    }

    JSRuntime *runtime = JS_NewRuntime();
    if (!runtime)
    {
      return BindingResult::failure(
          make_binding_error(
              BindingErrorCode::EngineInitializationFailed,
              "failed to create QuickJS runtime"),
          1);
    }

    JSContext *quickjs_context = JS_NewContext(runtime);
    if (!quickjs_context)
    {
      JS_FreeRuntime(runtime);

      return BindingResult::failure(
          make_binding_error(
              BindingErrorCode::EngineInitializationFailed,
              "failed to create QuickJS context"),
          1);
    }

    runtime_ = runtime;
    context_ = quickjs_context;

    return BindingResult::success("QuickJS backend initialized");
#else
    return BindingResult::failure(
        make_binding_error(
            BindingErrorCode::EngineUnavailable,
            "QuickJS backend is disabled in this build"),
        1);
#endif
  }

  BindingResult QuickJsBackendDriver::shutdown(
      EngineContext &context)
  {
    (void)context;

#if defined(KORDEX_BINDINGS_ENABLE_QUICKJS) && KORDEX_BINDINGS_ENABLE_QUICKJS
    if (context_)
    {
      JS_FreeContext(as_context(context_));
      context_ = nullptr;
    }

    if (runtime_)
    {
      JS_FreeRuntime(as_runtime(runtime_));
      runtime_ = nullptr;
    }

    return BindingResult::success("QuickJS backend stopped");
#else
    return BindingResult::success("QuickJS backend stopped");
#endif
  }

  ScriptResult QuickJsBackendDriver::run_script(
      EngineContext &engine_context,
      Script script)
  {
    if (!engine_context.initialized())
    {
      return ScriptResult::failure(
          make_binding_error(
              BindingErrorCode::ContextUnavailable,
              "engine context is not initialized"),
          1);
    }

    const auto validation = script.validate();
    if (validation)
    {
      return ScriptResult::failure(
          make_binding_error(
              BindingErrorCode::ScriptExecutionFailed,
              std::string(validation.message())),
          1);
    }

    if (!script.executable())
    {
      return ScriptResult::failure(
          make_binding_error(
              BindingErrorCode::ScriptExecutionFailed,
              "script type is not executable"),
          1);
    }

    if (script.type() == ScriptType::TypeScript)
    {
      TypeScriptLoader loader;

      auto loaded = loader.transpile(std::move(script));
      if (!loaded)
      {
        return ScriptResult::failure(
            loaded.error(),
            1);
      }

      return eval(
          engine_context,
          loaded.value().script.source(),
          loaded.value().script.name().empty()
              ? "main.js"
              : loaded.value().script.name());
    }

    return eval(
        engine_context,
        script.source(),
        script.name().empty() ? "main.js" : script.name());
  }

  ScriptResult QuickJsBackendDriver::eval(
      EngineContext &engine_context,
      std::string source,
      std::string name)
  {
    if (!engine_context.initialized())
    {
      return ScriptResult::failure(
          make_binding_error(
              BindingErrorCode::ContextUnavailable,
              "engine context is not initialized"),
          1);
    }

#if defined(KORDEX_BINDINGS_ENABLE_QUICKJS) && KORDEX_BINDINGS_ENABLE_QUICKJS
    if (!initialized())
    {
      return ScriptResult::failure(
          make_binding_error(
              BindingErrorCode::EngineUnavailable,
              "QuickJS backend is not initialized"),
          1);
    }

    JSContext *quickjs_context = as_context(context_);

    JSValue result = JS_Eval(
        quickjs_context,
        source.c_str(),
        source.size(),
        name.empty() ? "eval.js" : name.c_str(),
        JS_EVAL_TYPE_GLOBAL);

    if (JS_IsException(result))
    {
      JS_FreeValue(quickjs_context, result);
      return make_quickjs_failure(quickjs_context);
    }

    auto value = quickjs_value_to_binding_value(
        quickjs_context,
        result);

    JS_FreeValue(quickjs_context, result);

    if (!value)
    {
      return ScriptResult::failure(
          value.error(),
          1);
    }

    return ScriptResult::success(
        value.value());
#else
    (void)source;
    (void)name;

    return ScriptResult::failure(
        make_binding_error(
            BindingErrorCode::EngineUnavailable,
            "QuickJS backend is disabled in this build"),
        1);
#endif
  }

  bool QuickJsBackendDriver::initialized() const noexcept
  {
    return runtime_ != nullptr && context_ != nullptr;
  }

} // namespace kordex::bindings
