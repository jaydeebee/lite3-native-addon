import { describe, it, expect } from 'vitest';
import { encode, decode, version, lite3Version } from '../src/index';

describe('version', () => {
  it('returns a semver-like version string', () => {
    expect(typeof version()).toBe('string');
    // Accepts semver with optional pre-release suffix (e.g., "1.0.0" or "0.0.0-unreleased")
    expect(version()).toMatch(/^\d+\.\d+\.\d+(-[\w.]+)?$/);
  });
});

describe('lite3Version', () => {
  it('returns lite3 library version with git hash', () => {
    expect(typeof lite3Version()).toBe('string');
    // Format: "1.0.0-<hash>" or "1.0.0-<hash>-dirty"
    expect(lite3Version()).toMatch(/^\d+\.\d+\.\d+-[a-f0-9]+(-dirty)?$/);
  });
});

describe('encode/decode roundtrip', () => {
  it('handles simple objects', () => {
    const obj = { foo: 'bar', num: 42 };
    const buf = encode(obj);
    expect(buf).toBeInstanceOf(Buffer);
    expect(decode(buf)).toEqual(obj);
  });

  it('handles strings', () => {
    const obj = { str: 'hello world' };
    expect(decode(encode(obj))).toEqual(obj);
  });

  it('handles integers', () => {
    const obj = { int: 12345, negative: -999 };
    expect(decode(encode(obj))).toEqual(obj);
  });

  it('handles floats', () => {
    const obj = { pi: 3.14159, e: 2.71828 };
    expect(decode(encode(obj))).toEqual(obj);
  });

  it('handles booleans', () => {
    const obj = { yes: true, no: false };
    expect(decode(encode(obj))).toEqual(obj);
  });

  it('handles null', () => {
    const obj = { nothing: null };
    expect(decode(encode(obj))).toEqual(obj);
  });

  it('handles arrays', () => {
    const obj = { items: [1, 2, 3, 'four', true, null] };
    expect(decode(encode(obj))).toEqual(obj);
  });

  it('handles nested objects', () => {
    const obj = {
      level1: {
        level2: {
          level3: {
            value: 'deep'
          }
        }
      }
    };
    expect(decode(encode(obj))).toEqual(obj);
  });

  it('handles mixed nested structures', () => {
    const obj = {
      users: [
        { name: 'Alice', age: 30 },
        { name: 'Bob', age: 25 }
      ],
      metadata: {
        count: 2,
        tags: ['active', 'verified']
      }
    };
    expect(decode(encode(obj))).toEqual(obj);
  });

  it('handles empty objects', () => {
    const obj = {};
    expect(decode(encode(obj))).toEqual(obj);
  });

  it('handles empty arrays', () => {
    const obj = { empty: [] };
    expect(decode(encode(obj))).toEqual(obj);
  });

  it('handles unicode strings', () => {
    const obj = { emoji: '🎉', chinese: '中文', mixed: 'hello 世界 🌍' };
    expect(decode(encode(obj))).toEqual(obj);
  });

  it('handles mixed-type arrays with nested objects', () => {
    const obj = {
      arr: [5, { a: 1 }, 'string', null],
      foo: 'bar',
      obj: { a: 1, b: 2 }
    };
    expect(decode(encode(obj))).toEqual(obj);
  });
});

describe('root-level arrays', () => {
  it('handles simple arrays', () => {
    const arr = [1, 2, 3];
    expect(decode(encode(arr))).toEqual(arr);
  });

  it('handles empty arrays', () => {
    const arr: unknown[] = [];
    expect(decode(encode(arr))).toEqual(arr);
  });

  it('handles arrays with strings', () => {
    const arr = ['hello', 'world'];
    expect(decode(encode(arr))).toEqual(arr);
  });

  it('handles arrays with mixed types', () => {
    const arr = [1, 'two', true, null, 3.14];
    expect(decode(encode(arr))).toEqual(arr);
  });

  it('handles nested arrays', () => {
    const arr = [[1, 2], [3, 4], [5, 6]];
    expect(decode(encode(arr))).toEqual(arr);
  });

  it('handles arrays with objects', () => {
    const arr = [{ a: 1 }, { b: 2 }, { c: 3 }];
    expect(decode(encode(arr))).toEqual(arr);
  });

  it('handles deeply nested structures in arrays', () => {
    const arr = [
      { user: { name: 'Alice', tags: ['admin', 'active'] } },
      { user: { name: 'Bob', tags: ['guest'] } }
    ];
    expect(decode(encode(arr))).toEqual(arr);
  });

  it('handles arrays with unicode', () => {
    const arr = ['🎉', '中文', 'hello 世界'];
    expect(decode(encode(arr))).toEqual(arr);
  });
});