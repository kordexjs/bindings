/**
 *
 *  @file Object.cpp
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
#include <kordex/bindings/Object.hpp>

namespace kordex::bindings
{
  Object::Object(std::string name)
      : name_(std::move(name)),
        properties_()
  {
  }

  Object Object::create()
  {
    return Object{};
  }

  Object Object::create(std::string name)
  {
    return Object(std::move(name));
  }

  const std::string &Object::name() const noexcept
  {
    return name_;
  }

  void Object::set_name(std::string name)
  {
    name_ = std::move(name);
  }

  bool Object::has_name() const noexcept
  {
    return !name_.empty();
  }

  Error Object::set(
      std::string key,
      Value value)
  {
    const auto validation = validate_key(key);
    if (validation)
    {
      return validation;
    }

    properties_[std::move(key)] = std::move(value);
    return ok();
  }

  Result<Value> Object::get(
      const std::string &key) const
  {
    const auto validation = validate_key(key);
    if (validation)
    {
      return validation;
    }

    const auto found = properties_.find(key);
    if (found == properties_.end())
    {
      return make_binding_error(
          BindingErrorCode::InvalidArgument,
          "object property was not found: " + key);
    }

    return found->second;
  }

  bool Object::has(
      const std::string &key) const noexcept
  {
    return properties_.find(key) != properties_.end();
  }

  Error Object::remove(
      const std::string &key)
  {
    const auto validation = validate_key(key);
    if (validation)
    {
      return validation;
    }

    const auto erased = properties_.erase(key);
    if (erased == 0)
    {
      return make_binding_error(
          BindingErrorCode::InvalidArgument,
          "object property was not found: " + key);
    }

    return ok();
  }

  void Object::clear() noexcept
  {
    properties_.clear();
  }

  std::size_t Object::size() const noexcept
  {
    return properties_.size();
  }

  bool Object::empty() const noexcept
  {
    return properties_.empty();
  }

  std::vector<std::string> Object::keys() const
  {
    std::vector<std::string> result;
    result.reserve(properties_.size());

    for (const auto &[key, value] : properties_)
    {
      (void)value;
      result.push_back(key);
    }

    return result;
  }

  std::vector<Value> Object::values() const
  {
    std::vector<Value> result;
    result.reserve(properties_.size());

    for (const auto &[key, value] : properties_)
    {
      (void)key;
      result.push_back(value);
    }

    return result;
  }

  const Object::PropertyMap &Object::properties() const noexcept
  {
    return properties_;
  }

  bool Object::equals(const Object &other) const noexcept
  {
    return name_ == other.name_ &&
           properties_ == other.properties_;
  }

  Error Object::validate_key(
      const std::string &key) const
  {
    if (key.empty())
    {
      return make_binding_error(
          BindingErrorCode::InvalidArgument,
          "object property key cannot be empty");
    }

    return ok();
  }

  bool operator==(const Object &lhs, const Object &rhs) noexcept
  {
    return lhs.equals(rhs);
  }

  bool operator!=(const Object &lhs, const Object &rhs) noexcept
  {
    return !lhs.equals(rhs);
  }

} // namespace kordex::bindings
