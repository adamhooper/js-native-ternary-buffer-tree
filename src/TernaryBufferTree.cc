#include <algorithm>
#include <cstring> // memchr()
#include <deque>
#include <memory>
#include <string_view>
#include <vector>

#include "TernaryBufferTree.h"

Napi::FunctionReference TernaryBufferTree::constructor;

Napi::Object TernaryBufferTree::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "TernaryBufferTree", {
      InstanceMethod("contains", &TernaryBufferTree::Contains),
      InstanceMethod("get", &TernaryBufferTree::Get),
      InstanceMethod("findAllMatches", &TernaryBufferTree::FindAllMatches)
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("TernaryBufferTree", func);
  return exports;
}

struct KeyValue {
  std::string_view key;
  std::string_view value;
};

/**
 * `view`, plus its backing store (`mem`) if needed.
 *
 * Building a StringValue from a JS Buffer value doesn't require a new backing
 * store because the JS Buffer is the backing store. But building a StringValue
 * from a JS String _does_, because a JS String is natively UTF-16 so it must
 * be encoded as UTF-8 to create the view.
 */
struct StringHandle {
  const std::unique_ptr<std::string> mem; // if needed
  const std::string_view view;
};

static StringHandle
valueToStringHandle(const Napi::Value& value)
{
  if (value.IsBuffer()) {
    auto buf = value.As<Napi::Buffer<char>>();
    return { nullptr, std::string_view(buf.Data(), buf.Length()) };
  } else {
    // Convert to String. 
    Napi::String jsString = value.IsString() ? value.As<Napi::String>() : value.ToString();
    // UTF-8 encode
    auto mem = std::make_unique<std::string>(jsString.Utf8Value());
    std::string_view view(*mem);
    return { std::move(mem), view };
  }
}

static Napi::Value
stringViewToValue(const Napi::Env env, const std::string_view view, bool isBuffer)
{
  if (isBuffer) {
    return Napi::Buffer<char>::Copy(env, view.data(), view.size());
  } else {
    return Napi::String::New(env, view.data(), view.size());
  }
}

/**
 * Insert Strings into a TernaryBufferTree.
 *
 * We assume the Strings are sorted. Insert the median first, then recurse.
 * That's described in "Better Insertion Orders" at
 * http://www.drdobbs.com/database/ternary-search-trees/184410528?pgno=2
 */
static void
insertMany(TernarySearchTree& tree, const KeyValue* tokens, size_t len)
{
  if (len == 0) return;

  size_t mid = len >> 1;
  tree.insert(tokens[mid].key, tokens[mid].value);

  if (mid > 0) insertMany(tree, tokens, mid); // insert tokens to the left
  if (mid < len - 1) insertMany(tree, &tokens[mid + 1], len - mid - 1);
}

static std::vector<KeyValue>
parseBytesIntoKeyValues(const std::string_view utf8)
{
  size_t nKeyValues = std::count(utf8.cbegin(), utf8.cend(), '\n') + 1;
  std::vector<KeyValue> ret(nKeyValues); // all null strings

  size_t index(0);
  const char* keyStart(&utf8[0]);
  size_t keySize(0);
  const char* valueStart = nullptr;

#define INSERT_KEYVALUE(p) \
  do { \
    if (valueStart == nullptr) { \
      /* There was no value this line. Key maps to "null" */ \
      std::string_view key(keyStart, p - keyStart); \
      std::string_view value; \
      ret[index] = KeyValue { key, value }; \
    } else { \
      std::string_view key(keyStart, keySize); \
      std::string_view value(valueStart, p - valueStart); \
      ret[index] = KeyValue { key, value }; \
    } \
    index++; \
    keyStart = &p[1]; /* Start parsing the next line (after "\n") */ \
    valueStart = nullptr; \
  } while (0)

  for (const char* p = utf8.cbegin(); p < utf8.cend(); p++) {
    switch (*p) {
      case '\t':
        keySize = p - keyStart;
        valueStart = &p[1]; // Start parsing the value
        break;
      case '\n':
        INSERT_KEYVALUE(p);
        break;
    }
  }
  INSERT_KEYVALUE(utf8.cend());
#undef INSERT_KEYVALUE

  return ret;
}

static std::vector<std::string_view>
findAllMatches(const TernarySearchTree& tree, const std::string_view text, size_t maxNgramSize)
{
  std::vector<std::string_view> ret;
  std::deque<const char*> tokenStarts;
  const char* s = text.cbegin();
  const char* end = text.cend();

  tokenStarts.push_back(s);

  while (s < end) {
    const char* p  = std::find(s, end, ' '); // `p = end` if ' ' not found

    // Add s[tokenStarts[0],pos), s[tokenStarts[1],pos), ... for every token
    // in the set
    for (const char* tokenStart : tokenStarts) {
      std::string_view token(tokenStart, p - tokenStart);
      if (tree.contains(token)) {
        ret.push_back(token);
      }
    }

    if (tokenStarts.size() == maxNgramSize) tokenStarts.pop_front();

    s = p + 1;
    tokenStarts.push_back(s);
  }

  return ret;
}

TernaryBufferTree::TernaryBufferTree(const Napi::CallbackInfo& info)
  : Napi::ObjectWrap<TernaryBufferTree>(info)
  , tree_()
{
  Napi::Env env(info.Env());
  Napi::HandleScope scope(env);

  if (info.Length() != 1 || !(info[0].IsString() || info[0].IsBuffer())) {
    Napi::TypeError::New(env, "Usage: new TernaryBufferTree(<String or Buffer>)").ThrowAsJavaScriptException();
    return;
  }

  if (info[0].IsBuffer()) {
    auto buf = info[0].As<Napi::Buffer<char>>();
    this->mem_ = std::string(buf.Data(), &buf.Data()[buf.Length()]);
  } else {
    auto str = info[0].As<Napi::String>();
    this->mem_ = str.Utf8Value();
  }

  std::vector<KeyValue> keyValues(parseBytesIntoKeyValues(this->mem_));
  insertMany(this->tree_, &keyValues[0], keyValues.size());
}


Napi::Value
TernaryBufferTree::Contains(const Napi::CallbackInfo& info)
{
  Napi::Env env(info.Env());

  if (info.Length() != 1 || !(info[0].IsString() || info[0].IsBuffer())) {
    Napi::TypeError::New(env, "Usage: tree.contains(<String or Buffer>)").ThrowAsJavaScriptException();
    return env.Null();
  }

  StringHandle inString = valueToStringHandle(info[0]);
  bool isContained = this->tree_.contains(inString.view);
  return Napi::Boolean::New(env, isContained);
}

Napi::Value
TernaryBufferTree::Get(const Napi::CallbackInfo& info)
{
  Napi::Env env(info.Env());

  if (info.Length() != 1 || !(info[0].IsString() || info[0].IsBuffer())) {
    Napi::TypeError::New(env, "Usage: tree.get(<String or Buffer>)").ThrowAsJavaScriptException();
    return env.Null();
  }

  StringHandle inString = valueToStringHandle(info[0]);
  std::optional<std::string_view> maybeOutView = this->tree_.get(inString.view);
  if (maybeOutView.has_value()) {
    if (maybeOutView->data() == nullptr) { // default-ctor string_view
      return env.Null(); // user inserted null
    } else {
      return stringViewToValue(env, maybeOutView.value(), info[0].IsBuffer());
    }
  } else {
    return env.Undefined();
  }
}

Napi::Value
TernaryBufferTree::FindAllMatches(const Napi::CallbackInfo& info)
{
  Napi::Env env(info.Env());

  if (info.Length() != 2 || !(info[0].IsString() || info[0].IsBuffer()) || !info[1].IsNumber()) {
    Napi::TypeError::New(env, "Usage: tree.findAllMatches(<String or Buffer>, <Integer>)").ThrowAsJavaScriptException();
    return env.Null();
  }

  StringHandle inString = valueToStringHandle(info[0]);
  uint32_t maxNgramSize(info[1].ToNumber().Uint32Value());

  std::vector<std::string_view> retViews(findAllMatches(this->tree_, inString.view, maxNgramSize));

  Napi::Array retval = Napi::Array::New(env, retViews.size());
  uint32_t index(0);

  bool isBuffer = info[0].IsBuffer();
  for (const std::string_view& sv : retViews) {
    retval.Set(index, stringViewToValue(env, sv, isBuffer));
    index++;
  }

  return retval;
}
