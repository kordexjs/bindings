/**
 *
 *  @file ModuleRegistry.cpp
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

#include <utility>
#include <kordex/bindings/ModuleRegistry.hpp>

namespace kordex::bindings
{
  ModuleRegistry::ModuleRegistry()
      : options_(),
        modules_()
  {
  }

  ModuleRegistry::ModuleRegistry(ModuleRegistryOptions options)
      : options_(std::move(options)),
        modules_()
  {
  }

  const ModuleRegistryOptions &ModuleRegistry::options() const noexcept
  {
    return options_;
  }

  void ModuleRegistry::set_options(ModuleRegistryOptions options)
  {
    options_ = std::move(options);
  }

  Error ModuleRegistry::register_module(Module module)
  {
    if (!module.valid())
    {
      return make_binding_error(
          BindingErrorCode::ModuleRegistrationFailed,
          "module is invalid");
    }

    return register_module(module.name(), std::move(module));
  }

  Error ModuleRegistry::register_module(
      std::string name,
      Module module)
  {
    const auto name_validation = validate_name(name);
    if (name_validation)
    {
      return name_validation;
    }

    if (!module.valid())
    {
      return make_binding_error(
          BindingErrorCode::ModuleRegistrationFailed,
          "module is invalid");
    }

    const auto allowed = validate_module_allowed(module);
    if (allowed)
    {
      return allowed;
    }

    if (!options_.allow_overwrite && has_module(name))
    {
      return make_binding_error(
          BindingErrorCode::ModuleRegistrationFailed,
          "module already exists: " + name);
    }

    modules_[std::move(name)] = std::move(module);
    return ok();
  }

  Error ModuleRegistry::register_native_module(
      const NativeModule &native_module)
  {
    if (!native_module.valid())
    {
      return make_binding_error(
          BindingErrorCode::ModuleRegistrationFailed,
          "native module is invalid");
    }

    return register_module(Module::from_native(native_module));
  }

  Result<Module> ModuleRegistry::module(
      const std::string &name) const
  {
    const auto validation = validate_name(name);
    if (validation)
    {
      return validation;
    }

    const auto found = modules_.find(name);
    if (found == modules_.end())
    {
      return make_binding_error(
          BindingErrorCode::ModuleNotFound,
          "module was not found: " + name);
    }

    return found->second;
  }

  bool ModuleRegistry::has_module(
      const std::string &name) const noexcept
  {
    return modules_.find(name) != modules_.end();
  }

  Error ModuleRegistry::remove_module(
      const std::string &name)
  {
    const auto validation = validate_name(name);
    if (validation)
    {
      return validation;
    }

    const auto erased = modules_.erase(name);
    if (erased == 0)
    {
      return make_binding_error(
          BindingErrorCode::ModuleNotFound,
          "module was not found: " + name);
    }

    return ok();
  }

  Result<Module> ModuleRegistry::import_module(
      const std::string &name) const
  {
    auto found_module = module(name);
    if (!found_module)
    {
      return found_module.error();
    }

    if (!found_module.value().importable())
    {
      return make_binding_error(
          BindingErrorCode::PermissionDenied,
          "module is not importable: " + name);
    }

    return found_module.value();
  }

  std::vector<std::string> ModuleRegistry::names() const
  {
    std::vector<std::string> result;
    result.reserve(modules_.size());

    for (const auto &[name, module] : modules_)
    {
      (void)module;
      result.push_back(name);
    }

    return result;
  }

  std::vector<std::string> ModuleRegistry::names_by_kind(
      ModuleKind kind) const
  {
    std::vector<std::string> result;

    for (const auto &[name, module] : modules_)
    {
      if (module.kind() == kind)
      {
        result.push_back(name);
      }
    }

    return result;
  }

  std::size_t ModuleRegistry::size() const noexcept
  {
    return modules_.size();
  }

  bool ModuleRegistry::empty() const noexcept
  {
    return modules_.empty();
  }

  void ModuleRegistry::clear() noexcept
  {
    modules_.clear();
  }

  const ModuleRegistry::ModuleMap &ModuleRegistry::modules() const noexcept
  {
    return modules_;
  }

  Error ModuleRegistry::validate_name(
      const std::string &name) const
  {
    if (name.empty())
    {
      return make_binding_error(
          BindingErrorCode::InvalidArgument,
          "module name cannot be empty");
    }

    return ok();
  }

  Error ModuleRegistry::validate_module_allowed(
      const Module &module) const
  {
    switch (module.kind())
    {
    case ModuleKind::Native:
      if (!options_.allow_native_modules)
      {
        return make_binding_error(
            BindingErrorCode::PermissionDenied,
            "native modules are disabled");
      }
      break;

    case ModuleKind::Script:
      if (!options_.allow_script_modules)
      {
        return make_binding_error(
            BindingErrorCode::PermissionDenied,
            "script modules are disabled");
      }
      break;

    case ModuleKind::Builtin:
      if (!options_.allow_builtin_modules)
      {
        return make_binding_error(
            BindingErrorCode::PermissionDenied,
            "builtin modules are disabled");
      }
      break;

    case ModuleKind::Unknown:
      return make_binding_error(
          BindingErrorCode::ModuleRegistrationFailed,
          "module kind is unknown");
    }

    return ok();
  }

} // namespace kordex::bindings
