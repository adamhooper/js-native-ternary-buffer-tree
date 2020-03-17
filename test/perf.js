#!/usr/bin/env node

function buildWordListBuffer() {
  console.time('Building alphabetical word list')

  var alphabet = 'abcdefghijklmnopqrstuvwxyz';
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

  var tokensBuffer = Buffer.from(words.join('\n'), 'utf-8');

  console.timeEnd('Building alphabetical word list');
  console.log('%d words, %d bytes', words.length, tokensBuffer.length);
  return tokensBuffer;
}

function buildSearchStringBuffer() {
  console.time('Building search string')

  var alphabet = 'abcdefghijklmnopqrstuvwxyz';
  var searchWords = [];
  var a, b, c, d;

  for (a = 0; a < 26; a++) {
    for (b = 0; b < 26; b++) {
      for (c = 0; c < 26; c++) {
        searchWords.push(alphabet[a] + 'b' + alphabet[c] + alphabet[d] + 'e');
      }
    }
  }

  var searchBuffer = Buffer.from(searchWords.join(' ', 'utf-8'));

  console.timeEnd('Building search string');
  console.log('%d words, %d bytes', searchWords.length, searchBuffer.length);

  return searchBuffer;
}

function profileInitializeAndBuildSet(tokensBuffer) {
  var Set = require('../index');

  console.time('Initializing tree 5 times');
  var set;
  for (i = 0; i < 5; i++) {
    set = new Set(tokensBuffer);
  }
  console.timeEnd('Initializing tree 5 times');

  return set;
}

function profileSearch(set, searchBuffer) {
  console.time('Searching tree 2,000 times with maxNgramSize=4');
  for (i = 0; i < 2000; i++) {
    set.findAllMatches(searchBuffer, 2);
  }
  console.timeEnd('Searching tree 2,000 times with maxNgramSize=4');
}

function profileAll() {
  var tokensBuffer = buildWordListBuffer();

  var searchBuffer = buildSearchStringBuffer();

  var set = profileInitializeAndBuildSet(tokensBuffer);

  profileSearch(set, searchBuffer);
}

profileAll()

// Collect garbage, so Valgrind can find leaks
if (!global.gc) {
  console.warn('Running without garbage collection. Run node with --expose-gc to remove this warning and get better results');
} else {
  global.gc();
}
