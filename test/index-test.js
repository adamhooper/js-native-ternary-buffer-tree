const Set = require('../index');
const expect = require('chai').expect;

describe('TernaryBufferTree', function() {
  it('should return true/false from .contains() with a Buffer argument', function() {
    const set = new Set(Buffer.from('foo\nbar\nbaz\ncafé\nthe foo', 'utf-8'));
    expect(set.contains(Buffer.from('foo', 'utf-8'))).to.be.true;
    expect(set.contains(Buffer.from('the foo', 'utf-8'))).to.be.true;
    expect(set.contains(Buffer.from('café', 'utf-8'))).to.be.true;
    expect(set.contains(Buffer.from('moo', 'utf-8'))).to.be.false;
    expect(set.contains(Buffer.from('fooX', 'utf-8'))).to.be.false;
  });

  it('should return true/false from .contains() with a String argument', function() {
    const set = new Set(Buffer.from('foo\nbar\nbaz\ncafé\nthe foo', 'utf-8'));
    expect(set.contains('foo')).to.be.true;
    expect(set.contains('the foo')).to.be.true;
    expect(set.contains('café')).to.be.true;
    expect(set.contains('moo')).to.be.false;
    expect(set.contains('fooX')).to.be.false;
  });

  it('should return a String Array of found tokens', function() {
    const set = new Set(Buffer.from('foo\nbar\nbaz\ncafé\nthe foo\nmoo', 'utf-8'));

    expect(set.findAllMatches('café the foo went over the moo', 2))
      .to.deep.eq([ 'café', 'the foo', 'foo', 'moo' ]);

    expect(set.findAllMatches('café the foo went over the moo', 1))
      .to.deep.eq([ 'café', 'foo', 'moo' ]);
  });

  it('should return a Buffer Array of found tokens', function() {
    const set = new Set(Buffer.from('foo\nbar\nbaz\ncafé\nthe foo\nmoo', 'utf-8'));

    expect(set.findAllMatches(Buffer.from('café the foo went over the moo'), 2))
      .to.deep.eq([ 'café', 'the foo', 'foo', 'moo' ].map(Buffer.from));

    expect(set.findAllMatches(Buffer.from('café the foo went over the moo'), 1))
      .to.deep.eq([ 'café', 'foo', 'moo' ].map(Buffer.from));
  });

  it('should return `null` from .get() when there are no values', function() {
    const set = new Set(Buffer.from('foo\nbar\nbaz', 'utf-8'));
    expect(set.get('foo')).to.be.null; // it exists but has no value
    expect(set.get('bar')).to.be.null;
    expect(set.get('baz')).to.be.null;
    expect(set.get('aoo')).to.be.undefined;
    expect(set.get('coo')).to.be.undefined;
    expect(set.get('moo')).to.be.undefined;
    expect(set.get(Buffer.from('foo'))).to.be.null; // it exists but has no value
    expect(set.get(Buffer.from('bar'))).to.be.null;
    expect(set.get(Buffer.from('baz'))).to.be.null;
    expect(set.get(Buffer.from('aoo'))).to.be.undefined;
    expect(set.get(Buffer.from('coo'))).to.be.undefined;
    expect(set.get(Buffer.from('moo'))).to.be.undefined;
  });

  it('should return values from `.get()` when there are values', function() {
    const set = new Set(Buffer.from('foo\tFOO\nbar\t\nbaz\tBAZ\nmoo\nmar', 'utf-8'));
    expect(set.get('foo')).to.eq('FOO');
    expect(set.get('bar')).to.eq('');
    expect(set.get('baz')).to.eq('BAZ');
    expect(set.get('moo')).to.be.null;
    expect(set.get('mar')).to.be.null;
    expect(set.get('aoo')).to.be.undefined;
    expect(set.get('bat')).to.be.undefined;
    expect(set.get('coo')).to.be.undefined;
    expect(set.get('zoo')).to.be.undefined;
    expect(set.get(Buffer.from('foo'))).to.deep.eq(Buffer.from('FOO'));
    expect(set.get(Buffer.from('bar'))).to.deep.eq(Buffer.from(''));
    expect(set.get(Buffer.from('baz'))).to.deep.eq(Buffer.from('BAZ'));
    expect(set.get(Buffer.from('moo'))).to.be.null;
    expect(set.get(Buffer.from('mar'))).to.be.null;
    expect(set.get(Buffer.from('aoo'))).to.be.undefined;
    expect(set.get(Buffer.from('bat'))).to.be.undefined;
    expect(set.get(Buffer.from('coo'))).to.be.undefined;
    expect(set.get(Buffer.from('zoo'))).to.be.undefined;
  });

  it('should reject zero-length strings', function() {
    const set = new Set(Buffer.from('foo\n'));
    expect(set.contains('')).to.be.false;
    expect(set.contains(Buffer.from(''))).to.be.false;
  });
});
