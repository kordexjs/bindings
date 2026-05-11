/**
 *
 *  @file NativeFunction.cpp
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
#include <kordex/bindings/NativeFunction.hpp>

namespace kordex::bindings
{
  bool NativeFunctionInfo::has_name() const noexcept
  {
    return !name.empty();
  }

  bool NativeFunctionInfo::has_module_name() const noexcept
  {
    return !module_name.empty();
  }

  bool NativeFunctionInfo::has_description() const noexcept
  {
    return !description.empty();
  }

  bool NativeFunctionInfo::has_max_args() const noexcept
  {
    return max_args > 0;
  }

  bool NativeFunctionInfo::accepts(std::size_t count) const noexcept
  {
    if (count < min_args)
    {
      return false;
    }

    if (has_max_args() && count > max_args)
    {
      return false;
    }

    return true;
  }

  NativeFunction::NativeFunction(
      NativeFunctionInfo info,
      NativeFunctionCallback callback)
      : info_(std::move(info)),
        callback_(std::move(callback))
  {
  }

  NativeFunction NativeFunction::create(
      std::string name,
      NativeFunctionCallback callback)
  {
    NativeFunctionInfo info;
    info.name = std::move(name);

    return NativeFunction(std::move(info), std::move(callback));
  }

  NativeFunction NativeFunction::create(
      NativeFunctionInfo info,
      NativeFunctionCallback callback)
  {
    return NativeFunction(std::move(info), std::move(callback));
  }

  bool NativeFunction::valid() const noexcept
  {
    return static_cast<bool>(callback_);
  }

  bool NativeFunction::callable() const noexcept
  {
    return valid() && info_.callable;
  }

  bool NativeFunction::safe() const noexcept
  {
    return info_.safe;
  }

  const NativeFunctionInfo &NativeFunction::info() const noexcept
  {
    return info_;
  }

  const std::string &NativeFunction::name() const noexcept
  {
    return info_.name;
  }

  const std::string &NativeFunction::module_name() const noexcept
  {
    return info_.module_name;
  }

  Result<Value> NativeFunction::call(
      const NativeFunctionArguments &args) const
  {
    const auto validation = validate_call(args);
    if (validation)
    {
      return validation;
    }

    return callback_(args);
  }

  Result<Value> NativeFunction::operator()() const
  {
    return call({});
  }

  Result<Value> NativeFunction::operator()(
      const NativeFunctionArguments &args) const
  {
    return call(args);
  }

  Function NativeFunction::to_function() const
  {
    FunctionInfo function_info;
    function_info.name = info_.name;
    function_info.min_args = info_.min_args;
    function_info.max_args = info_.max_args;
    function_info.callable = info_.callable;

    auto callback = callback_;

    return Function::create(
        std::move(function_info),
        [callback = std::move(callback)](
            const FunctionArguments &args) -> Result<Value>
        {
          if (!callback)
          {
            return make_binding_error(
                BindingErrorCode::FunctionCallFailed,
                "native function callback is not set");
          }

          return callback(args);
        });
  }

  Error NativeFunction::validate_call(
      const NativeFunctionArguments &args) const
  {
    if (!callback_)
    {
      return make_binding_error(
          BindingErrorCode::FunctionCallFailed,
          "native function callback is not set");
    }

    if (!info_.callable)
    {
      return make_binding_error(
          BindingErrorCode::PermissionDenied,
          "native function is not callable");
    }

    if (!info_.accepts(args.size()))
    {
      return make_binding_error(
          BindingErrorCode::InvalidArgument,
          "invalid native function argument count");
    }

    return ok();
  }

} // namespace kordex::bindings
