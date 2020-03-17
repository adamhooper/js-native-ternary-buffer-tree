#ifndef TERNARY_SEARCH_TREE_H
#define TERNARY_SEARCH_TREE_H

#include <optional>
#include <string_view>

class TSTPriv;

/**
 * Ternary search tree. Keys and values are `std::string_view`.
 *
 * Every string inserted into this tree will be an external one. That means the
 * caller must ensure none of those strings are freed until the
 * TernarySearchTree is deleted. Otherwise, undefined behavior!
 */
class TernarySearchTree
{
public:
  /**
   * Insert `value` into the map, with key `key`.
   *
   * `value` may be `std::string_view(nullptr, 0)` (the default-constructed
   * string_view). This means "null" -- i.e., the value is set to null, and
   * not undefined. (This is handy if you're storing a Set, not a Map.)
   *
   * `key` and `value` are borrowed. The buffer(s) they reference must exist as
   * long as the TernarySearchTree. Otherwise, undefined behavior!
   */
  void insert(const std::string_view key, const std::string_view value);

  /**
   * Return `true` iff `key` was previously inserted into this tree.
   */
  bool contains(const std::string_view key) const;

  /**
   * Return the `value` last inserted under key `key`.
   *
   * If `retval.has_value() == false`, no value was ever inserted. (In other
   * words, `this->contains(key) == false`.)
   */
  std::optional<std::string_view> get(const std::string_view key) const;

  TernarySearchTree();
  ~TernarySearchTree();

private:
  TSTPriv* priv;
};

#endif // TERNARY_SEARCH_TREE_H
