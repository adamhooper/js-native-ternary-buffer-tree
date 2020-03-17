#ifndef TERNARY_BUFFER_TREE_H
#define TERNARY_BUFFER_TREE_H

#include <string>

#include <napi.h>

#include "TernarySearchTree.h"

/**
 * JavaScript class that uses Node Buffers to expose a TernarySearchTree.
 */
class TernaryBufferTree : public Napi::ObjectWrap<TernaryBufferTree> {
public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  TernaryBufferTree(const Napi::CallbackInfo& info);

private:
  static Napi::FunctionReference constructor;

  Napi::Value Contains(const Napi::CallbackInfo& info);
  Napi::Value Get(const Napi::CallbackInfo& info);
  Napi::Value FindAllMatches(const Napi::CallbackInfo& info);

  std::string mem_;
  TernarySearchTree tree_;
};

#endif // TERNARY_BUFFER_TREE_H
