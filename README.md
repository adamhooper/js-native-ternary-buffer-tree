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

// Super-optimized method that scratches an itch we had
console.log(tree.findAllMatches('the foo drove over the moo', 2)); // [ 'the foo', 'foo', 'moo' ]

// Use as a map. Input is a single Buffer: tabs separate keys from values.
//
// Performance will be best if the keys are sorted in UTF-8 byte order.
var tree = new TernaryBufferTree(new Buffer('foo\tFOO\nbar\tBAR\nbaz\tBAZ', 'utf-8'));

console.log(tree.get('foo')); // 'FOO'
console.log(tree.get('moo')); // undefined
```

findAllMatches
--------------

This method is interesting in that it can search for tokens that span multiple
words (the second argument specifies the number of words), in a memory-efficient
manner. The memory used is the size of the output Array. The time complexity is
on the order of the size of the input times the number of tokens.

get
---

You can use TernaryBufferTree as a set or as a map. The input Buffer always
contains one key per line, but if you add a tab and then a value, the `get()`
method will return the given value for the given key.

If the given key has no value (i.e., there is no tab), then `get(key)` will
return `undefined`. In other words: `undefined` means either the key does not
exist, or the key has no value. Use `contains()` to determine which is the
case.

Developing
----------

Download, `npm install`.

Run `mocha -w` in the background as you implement features. Write tests in
`test` and code in `src`.

LICENSE
-------

AGPL-3.0. This project is (c) Overview Services Inc., Overview Computing Inc.
Please contact us should you desire a more permissive license.
