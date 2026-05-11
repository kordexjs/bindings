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
#include <vector>

#include <kordex/bindings/backend/QuickJsBackendDriver.hpp>
#include <kordex/bindings/TypeScriptLoader.hpp>
#include <kordex/bindings/ModuleLoader.hpp>
#include <kordex/bindings/ModuleLoader.hpp>

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

    [[nodiscard]] JSValue binding_value_to_quickjs_value(
        JSContext *context,
        const Value &value)
    {
      if (value.is_undefined())
      {
        return JS_UNDEFINED;
      }

      if (value.is_null())
      {
        return JS_NULL;
      }

      if (value.is_boolean())
      {
        auto boolean = value.as_boolean();
        return JS_NewBool(context, boolean && boolean.value());
      }

      if (value.is_number())
      {
        auto number = value.as_number();
        return JS_NewFloat64(context, number ? number.value() : 0.0);
      }

      if (value.is_string())
      {
        auto text = value.as_string();
        return JS_NewString(
            context,
            text ? text.value().c_str() : "");
      }

      return JS_UNDEFINED;
    }

    [[nodiscard]] Result<Value> quickjs_argument_to_binding_value(
        JSContext *context,
        JSValueConst value)
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

      const char *text = JS_ToCString(context, value);
      if (!text)
      {
        return make_binding_error(
            BindingErrorCode::ValueConversionFailed,
            quickjs_exception_message(context));
      }

      std::string result(text);
      JS_FreeCString(context, text);

      return Value::string(std::move(result));
    }

    [[nodiscard]] JSValue throw_quickjs_type_error(
        JSContext *context,
        const std::string &message)
    {
      return JS_ThrowTypeError(
          context,
          "%s",
          message.c_str());
    }

    [[nodiscard]] JSValue kordex_call_native(
        JSContext *context,
        JSValueConst this_value,
        int argc,
        JSValueConst *argv)
    {
      (void)this_value;

      if (argc < 2)
      {
        return throw_quickjs_type_error(
            context,
            "__kordex_call_native requires module and function names");
      }

      EngineContext *engine_context =
          static_cast<EngineContext *>(JS_GetContextOpaque(context));

      if (!engine_context || !engine_context->initialized())
      {
        return throw_quickjs_type_error(
            context,
            "Kordex engine context is not available");
      }

      auto module_name_value = quickjs_argument_to_binding_value(
          context,
          argv[0]);

      if (!module_name_value || !module_name_value.value().is_string())
      {
        return throw_quickjs_type_error(
            context,
            "native module name must be a string");
      }

      auto function_name_value = quickjs_argument_to_binding_value(
          context,
          argv[1]);

      if (!function_name_value || !function_name_value.value().is_string())
      {
        return throw_quickjs_type_error(
            context,
            "native function name must be a string");
      }

      auto module_name = module_name_value.value().as_string();
      auto function_name = function_name_value.value().as_string();

      if (!module_name || !function_name)
      {
        return throw_quickjs_type_error(
            context,
            "invalid native module call");
      }

      auto module = engine_context->import_module(module_name.value());
      if (!module)
      {
        return JS_ThrowReferenceError(
            context,
            "%s",
            std::string(module.error().message()).c_str());
      }

      FunctionArguments args;
      args.reserve(argc > 2 ? static_cast<std::size_t>(argc - 2) : 0);

      for (int index = 2; index < argc; ++index)
      {
        auto argument = quickjs_argument_to_binding_value(
            context,
            argv[index]);

        if (!argument)
        {
          return JS_ThrowTypeError(
              context,
              "%s",
              std::string(argument.error().message()).c_str());
        }

        args.push_back(argument.value());
      }

      auto result = module.value().call(
          function_name.value(),
          args);

      if (!result)
      {
        return JS_ThrowInternalError(
            context,
            "%s",
            std::string(result.error().message()).c_str());
      }

      return binding_value_to_quickjs_value(
          context,
          result.value());
    }

    [[nodiscard]] Error install_native_bridge(
        JSContext *context,
        EngineContext &engine_context)
    {
      JS_SetContextOpaque(
          context,
          &engine_context);

      JSValue global = JS_GetGlobalObject(context);

      JSValue function = JS_NewCFunction(
          context,
          kordex_call_native,
          "__kordex_call_native",
          2);

      if (JS_IsException(function))
      {
        JS_FreeValue(context, global);

        return make_binding_error(
            BindingErrorCode::EngineInitializationFailed,
            quickjs_exception_message(context));
      }

      const int set_result = JS_SetPropertyStr(
          context,
          global,
          "__kordex_call_native",
          function);

      JS_FreeValue(context, global);

      if (set_result < 0)
      {
        return make_binding_error(
            BindingErrorCode::EngineInitializationFailed,
            quickjs_exception_message(context));
      }

      return ok();
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

    const auto bridge_error = install_native_bridge(
        quickjs_context,
        context);

    if (bridge_error)
    {
      JS_FreeContext(quickjs_context);
      JS_FreeRuntime(runtime);

      runtime_ = nullptr;
      context_ = nullptr;

      return BindingResult::failure(
          bridge_error,
          1);
    }

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

    if (!script.executable() && script.type() != ScriptType::Json)
    {
      return ScriptResult::failure(
          make_binding_error(
              BindingErrorCode::ScriptExecutionFailed,
              "script type is not executable"),
          1);
    }

    ModuleLoader loader(&engine_context);

    auto loaded = loader.load_entry(std::move(script));
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

    const auto bridge_error = install_native_bridge(
        quickjs_context,
        engine_context);

    if (bridge_error)
    {
      return ScriptResult::failure(
          bridge_error,
          1);
    }

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
