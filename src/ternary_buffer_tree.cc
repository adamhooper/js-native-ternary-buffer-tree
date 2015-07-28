#include <cstring>
#include <deque>
#include <vector>

#include <node.h>
#include <node_buffer.h>
#include <node_object_wrap.h>
#include <v8.h>

#include "MemoryPool.h"

using namespace v8;

/**
 * A string whose characters are stored elsewhere.
 */
struct SimpleString
{
  const char* s;
  size_t len;

  SimpleString(const char* aS, size_t aLen) : s(aS), len(aLen) {}
  SimpleString() : s(NULL), len(0) {}
};

struct TSTNode
{
  char ch;
  TSTNode* left;
  TSTNode* eq;
  TSTNode* right;
  bool isLeaf;

  explicit TSTNode(char aCh)
    : ch(aCh),
      left(NULL),
      eq(NULL),
      right(NULL),
      isLeaf(false) {}

  ~TSTNode()
  {
    if (this->left) delete this->left;
    if (this->eq) delete this->eq;
    if (this->right) delete this->right;
  }
};

static const size_t NodePoolChunkSize = 65536; // 4096 is slower, according to perf.js

/**
 * Ternary search tree.
 *
 * Every string inserted into this tree will be an external one. That means the
 * caller must ensure none of those strings are freed until the TST is deleted.
 */
class TST
{
public:
  void insert(const char* s, size_t len)
  {
    if (len == 0) return; // We don't store zero-length strings

    this->_insert(&this->root, s, len);
  }

  bool contains(const char* s, size_t len) const
  {
    if (len == 0) return false; // We don't store zero-length strings

    return this->_contains(this->root, s, len);
  }

  explicit TST() : root(NULL) {}

private:
  TSTNode* root;
  MemoryPool<TSTNode,NodePoolChunkSize> pool;

  void _insert(TSTNode** nodeAddress, const char* s, size_t len) {
    char ch = *s;

    if (*nodeAddress == NULL) {
      *nodeAddress = pool.newElement(ch);
      // All the nodes get deleted by the MemoryPool when we destroy the TST
    }

    TSTNode* node = *nodeAddress;

    if (ch < node->ch) {
      this->_insert(&node->left, s, len);
    } else if (ch > node->ch) {
      this->_insert(&node->right, s, len);
    } else if (len > 1) {
      this->_insert(&node->eq, &s[1], len - 1);
    } else {
      node->isLeaf = true;
    }
  }

  bool _contains(TSTNode* node, const char* s, size_t len) const {
    if (node == NULL) return false;

    char ch = *s;

    if (ch < node->ch) {
      return this->_contains(node->left, s, len);
    } else if (ch > node->ch) {
      return this->_contains(node->right, s, len);
    } else if (len > 1) {
      return this->_contains(node->eq, &s[1], len - 1);
    } else {
      return node->isLeaf;
    }
  }
};

class TernaryBufferTree : public node::ObjectWrap {
public:
  static void Init(Handle<Object> exports);
  static Persistent<Function> constructor;

  inline bool contains(const char* s, size_t len);
  std::vector<SimpleString> findAllMatches(const char* s, size_t len, size_t maxNgramSize);

private:
  char* mem = NULL;
  TST tree;

  explicit TernaryBufferTree(const char* s, size_t len);
  ~TernaryBufferTree();

  static void New(const FunctionCallbackInfo<Value>& args);
  static void Contains(const FunctionCallbackInfo<Value>& args);
  static void FindAllMatches(const FunctionCallbackInfo<Value>& args);
};

Persistent<Function> TernaryBufferTree::constructor;

static size_t
count_char_in_str(char ch, const char* s, size_t len) {
  size_t ret = 0;

  while (len > 0) {
    if (*s == ch) ret++;
    len--;
    s++;
  }

  return ret;
}

/**
 * Insert Strings into a TST.
 *
 * We assume the Strings are sorted. Insert the median first, then recurse.
 * That's described in "Better Insertion Orders" at
 * http://www.drdobbs.com/database/ternary-search-trees/184410528?pgno=2
 */
static void
insert_many(TST& tree, SimpleString tokens[], size_t begin, size_t end)
{
  if (end == begin) return;

  size_t mid = (begin + end - 1) >> 1;
  tree.insert(tokens[mid].s, tokens[mid].len);

  if (mid > begin) insert_many(tree, tokens, begin, mid);
  if (mid < end - 1) insert_many(tree, tokens, mid + 1, end);
}

TernaryBufferTree::TernaryBufferTree(const char* s, size_t len)
{
  this->mem = new char[len];
  memcpy(this->mem, s, len);

  size_t nTokens = count_char_in_str('\n', s, len) + 1;
  SimpleString* tokens = new SimpleString[nTokens];

  SimpleString* curToken = &tokens[0];
  curToken->s = this->mem;

  const char* end = this->mem + len;
  for (const char* p = this->mem; p < end; p++) {
    if (*p == '\n') {
      curToken->len = p - curToken->s;
      curToken++;
      curToken->s = &p[1];
    }
  }
  curToken->len = end - curToken->s;

  insert_many(this->tree, tokens, 0, nTokens);

  delete tokens;
}

TernaryBufferTree::~TernaryBufferTree()
{
  if (this->mem) delete this->mem;
}

bool
TernaryBufferTree::contains(const char* s, size_t len)
{
  return this->tree.contains(s, len);
}

std::vector<SimpleString>
TernaryBufferTree::findAllMatches(const char* s, size_t len, size_t maxNgramSize) {
  std::vector<SimpleString> ret;
  std::deque<const char*> tokenStarts;
  const char* lastP = s;
  const char* end = s + len;

  tokenStarts.push_back(s);

  while (true) {
    const char* p = static_cast<const char*>(memchr(lastP, ' ', end - lastP));
    if (p == NULL) p = s + len;

    // Add s[tokenStarts[0],pos), s[tokenStarts[1],pos), ... for every token
    // in the set
    for (auto i = tokenStarts.begin(); i < tokenStarts.end(); i++) {
      const char* tokenStart = *i;
      if (this->tree.contains(tokenStart, p - tokenStart)) {
        ret.push_back(SimpleString(tokenStart, p - tokenStart));
      }
    }

    if (tokenStarts.size() == maxNgramSize) tokenStarts.pop_front();

    if (p == s + len) break;

    lastP = p + 1;
    tokenStarts.push_back(lastP);
  }

  return ret;
}

void
TernaryBufferTree::Init(Handle<Object> exports) {
  Isolate* isolate = Isolate::GetCurrent();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "TernaryBufferTree"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  NODE_SET_PROTOTYPE_METHOD(tpl, "contains", Contains);
  NODE_SET_PROTOTYPE_METHOD(tpl, "findAllMatches", FindAllMatches);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "TernaryBufferTree"), tpl->GetFunction());
}

void
TernaryBufferTree::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  if (args.IsConstructCall()) {
    // Invoked as constructor: `new MyObject(...)`
    const char* s = node::Buffer::Data(args[0]);
    const size_t len = node::Buffer::Length(args[0]);
    TernaryBufferTree* obj = new TernaryBufferTree(s, len);
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    // Invoked as plain function `MyObject(...)`, turn into construct call
    Local<Value> argv[1] = { args[0] };
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(cons->NewInstance(1, argv));
  }
}

void TernaryBufferTree::Contains(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  TernaryBufferTree* obj = ObjectWrap::Unwrap<TernaryBufferTree>(args.Holder());

  bool ret = false;

  Local<Value> arg = args[0]; // Buffer or String

  if (node::Buffer::HasInstance(arg)) {
    // It's a buffer. Go char-by-char
    const char* data(node::Buffer::Data(arg));
    const size_t len(node::Buffer::Length(arg));

    ret = obj->contains(data, len);
  } else {
    // We can convert it to utf-8. On failure, it's just an empty String.
    String::Utf8Value argString(arg);
    ret = obj->contains(*argString, argString.length());
  }

  args.GetReturnValue().Set(ret);
}

void
TernaryBufferTree::FindAllMatches(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  TernaryBufferTree* obj = ObjectWrap::Unwrap<TernaryBufferTree>(args.Holder());

  std::vector<SimpleString> ret;
  Handle<Array> retArray;

  Local<Value> arg = args[0]; // Buffer or String
  uint32_t maxNgramSize = args[1]->Uint32Value();
  if (maxNgramSize == 0) maxNgramSize = 1;

  if (node::Buffer::HasInstance(arg)) {
    const char* data(node::Buffer::Data(arg));
    const size_t len(node::Buffer::Length(arg));
    ret = obj->findAllMatches(data, len, maxNgramSize);

    // Icky copy/paste, I know
    size_t size = ret.size();
    retArray = Array::New(isolate, size);
    for (size_t i = 0; i < size; i++) {
      retArray->Set(i, String::NewFromUtf8(isolate, ret[i].s, String::NewStringType::kNormalString, ret[i].len));
    }
  } else {
    // We can convert it to utf-8. On failure, it's just an empty String.
    String::Utf8Value argString(arg);
    ret = obj->findAllMatches(*argString, argString.length(), maxNgramSize);

    // Icky copy/paste, I know
    size_t size = ret.size();
    retArray = Array::New(isolate, size);
    for (size_t i = 0; i < size; i++) {
      retArray->Set(i, String::NewFromUtf8(isolate, ret[i].s, String::NewStringType::kNormalString, ret[i].len));
    }
  }

  args.GetReturnValue().Set(retArray);
}

void
init(Handle<Object> exports, Handle<Object> module) {
  TernaryBufferTree::Init(exports);
}

NODE_MODULE(binding, init);
