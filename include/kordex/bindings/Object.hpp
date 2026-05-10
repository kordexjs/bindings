/**
 *
 *  @file Object.hpp
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

#ifndef KORDEX_BINDINGS_OBJECT_HPP
#define KORDEX_BINDINGS_OBJECT_HPP

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include <kordex/bindings/Error.hpp>
#include <kordex/bindings/Result.hpp>
#include <kordex/bindings/Value.hpp>

namespace kordex::bindings
{
  /**
   * @class Object
   * @brief Engine-independent object representation.
   *
   * Object stores named Value properties. It is used as a lightweight
   * representation before values are attached to a concrete JavaScript engine.
   */
  class Object
  {
  public:
    /**
     * @brief Property storage type.
     */
    using PropertyMap = std::map<std::string, Value>;

    /**
     * @brief Construct an empty object.
     */
    Object() = default;

    /**
     * @brief Construct an object with an optional name.
     */
    explicit Object(std::string name);

    /**
     * @brief Create an empty object.
     */
    [[nodiscard]] static Object create();

    /**
     * @brief Create a named object.
     */
    [[nodiscard]] static Object create(std::string name);

    /**
     * @brief Return the object name.
     */
    [[nodiscard]] const std::string &name() const noexcept;

    /**
     * @brief Set the object name.
     */
    void set_name(std::string name);

    /**
     * @brief Return true if the object has a name.
     */
    [[nodiscard]] bool has_name() const noexcept;

    /**
     * @brief Set a property value.
     */
    [[nodiscard]] Error set(
        std::string key,
        Value value);

    /**
     * @brief Return a property value or an error.
     */
    [[nodiscard]] Result<Value> get(
        const std::string &key) const;

    /**
     * @brief Return true if a property exists.
     */
    [[nodiscard]] bool has(
        const std::string &key) const noexcept;

    /**
     * @brief Remove a property.
     */
    [[nodiscard]] Error remove(
        const std::string &key);

    /**
     * @brief Clear all properties.
     */
    void clear() noexcept;

    /**
     * @brief Return the number of properties.
     */
    [[nodiscard]] std::size_t size() const noexcept;

    /**
     * @brief Return true if the object has no properties.
     */
    [[nodiscard]] bool empty() const noexcept;

    /**
     * @brief Return all property names.
     */
    [[nodiscard]] std::vector<std::string> keys() const;

    /**
     * @brief Return all property values.
     */
    [[nodiscard]] std::vector<Value> values() const;

    /**
     * @brief Return the underlying property map.
     */
    [[nodiscard]] const PropertyMap &properties() const noexcept;

    /**
     * @brief Return true if two objects have the same name and properties.
     */
    [[nodiscard]] bool equals(const Object &other) const noexcept;

  private:
    [[nodiscard]] Error validate_key(
        const std::string &key) const;

    std::string name_{};
    PropertyMap properties_{};
  };

  /**
   * @brief Compare two objects.
   */
  [[nodiscard]] bool operator==(const Object &lhs, const Object &rhs) noexcept;

  /**
   * @brief Compare two objects.
   */
  [[nodiscard]] bool operator!=(const Object &lhs, const Object &rhs) noexcept;

} // namespace kordex::bindings

#endif // KORDEX_BINDINGS_OBJECT_HPP
