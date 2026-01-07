import { describe, it, expect, beforeAll } from 'vitest';
import {
  encode,
  decode,
  Lite3Buffer,
  $buffer,
  $decode,
  $isLite3Buffer,
} from '../src/index';

describe('Lite3Buffer', () => {
  describe('from() with objects', () => {
    const testObj = {
      name: 'Alice',
      age: 30,
      active: true,
      score: 95.5,
      nothing: null,
      nested: {
        city: 'NYC',
        zip: 10001,
      },
    };

    let proxy: typeof testObj;

    beforeAll(() => {
      proxy = Lite3Buffer.from(testObj);
    });

    it('creates a proxy from an object', () => {
      expect(Lite3Buffer.isLite3Buffer(proxy)).toBe(true);
    });

    it('accesses string properties', () => {
      expect(proxy.name).toBe('Alice');
    });

    it('accesses number properties', () => {
      expect(proxy.age).toBe(30);
      expect(proxy.score).toBeCloseTo(95.5);
    });

    it('accesses boolean properties', () => {
      expect(proxy.active).toBe(true);
    });

    it('accesses null properties', () => {
      expect(proxy.nothing).toBe(null);
    });

    it('returns undefined for non-existent keys', () => {
      expect((proxy as Record<string, unknown>).nonexistent).toBe(undefined);
    });

    it('accesses nested object properties', () => {
      expect(proxy.nested.city).toBe('NYC');
      expect(proxy.nested.zip).toBe(10001);
    });

    it('nested objects are also proxies', () => {
      expect(Lite3Buffer.isLite3Buffer(proxy.nested)).toBe(true);
    });

    it('maintains identity for nested access', () => {
      const nested1 = proxy.nested;
      const nested2 = proxy.nested;
      expect(nested1).toBe(nested2);
    });

    it('supports Object.keys()', () => {
      const keys = Object.keys(proxy);
      expect(keys).toContain('name');
      expect(keys).toContain('age');
      expect(keys).toContain('nested');
    });

    it('supports "in" operator', () => {
      expect('name' in proxy).toBe(true);
      expect('nonexistent' in proxy).toBe(false);
    });

    it('supports for...in iteration', () => {
      const keys: string[] = [];
      for (const key in proxy) {
        keys.push(key);
      }
      expect(keys).toContain('name');
      expect(keys).toContain('nested');
    });

    it('supports JSON.stringify()', () => {
      const json = JSON.stringify(proxy);
      const parsed = JSON.parse(json);
      expect(parsed.name).toBe('Alice');
      expect(parsed.nested.city).toBe('NYC');
    });

    it('supports spreading', () => {
      const spread = { ...proxy };
      expect(spread.name).toBe('Alice');
    });
  });

  describe('from() with arrays', () => {
    const testArr = ['a', 'b', 'c', 42, true, null];

    let proxy: typeof testArr;

    beforeAll(() => {
      proxy = Lite3Buffer.from(testArr);
    });

    it('creates a proxy from an array', () => {
      expect(Lite3Buffer.isLite3Buffer(proxy)).toBe(true);
    });

    it('has correct length', () => {
      expect(proxy.length).toBe(6);
    });

    it('accesses elements by index', () => {
      expect(proxy[0]).toBe('a');
      expect(proxy[1]).toBe('b');
      expect(proxy[3]).toBe(42);
      expect(proxy[4]).toBe(true);
      expect(proxy[5]).toBe(null);
    });

    it('maintains identity for repeated access', () => {
      const first1 = proxy[0];
      const first2 = proxy[0];
      // For primitives, they're equal by value
      expect(first1).toBe(first2);
    });

    it('supports for...of iteration', () => {
      const values: unknown[] = [];
      for (const val of proxy) {
        values.push(val);
      }
      expect(values).toEqual(['a', 'b', 'c', 42, true, null]);
    });

    it('supports map()', () => {
      const result = proxy.map((v, i) => `${i}:${v}`);
      expect(result[0]).toBe('0:a');
      expect(result[3]).toBe('3:42');
    });

    it('supports forEach()', () => {
      const values: unknown[] = [];
      proxy.forEach((v) => values.push(v));
      expect(values.length).toBe(6);
    });

    it('supports filter()', () => {
      const strings = proxy.filter((v) => typeof v === 'string');
      expect(strings).toEqual(['a', 'b', 'c']);
    });

    it('supports find()', () => {
      const found = proxy.find((v) => v === 42);
      expect(found).toBe(42);
    });

    it('supports findIndex()', () => {
      const index = proxy.findIndex((v) => v === 42);
      expect(index).toBe(3);
    });

    it('supports some()', () => {
      expect(proxy.some((v) => v === true)).toBe(true);
      expect(proxy.some((v) => v === 'nonexistent')).toBe(false);
    });

    it('supports every()', () => {
      expect(proxy.every((v) => v !== undefined)).toBe(true);
    });

    it('supports reduce()', () => {
      const nums = Lite3Buffer.from([1, 2, 3, 4]);
      const sum = nums.reduce((acc: number, v: number) => acc + v, 0);
      expect(sum).toBe(10);
    });

    it('supports includes()', () => {
      expect(proxy.includes('a')).toBe(true);
      expect(proxy.includes('z')).toBe(false);
    });

    it('supports indexOf()', () => {
      expect(proxy.indexOf('b')).toBe(1);
      expect(proxy.indexOf('z')).toBe(-1);
    });

    it('supports at()', () => {
      expect(proxy.at(0)).toBe('a');
      expect(proxy.at(-1)).toBe(null);
      expect(proxy.at(-2)).toBe(true);
    });

    it('supports slice()', () => {
      expect(proxy.slice(0, 2)).toEqual(['a', 'b']);
      expect(proxy.slice(-2)).toEqual([true, null]);
    });

    it('supports JSON.stringify()', () => {
      const json = JSON.stringify(proxy);
      const parsed = JSON.parse(json);
      expect(parsed).toEqual(['a', 'b', 'c', 42, true, null]);
    });
  });

  describe('from() with nested arrays and objects', () => {
    const testData = {
      users: [
        { name: 'Alice', scores: [95, 87, 92] },
        { name: 'Bob', scores: [88, 91, 85] },
      ],
      metadata: {
        total: 2,
        tags: ['active', 'verified'],
      },
    };

    let proxy: typeof testData;

    beforeAll(() => {
      proxy = Lite3Buffer.from(testData);
    });

    it('accesses deeply nested values', () => {
      expect(proxy.users[0].name).toBe('Alice');
      expect(proxy.users[1].scores[0]).toBe(88);
      expect(proxy.metadata.tags[1]).toBe('verified');
    });

    it('nested structures are proxies', () => {
      expect(Lite3Buffer.isLite3Buffer(proxy.users)).toBe(true);
      expect(Lite3Buffer.isLite3Buffer(proxy.users[0])).toBe(true);
      expect(Lite3Buffer.isLite3Buffer(proxy.users[0].scores)).toBe(true);
    });

    it('can iterate nested arrays', () => {
      const names = proxy.users.map((u) => u.name);
      expect(names).toEqual(['Alice', 'Bob']);
    });

    it('maintains identity across nested access', () => {
      const user1a = proxy.users[0];
      const user1b = proxy.users[0];
      expect(user1a).toBe(user1b);
    });
  });

  describe('from() with existing Buffer', () => {
    it('creates proxy from encoded buffer', () => {
      const obj = { foo: 'bar' };
      const buffer = encode(obj);
      const proxy = Lite3Buffer.from(buffer);

      expect(Lite3Buffer.isLite3Buffer(proxy)).toBe(true);
      expect((proxy as Record<string, unknown>).foo).toBe('bar');
    });
  });

  describe('symbol access', () => {
    const obj = { x: 1 };
    let proxy: typeof obj;

    beforeAll(() => {
      proxy = Lite3Buffer.from(obj);
    });

    it('$buffer returns the underlying buffer', () => {
      const buffer = (proxy as unknown as Record<symbol, Buffer>)[$buffer];
      expect(Buffer.isBuffer(buffer)).toBe(true);
    });

    it('$decode() returns fully decoded object', () => {
      const decoded = (proxy as unknown as Record<symbol, () => unknown>)[$decode]();
      expect(decoded).toEqual({ x: 1 });
      expect(Lite3Buffer.isLite3Buffer(decoded)).toBe(false);
    });

    it('$isLite3Buffer returns true', () => {
      expect((proxy as unknown as Record<symbol, boolean>)[$isLite3Buffer]).toBe(true);
    });
  });

  describe('utility methods', () => {
    it('isLite3Buffer() returns true for proxies', () => {
      const proxy = Lite3Buffer.from({ a: 1 });
      expect(Lite3Buffer.isLite3Buffer(proxy)).toBe(true);
    });

    it('isLite3Buffer() returns false for regular objects', () => {
      expect(Lite3Buffer.isLite3Buffer({ a: 1 })).toBe(false);
      expect(Lite3Buffer.isLite3Buffer(null)).toBe(false);
      expect(Lite3Buffer.isLite3Buffer(undefined)).toBe(false);
      expect(Lite3Buffer.isLite3Buffer(42)).toBe(false);
    });

    it('getBuffer() returns buffer from proxy', () => {
      const proxy = Lite3Buffer.from({ a: 1 });
      const buffer = Lite3Buffer.getBuffer(proxy);
      expect(Buffer.isBuffer(buffer)).toBe(true);
    });

    it('getBuffer() returns undefined for non-proxies', () => {
      expect(Lite3Buffer.getBuffer({ a: 1 })).toBe(undefined);
    });
  });

  describe('mixed array types (user test case)', () => {
    const testData = {
      arr: [5, { a: 1 }, 'string', null],
      foo: 'bar',
      obj: { a: 1, b: 2 },
    };

    let proxy: typeof testData;

    beforeAll(() => {
      proxy = Lite3Buffer.from(testData);
    });

    it('handles mixed-type arrays', () => {
      expect(proxy.arr[0]).toBe(5);
      expect((proxy.arr[1] as { a: number }).a).toBe(1);
      expect(proxy.arr[2]).toBe('string');
      expect(proxy.arr[3]).toBe(null);
    });

    it('handles object within array', () => {
      const nested = proxy.arr[1] as { a: number };
      expect(Lite3Buffer.isLite3Buffer(nested)).toBe(true);
      expect(nested.a).toBe(1);
    });

    it('JSON.stringify works with nested structures', () => {
      const json = JSON.stringify(proxy);
      const parsed = JSON.parse(json);
      expect(parsed).toEqual(testData);
    });
  });
});