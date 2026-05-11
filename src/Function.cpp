/**
 *
 *  @file Function.cpp
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
#include <kordex/bindings/Function.hpp>

namespace kordex::bindings
{
  bool FunctionInfo::has_name() const noexcept
  {
    return !name.empty();
  }

  bool FunctionInfo::has_max_args() const noexcept
  {
    return max_args > 0;
  }

  bool FunctionInfo::accepts(std::size_t count) const noexcept
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

  Function::Function(
      FunctionInfo info,
      FunctionCallback callback)
      : info_(std::move(info)),
        callback_(std::move(callback))
  {
  }

  Function Function::create(
      std::string name,
      FunctionCallback callback)
  {
    FunctionInfo info;
    info.name = std::move(name);

    return Function(std::move(info), std::move(callback));
  }

  Function Function::create(
      FunctionInfo info,
      FunctionCallback callback)
  {
    return Function(std::move(info), std::move(callback));
  }

  bool Function::valid() const noexcept
  {
    return static_cast<bool>(callback_);
  }

  bool Function::callable() const noexcept
  {
    return valid() && info_.callable;
  }

  const FunctionInfo &Function::info() const noexcept
  {
    return info_;
  }

  const std::string &Function::name() const noexcept
  {
    return info_.name;
  }

  Result<Value> Function::call(
      const FunctionArguments &args) const
  {
    const auto validation = validate_call(args);
    if (validation)
    {
      return validation;
    }

    return callback_(args);
  }

  Result<Value> Function::operator()() const
  {
    return call({});
  }

  Result<Value> Function::operator()(
      const FunctionArguments &args) const
  {
    return call(args);
  }

  Error Function::validate_call(
      const FunctionArguments &args) const
  {
    if (!callback_)
    {
      return make_binding_error(
          BindingErrorCode::FunctionCallFailed,
          "function callback is not set");
    }

    if (!info_.callable)
    {
      return make_binding_error(
          BindingErrorCode::PermissionDenied,
          "function is not callable");
    }

    if (!info_.accepts(args.size()))
    {
      return make_binding_error(
          BindingErrorCode::InvalidArgument,
          "invalid function argument count");
    }

    return ok();
  }

} // namespace kordex::bindings
