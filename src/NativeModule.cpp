/**
 *
 *  @file NativeModule.cpp
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

#include <utility>
#include <kordex/bindings/NativeModule.hpp>

namespace kordex::bindings
{
  bool NativeModuleInfo::has_name() const noexcept
  {
    return !name.empty();
  }

  bool NativeModuleInfo::has_namespace_name() const noexcept
  {
    return !namespace_name.empty();
  }

  bool NativeModuleInfo::has_description() const noexcept
  {
    return !description.empty();
  }

  NativeModule::NativeModule(NativeModuleInfo info)
      : info_(std::move(info)),
        functions_(),
        values_()
  {
  }

  NativeModule NativeModule::create(std::string name)
  {
    NativeModuleInfo info;
    info.name = std::move(name);

    return NativeModule(std::move(info));
  }

  NativeModule NativeModule::create(NativeModuleInfo info)
  {
    return NativeModule(std::move(info));
  }

  const NativeModuleInfo &NativeModule::info() const noexcept
  {
    return info_;
  }

  const std::string &NativeModule::name() const noexcept
  {
    return info_.name;
  }

  const std::string &NativeModule::namespace_name() const noexcept
  {
    return info_.namespace_name;
  }

  bool NativeModule::valid() const noexcept
  {
    return info_.has_name();
  }

  bool NativeModule::importable() const noexcept
  {
    return valid() && info_.importable;
  }

  bool NativeModule::safe() const noexcept
  {
    return info_.safe;
  }

  Error NativeModule::add_function(NativeFunction function)
  {
    if (!function.valid())
    {
      return make_binding_error(
          BindingErrorCode::ModuleRegistrationFailed,
          "native module function is invalid");
    }

    if (function.name().empty())
    {
      return make_binding_error(
          BindingErrorCode::InvalidArgument,
          "native module function name cannot be empty");
    }

    return add_function(function.name(), std::move(function));
  }

  Error NativeModule::add_function(
      std::string name,
      NativeFunction function)
  {
    const auto validation = validate_name(name, "function");
    if (validation)
    {
      return validation;
    }

    if (!function.valid())
    {
      return make_binding_error(
          BindingErrorCode::ModuleRegistrationFailed,
          "native module function is invalid");
    }

    functions_[std::move(name)] = std::move(function);
    return ok();
  }

  Result<NativeFunction> NativeModule::function(
      const std::string &name) const
  {
    const auto validation = validate_name(name, "function");
    if (validation)
    {
      return validation;
    }

    const auto found = functions_.find(name);
    if (found == functions_.end())
    {
      return make_binding_error(
          BindingErrorCode::ModuleNotFound,
          "native module function was not found: " + name);
    }

    return found->second;
  }

  bool NativeModule::has_function(
      const std::string &name) const noexcept
  {
    return functions_.find(name) != functions_.end();
  }

  Error NativeModule::remove_function(
      const std::string &name)
  {
    const auto validation = validate_name(name, "function");
    if (validation)
    {
      return validation;
    }

    const auto erased = functions_.erase(name);
    if (erased == 0)
    {
      return make_binding_error(
          BindingErrorCode::ModuleNotFound,
          "native module function was not found: " + name);
    }

    return ok();
  }

  Error NativeModule::set_value(
      std::string name,
      Value value)
  {
    const auto validation = validate_name(name, "value");
    if (validation)
    {
      return validation;
    }

    values_[std::move(name)] = std::move(value);
    return ok();
  }

  Result<Value> NativeModule::value(
      const std::string &name) const
  {
    const auto validation = validate_name(name, "value");
    if (validation)
    {
      return validation;
    }

    const auto found = values_.find(name);
    if (found == values_.end())
    {
      return make_binding_error(
          BindingErrorCode::ModuleNotFound,
          "native module value was not found: " + name);
    }

    return found->second;
  }

  bool NativeModule::has_value(
      const std::string &name) const noexcept
  {
    return values_.find(name) != values_.end();
  }

  Error NativeModule::remove_value(
      const std::string &name)
  {
    const auto validation = validate_name(name, "value");
    if (validation)
    {
      return validation;
    }

    const auto erased = values_.erase(name);
    if (erased == 0)
    {
      return make_binding_error(
          BindingErrorCode::ModuleNotFound,
          "native module value was not found: " + name);
    }

    return ok();
  }

  Result<Value> NativeModule::call(
      const std::string &name,
      const NativeFunctionArguments &args) const
  {
    auto native_function = function(name);
    if (!native_function)
    {
      return native_function.error();
    }

    return native_function.value().call(args);
  }

  std::vector<std::string> NativeModule::function_names() const
  {
    std::vector<std::string> result;
    result.reserve(functions_.size());

    for (const auto &[name, function] : functions_)
    {
      (void)function;
      result.push_back(name);
    }

    return result;
  }

  std::vector<std::string> NativeModule::value_names() const
  {
    std::vector<std::string> result;
    result.reserve(values_.size());

    for (const auto &[name, value] : values_)
    {
      (void)value;
      result.push_back(name);
    }

    return result;
  }

  std::size_t NativeModule::function_count() const noexcept
  {
    return functions_.size();
  }

  std::size_t NativeModule::value_count() const noexcept
  {
    return values_.size();
  }

  bool NativeModule::empty() const noexcept
  {
    return functions_.empty() && values_.empty();
  }

  void NativeModule::clear() noexcept
  {
    functions_.clear();
    values_.clear();
  }

  const NativeModule::FunctionMap &NativeModule::functions() const noexcept
  {
    return functions_;
  }

  const NativeModule::ValueMap &NativeModule::values() const noexcept
  {
    return values_;
  }

  Object NativeModule::to_object() const
  {
    Object object = Object::create(info_.name);

    for (const auto &[name, value] : values_)
    {
      (void)object.set(name, value);
    }

    for (const auto &[name, function] : functions_)
    {
      (void)function;
      (void)object.set(
          name,
          Value::string("[native function]"));
    }

    return object;
  }

  Error NativeModule::validate_name(
      const std::string &name,
      const char *kind) const
  {
    if (name.empty())
    {
      return make_binding_error(
          BindingErrorCode::InvalidArgument,
          std::string("native module ") +
              (kind ? kind : "entry") +
              " name cannot be empty");
    }

    return ok();
  }

} // namespace kordex::bindings
