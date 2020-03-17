#include "TernarySearchTree.h"

#include "MemoryPool.h"

static const size_t NodePoolChunkSize = 65536; // 4096 is slower, according to perf.js

struct TSTNode
{
  char ch;
  TSTNode* left;
  TSTNode* eq;
  TSTNode* right;

  /**
   * Value of the node.
   *
   * If `maybeValue.has_value()`, a value is set. That value may be
   * `std::string_view(nullptr, 0)` -- this means "null" -- i.e., the user
   * said "value = null" (as opposed to value = undefined).
   */
  std::optional<std::string_view> maybeValue; // unset iff this is a leaf node

  explicit TSTNode(char aCh) : ch(aCh), left(nullptr), eq(nullptr), right(nullptr) {}

  // No destructor! There's no way to remove a TSTNode from a TernarySearchTree.
  // So the only way to destruct a TSTNode is to delete the TernarySearchTree.
  // Each TSTNode is managed by a MemoryPool, and the MemoryPool is owned by the
  // TernarySearchTree. So deleting a TernarySearchTree already releases memory
  // associated with _all_ its TSTNodes. A destroctor would be redundant.
};

struct TSTPriv {
  TSTNode* root;
  MemoryPool<TSTNode, NodePoolChunkSize> pool;

  TSTPriv() : root(nullptr) {}
  // No explicit destructor! `this-pool` dtor will release all TSTNodes,
  // including this->root.

  void
  insertNode(const std::string_view key, const std::string_view value)
  {
    if (key.size() == 0) return; // Empty string can't be a key

    TSTNode** nodeAddress = &this->root;
    std::string_view subkey(key);

    for (;;) {
      char ch = subkey[0];
      if (*nodeAddress == nullptr) {
        *nodeAddress = pool.newElement(ch);
        // Nodes get deleted by MemoryPool when we destroy it
      }

      TSTNode* node = *nodeAddress;

      if (ch < node->ch) {
        nodeAddress = &node->left;
      } else if (ch > node->ch) {
        nodeAddress = &node->right;
      } else if (subkey.size() > 1) {
        nodeAddress = &node->eq;
        subkey = subkey.substr(1);
      } else {
        // subkey.size() == 1. We've exhausted `key`, so this is the node where
        // we'll store our value.
        node->maybeValue = value;
        return;
      }
    }
  }

  const std::optional<std::string_view>
  lookupValue(const std::string_view key) const
  {
    if (key.size() == 0) return std::nullopt; // We don't store zero-length strings
    std::string_view subkey(key);

    const TSTNode* node = this->root;

    while (node != nullptr) {
      char ch = subkey[0];

      if (ch < node->ch) {
        node = node->left;
      } else if (ch > node->ch) {
        node = node->right;
      } else if (subkey.size() > 1) {
        node = node->eq;
        subkey = subkey.substr(1);
      } else {
        // we've reached the end of `key`. Here we are. Return either the
        // value or the lack thereof.
        return node->maybeValue;
      }
    }

    return std::nullopt;
  }
};

TernarySearchTree::TernarySearchTree() : priv(new TSTPriv())
{
}

TernarySearchTree::~TernarySearchTree()
{
  delete this->priv;
}

void
TernarySearchTree::insert(const std::string_view key, const std::string_view value)
{
  this->priv->insertNode(key, value);
}

bool
TernarySearchTree::contains(const std::string_view key) const
{
  return this->priv->lookupValue(key).has_value();
}

std::optional<std::string_view>
TernarySearchTree::get(const std::string_view key) const
{
  return this->priv->lookupValue(key);
}
