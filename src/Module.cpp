/**
 *
 *  @file Module.cpp
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
#include <kordex/bindings/Module.hpp>

namespace kordex::bindings
{
  bool ModuleInfo::has_name() const noexcept
  {
    return !name.empty();
  }

  bool ModuleInfo::has_path() const noexcept
  {
    return !path.empty();
  }

  bool ModuleInfo::has_description() const noexcept
  {
    return !description.empty();
  }

  Module::Module(ModuleInfo info)
      : info_(std::move(info)),
        exports_(),
        functions_()
  {
  }

  Module Module::create(
      std::string name,
      ModuleKind kind)
  {
    ModuleInfo info;
    info.name = std::move(name);
    info.kind = kind;

    return Module(std::move(info));
  }

  Module Module::create(ModuleInfo info)
  {
    return Module(std::move(info));
  }

  Module Module::from_native(
      const NativeModule &native_module)
  {
    ModuleInfo info;
    info.name = native_module.name();
    info.kind = ModuleKind::Native;
    info.importable = native_module.importable();
    info.loaded = true;

    if (native_module.info().has_description())
    {
      info.description = native_module.info().description;
    }

    Module module(std::move(info));

    for (const auto &[name, value] : native_module.values())
    {
      (void)module.set_export(name, value);
    }

    for (const auto &[name, native_function] : native_module.functions())
    {
      (void)module.set_function(name, native_function.to_function());
    }

    return module;
  }

  const ModuleInfo &Module::info() const noexcept
  {
    return info_;
  }

  const std::string &Module::name() const noexcept
  {
    return info_.name;
  }

  const std::string &Module::path() const noexcept
  {
    return info_.path;
  }

  ModuleKind Module::kind() const noexcept
  {
    return info_.kind;
  }

  bool Module::valid() const noexcept
  {
    return info_.has_name() && info_.kind != ModuleKind::Unknown;
  }

  bool Module::importable() const noexcept
  {
    return valid() && info_.importable;
  }

  bool Module::loaded() const noexcept
  {
    return info_.loaded;
  }

  void Module::mark_loaded() noexcept
  {
    info_.loaded = true;
  }

  void Module::mark_unloaded() noexcept
  {
    info_.loaded = false;
  }

  Error Module::set_export(
      std::string name,
      Value value)
  {
    const auto validation = validate_name(name, "export");
    if (validation)
    {
      return validation;
    }

    exports_[std::move(name)] = std::move(value);
    return ok();
  }

  Result<Value> Module::export_value(
      const std::string &name) const
  {
    const auto validation = validate_name(name, "export");
    if (validation)
    {
      return validation;
    }

    const auto found = exports_.find(name);
    if (found == exports_.end())
    {
      return make_binding_error(
          BindingErrorCode::ModuleNotFound,
          "module export was not found: " + name);
    }

    return found->second;
  }

  bool Module::has_export(
      const std::string &name) const noexcept
  {
    return exports_.find(name) != exports_.end();
  }

  Error Module::remove_export(
      const std::string &name)
  {
    const auto validation = validate_name(name, "export");
    if (validation)
    {
      return validation;
    }

    const auto erased = exports_.erase(name);
    if (erased == 0)
    {
      return make_binding_error(
          BindingErrorCode::ModuleNotFound,
          "module export was not found: " + name);
    }

    return ok();
  }

  Error Module::set_function(
      std::string name,
      Function function)
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
          "module function is invalid");
    }

    functions_[std::move(name)] = std::move(function);
    return ok();
  }

  Error Module::add_function(Function function)
  {
    if (!function.valid())
    {
      return make_binding_error(
          BindingErrorCode::ModuleRegistrationFailed,
          "module function is invalid");
    }

    if (function.name().empty())
    {
      return make_binding_error(
          BindingErrorCode::InvalidArgument,
          "module function name cannot be empty");
    }

    return set_function(function.name(), std::move(function));
  }

  Result<Function> Module::function(
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
          "module function was not found: " + name);
    }

    return found->second;
  }

  bool Module::has_function(
      const std::string &name) const noexcept
  {
    return functions_.find(name) != functions_.end();
  }

  Error Module::remove_function(
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
          "module function was not found: " + name);
    }

    return ok();
  }

  Result<Value> Module::call(
      const std::string &name,
      const FunctionArguments &args) const
  {
    auto found_function = function(name);
    if (!found_function)
    {
      return found_function.error();
    }

    return found_function.value().call(args);
  }

  std::vector<std::string> Module::export_names() const
  {
    std::vector<std::string> result;
    result.reserve(exports_.size());

    for (const auto &[name, value] : exports_)
    {
      (void)value;
      result.push_back(name);
    }

    return result;
  }

  std::vector<std::string> Module::function_names() const
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

  std::size_t Module::export_count() const noexcept
  {
    return exports_.size();
  }

  std::size_t Module::function_count() const noexcept
  {
    return functions_.size();
  }

  bool Module::empty() const noexcept
  {
    return exports_.empty() && functions_.empty();
  }

  void Module::clear() noexcept
  {
    exports_.clear();
    functions_.clear();
  }

  const Module::ValueMap &Module::exports() const noexcept
  {
    return exports_;
  }

  const Module::FunctionMap &Module::functions() const noexcept
  {
    return functions_;
  }

  Object Module::to_object() const
  {
    Object object = Object::create(info_.name);

    for (const auto &[name, value] : exports_)
    {
      (void)object.set(name, value);
    }

    for (const auto &[name, function] : functions_)
    {
      (void)function;
      (void)object.set(
          name,
          Value::string("[function]"));
    }

    return object;
  }

  Error Module::validate_name(
      const std::string &name,
      const char *kind) const
  {
    if (name.empty())
    {
      return make_binding_error(
          BindingErrorCode::InvalidArgument,
          std::string("module ") +
              (kind ? kind : "entry") +
              " name cannot be empty");
    }

    return ok();
  }

  const char *to_string(ModuleKind kind) noexcept
  {
    switch (kind)
    {
    case ModuleKind::Unknown:
      return "unknown";
    case ModuleKind::Native:
      return "native";
    case ModuleKind::Script:
      return "script";
    case ModuleKind::Builtin:
      return "builtin";
    }

    return "unknown";
  }

} // namespace kordex::bindings
