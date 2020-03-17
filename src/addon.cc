#include <napi.h>

#include "TernaryBufferTree.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports)
{
  return TernaryBufferTree::Init(env, exports);
}

NODE_API_MODULE(addon, InitAll);
