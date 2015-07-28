#!/usr/bin/env node

var Set = require('../index');

alphabet = 'abcdefghijklmnopqrstuvwxyz';

console.time('Building alphabetical word list')

var words = [];
var a, b, c, d, e, i;

for (a = 0; a < 26; a++) {
  for (b = 0; b < 26; b++) {
    for (c = 0; c < 26; c++) {
      for (d = 0; d < 26; d++) {
        for (e = 0; e < 10; e++) {
          words.push(alphabet[a] + alphabet[b] + alphabet[c] + alphabet[d] + alphabet[e]);
        }
      }
    }
  }
}

var tokensBuffer = new Buffer(words.join('\n'), 'utf-8');

console.timeEnd('Building alphabetical word list');
console.log('%d words, %d bytes', words.length, tokensBuffer.length);

console.time('Building search string')

var searchWords = [];

for (a = 0; a < 26; a++) {
  for (b = 0; b < 26; b++) {
    for (c = 0; c < 26; c++) {
      searchWords.push(alphabet[a] + 'b' + alphabet[c] + alphabet[d] + 'e');
    }
  }
}

var searchBuffer = new Buffer(searchWords.join(' ', 'utf-8'));

console.timeEnd('Building search string');
console.log('%d words, %d bytes', searchWords.length, searchBuffer.length);

console.time('Initializing tree 10 times');
var set;
for (i = 0; i < 10; i++) {
  set = new Set(tokensBuffer);
}
console.timeEnd('Initializing tree 10 times');

console.time('Searching tree 10 times with maxNgramSize=4');
for (i = 0; i < 10; i++) {
  set.findAllMatches(searchBuffer, 2);
}
console.timeEnd('Searching tree 10 times with maxNgramSize=4');
