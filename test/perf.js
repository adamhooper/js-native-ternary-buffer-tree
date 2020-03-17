#!/usr/bin/env node

function buildWordListBuffer() {
  console.log('Building alphabetical word list')

  const alphabet = 'abcdefghijklmnopqrstuvwxyz';
  const words = [];

  for (let a = 0; a < 26; a++) {
    for (let b = 0; b < 26; b++) {
      for (let c = 0; c < 26; c++) {
        for (let d = 0; d < 26; d++) {
          for (let e = 0; e < 10; e++) {
            words.push(alphabet[a] + alphabet[b] + alphabet[c] + alphabet[d] + alphabet[e]);
          }
        }
      }
    }
  }

  const tokensBuffer = Buffer.from(words.join('\n'), 'utf-8');

  console.log('%d words, %d bytes', words.length, tokensBuffer.length);
  return tokensBuffer;
}

function buildSearchStringBuffer() {
  console.log('Building search string')

  const alphabet = 'abcdefghijklmnopqrstuvwxyz';
  const searchWords = [];

  for (let a = 0; a < 26; a++) {
    for (let b = 0; b < 26; b++) {
      for (let c = 0; c < 26; c++) {
        searchWords.push(alphabet[a] + 'b' + alphabet[c] + 'de');
        searchWords.push(alphabet[a] + 'b' + alphabet[c] + 'dx');
      }
    }
  }

  const searchBuffer = Buffer.from(searchWords.join(' ', 'utf-8'));

  console.log('%d words, %d bytes', searchWords.length, searchBuffer.length);

  return searchBuffer;
}

function profileInitializeAndBuildSet(tokensBuffer) {
  const Set = require('../index');
  let set;

  console.time('Initializing tree 5 times');
  for (let i = 0; i < 5; i++) {
    set = new Set(tokensBuffer);
  }
  console.timeEnd('Initializing tree 5 times');

  return set;
}

function profileSearch(set, searchBuffer) {
  let ret

  console.time('Searching tree 100 times with maxNgramSize=2');
  for (let i = 0; i < 100; i++) {
    ret = set.findAllMatches(searchBuffer, 2);
  }
  console.timeEnd('Searching tree 100 times with maxNgramSize=2');

  console.log('Match length', ret.length, 'first match', ret[0]);
}

function profileAll() {
  const tokensBuffer = buildWordListBuffer();
  const searchBuffer = buildSearchStringBuffer();
  const set = profileInitializeAndBuildSet(tokensBuffer);

  profileSearch(set, searchBuffer);
}

profileAll()

// Collect garbage, so Valgrind can find leaks
if (!global.gc) {
  console.warn('Running without garbage collection. Run node with --expose-gc to remove this warning and get better results');
} else {
  global.gc();
}
