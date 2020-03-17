js-native-ternary-buffer-tree
=============================

A native Node module implementing an ordered mapping of Buffers to Objects.

The goal is for super-duper-fast string matching. That means inputs and outputs
are always Buffers.

The implementation is a ternary search tree. That strikes a good balance:
memory usage isn't as excessive as with a hash table, and lookups aren't as
slow as with a binary search tree.

Usage
-----

```javascript
var TernaryBufferTree = require('js-native-ternary-buffer-tree');

// Use as a set. Input is a single Buffer: newline-separated Strings.
//
// Performance will be best if the Strings are sorted in UTF-8 byte order.
var tree = new TernaryBufferTree(new Buffer('bar\nbaz\nfoo\nmoo\nthe foo', 'utf-8'));

console.log(tree.contains('foo')); // true
console.log(tree.contains('moo')); // false

// Use as a map. Input is a single Buffer: tabs separate keys from values.
//
// When a key is queried without a value, `.get()` returns `null`. When a key
// does not exist, `.get()` returns `undefined`.
//
// Performance will be best if input keys are sorted in UTF-8 byte order.
var tree = new TernaryBufferTree(new Buffer('foo\tFOO\nbar\tBAR\nbaz\tBAZ\nx\ny', 'utf-8'));

console.log(tree.get('foo')); // 'FOO'
console.log(tree.get('x')); // null -- exists but has no value
console.log(tree.get('moo')); // undefined

// Super-optimized method that scratches an itch we had
//
// You can insert multi-word tokens and then query for single-word or
// multi-word combinations. The dataset needn't have any values; the query
// words must be space-separated.
var tree = new TernaryBufferTree(new Buffer('bar\nbaz\nfoo\nmoo\nthe foo', 'utf-8'));
console.log(tree.findAllMatches('the foo drove over the moo', 2)); // [ 'the foo', 'foo', 'moo' ]
```

contains(key)
-------------

`.contains(key)` is equivalent to `.get(key) !== undefined`.

`key` may be a `String` or a UTF-8 `Buffer`. `Buffer` logic should be more
efficient, as `String` input will be encoded as UTF-8 internally.

get(key)
--------

You can use TernaryBufferTree as a set or as a map. The input Buffer always
contains one key per line, but if you add a tab and then a value, the `get()`
method will return the given value for the given key.

If the key was provided without a value, `.get(key)` will return `null`. If
the key was not provided, `.get(key)` will return `undefined`.

`key` may be a `String` (and `.get(key)` will return a `String`); or it may be
a UTF-8 `Buffer` (and `.get(key)` will return a `Buffer`). `String` types imply
UTF-8 encoding and decoding; `Buffer` types are more direct because the
tree data is stored as UTF-8.

findAllMatches(tokens, maxNgramSize)
------------------------------------

This method is interesting in that it can search for tokens that span multiple
words (the second argument specifies the number of words), in a memory-efficient
manner. The memory used is the size of the output Array. The time complexity is
on the order of the size of the input times the number of tokens.

`tokens` may be a `String` (and `.findAllMatches(tokens, n)` will return an
`Array` of `String`s); or it may be a UTF-8 `Buffer` (and
`.findAllMatches(tokens, n)` will return an `Array` of `Buffer`s). `String`
types imply UTF-8 encoding and decoding; `Buffer` types are more direct because
the tree data is stored as UTF-8.

Developing
----------

Download, `npm install`.

Run `mocha -w` in the background as you implement features. Write tests in
`test` and code in `src`.

LICENSE
-------

AGPL-3.0. This project is (c) Overview Services Inc., Overview Computing Inc.
Please contact us should you desire a more permissive license.
