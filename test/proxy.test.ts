import { describe, it, expect, beforeAll } from 'vitest';
import {
  encode,
  getRootType,
  getType,
  getArrayType,
  getValue,
  getArrayElement,
  getChildOffset,
  getArrayChildOffset,
  getKeys,
  getLength,
  hasKey,
} from '../src/index';

describe('proxy functions', () => {
  const testObj = {
    name: 'test',
    count: 42,
    price: 19.99,
    active: true,
    nothing: null,
    nested: {
      deep: 'value',
      num: 3.14,
    },
    items: ['a', 'b', 'c'],
    mixed: [1, 'two', { three: 3 }],
  };

  let buf: Buffer;

  beforeAll(() => {
    buf = encode(testObj);
  });

  describe('getRootType', () => {
    it('returns object for object root', () => {
      expect(getRootType(buf)).toBe('object');
    });

    it('returns array for array root', () => {
      const arrBuf = encode([1, 2, 3]);
      expect(getRootType(arrBuf)).toBe('array');
    });
  });

  describe('getType', () => {
    it('returns string for string property', () => {
      expect(getType(buf, 0, 'name')).toBe('string');
    });

    it('returns number for integer property', () => {
      expect(getType(buf, 0, 'count')).toBe('number');
    });

    it('returns number for float property', () => {
      expect(getType(buf, 0, 'price')).toBe('number');
    });

    it('returns boolean for boolean property', () => {
      expect(getType(buf, 0, 'active')).toBe('boolean');
    });

    it('returns null for null property', () => {
      expect(getType(buf, 0, 'nothing')).toBe('null');
    });

    it('returns object for nested object', () => {
      expect(getType(buf, 0, 'nested')).toBe('object');
    });

    it('returns array for array property', () => {
      expect(getType(buf, 0, 'items')).toBe('array');
    });

    it('returns undefined for non-existent property', () => {
      expect(getType(buf, 0, 'nonexistent')).toBe('undefined');
    });
  });

  describe('getValue', () => {
    it('returns string value', () => {
      expect(getValue(buf, 0, 'name')).toBe('test');
    });

    it('returns integer value', () => {
      expect(getValue(buf, 0, 'count')).toBe(42);
    });

    it('returns float value', () => {
      expect(getValue(buf, 0, 'price')).toBeCloseTo(19.99);
    });

    it('returns boolean value', () => {
      expect(getValue(buf, 0, 'active')).toBe(true);
    });

    it('returns null value', () => {
      expect(getValue(buf, 0, 'nothing')).toBe(null);
    });

    it('returns offset for nested object', () => {
      const offset = getValue(buf, 0, 'nested');
      expect(typeof offset).toBe('number');
      expect(offset).toBeGreaterThan(0);
    });
  });

  describe('getKeys', () => {
    it('returns all keys of root object', () => {
      const keys = getKeys(buf, 0);
      expect(keys).toContain('name');
      expect(keys).toContain('count');
      expect(keys).toContain('nested');
      expect(keys).toContain('items');
    });

    it('returns keys of nested object', () => {
      const nestedOffset = getChildOffset(buf, 0, 'nested');
      const keys = getKeys(buf, nestedOffset);
      expect(keys).toContain('deep');
      expect(keys).toContain('num');
    });
  });

  describe('getLength', () => {
    it('returns length of root object', () => {
      const len = getLength(buf, 0);
      expect(len).toBe(Object.keys(testObj).length);
    });

    it('returns length of array', () => {
      const itemsOffset = getChildOffset(buf, 0, 'items');
      expect(getLength(buf, itemsOffset)).toBe(3);
    });
  });

  describe('hasKey', () => {
    it('returns true for existing key', () => {
      expect(hasKey(buf, 0, 'name')).toBe(true);
    });

    it('returns false for non-existent key', () => {
      expect(hasKey(buf, 0, 'nonexistent')).toBe(false);
    });
  });

  describe('getChildOffset', () => {
    it('returns offset for nested object', () => {
      const offset = getChildOffset(buf, 0, 'nested');
      expect(typeof offset).toBe('number');
      expect(offset).toBeGreaterThan(0);
    });

    it('returns offset for array', () => {
      const offset = getChildOffset(buf, 0, 'items');
      expect(typeof offset).toBe('number');
      expect(offset).toBeGreaterThan(0);
    });

    it('throws for primitive property', () => {
      expect(() => getChildOffset(buf, 0, 'name')).toThrow();
    });
  });

  describe('array access', () => {
    let itemsOffset: number;
    let mixedOffset: number;

    beforeAll(() => {
      itemsOffset = getChildOffset(buf, 0, 'items');
      mixedOffset = getChildOffset(buf, 0, 'mixed');
    });

    describe('getArrayType', () => {
      it('returns type of string element', () => {
        expect(getArrayType(buf, itemsOffset, 0)).toBe('string');
      });

      it('returns type of number element', () => {
        expect(getArrayType(buf, mixedOffset, 0)).toBe('number');
      });

      it('returns type of object element', () => {
        expect(getArrayType(buf, mixedOffset, 2)).toBe('object');
      });
    });

    describe('getArrayElement', () => {
      it('returns string element', () => {
        expect(getArrayElement(buf, itemsOffset, 0)).toBe('a');
        expect(getArrayElement(buf, itemsOffset, 1)).toBe('b');
        expect(getArrayElement(buf, itemsOffset, 2)).toBe('c');
      });

      it('returns number element', () => {
        expect(getArrayElement(buf, mixedOffset, 0)).toBe(1);
      });

      it('returns string element from mixed array', () => {
        expect(getArrayElement(buf, mixedOffset, 1)).toBe('two');
      });

      it('returns offset for nested object in array', () => {
        const nestedObjOffset = getArrayElement(buf, mixedOffset, 2);
        expect(typeof nestedObjOffset).toBe('number');
      });
    });

    describe('getArrayChildOffset', () => {
      it('returns offset for nested object in array', () => {
        const offset = getArrayChildOffset(buf, mixedOffset, 2);
        expect(typeof offset).toBe('number');

        // Verify we can access the nested object
        const keys = getKeys(buf, offset);
        expect(keys).toContain('three');
        expect(getValue(buf, offset, 'three')).toBe(3);
      });

      it('throws for primitive element', () => {
        expect(() => getArrayChildOffset(buf, itemsOffset, 0)).toThrow();
      });
    });
  });

  describe('nested navigation', () => {
    it('can navigate deep into nested structures', () => {
      // Access testObj.nested.deep via proxy functions
      const nestedOffset = getChildOffset(buf, 0, 'nested');
      const deepValue = getValue(buf, nestedOffset, 'deep');
      expect(deepValue).toBe('value');
    });

    it('can access nested object in array and read its properties', () => {
      // Access testObj.mixed[2].three
      const mixedOffset = getChildOffset(buf, 0, 'mixed');
      const objOffset = getArrayChildOffset(buf, mixedOffset, 2);
      const threeValue = getValue(buf, objOffset, 'three');
      expect(threeValue).toBe(3);
    });
  });
});