/**
 * Lite3Buffer - Lazy proxy wrapper for Lite3 binary buffers
 *
 * Provides transparent access to Lite3-encoded data without eager decoding.
 * Access properties/elements on-demand; nested structures return new proxies.
 */

import {
  encode,
  decode,
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
  type Lite3Serializable,
  type Lite3TypeString,
} from './index';

/** Symbol for accessing the underlying buffer */
export const $buffer = Symbol.for('lite3.buffer');

/** Symbol for forcing full decode */
export const $decode = Symbol.for('lite3.decode');

/** Symbol to check if value is a Lite3Buffer proxy */
export const $isLite3Buffer = Symbol.for('lite3.isLite3Buffer');

type ProxyCache = Map<string | number, unknown>;

interface Lite3ProxyState {
  buffer: Buffer;
  offset: number;
  isArray: boolean;
  cache: ProxyCache;
}

function createObjectProxy<T extends object>(state: Lite3ProxyState): T {
  const { buffer, offset, cache } = state;

  return new Proxy({} as T, {
    get(_target, prop: string | symbol): unknown {
      // Handle symbols
      if (prop === $buffer) return buffer;
      if (prop === $isLite3Buffer) return true;
      if (prop === $decode) {
        return () => decode(buffer);
      }

      // Handle special properties
      if (typeof prop === 'symbol') return undefined;

      // Check cache first (for identity consistency)
      if (cache.has(prop)) {
        return cache.get(prop);
      }

      // Get type and value
      const type = getType(buffer, offset, prop);

      if (type === 'undefined') {
        return undefined;
      }

      let result: unknown;

      if (type === 'object') {
        const childOffset = getChildOffset(buffer, offset, prop);
        result = createObjectProxy({
          buffer,
          offset: childOffset,
          isArray: false,
          cache: new Map(),
        });
      } else if (type === 'array') {
        const childOffset = getChildOffset(buffer, offset, prop);
        result = createArrayProxy({
          buffer,
          offset: childOffset,
          isArray: true,
          cache: new Map(),
        });
      } else {
        // Primitive value
        result = getValue(buffer, offset, prop);
      }

      // Cache for identity consistency
      cache.set(prop, result);
      return result;
    },

    has(_target, prop: string | symbol): boolean {
      if (typeof prop === 'symbol') {
        return prop === $buffer || prop === $decode || prop === $isLite3Buffer;
      }
      return hasKey(buffer, offset, prop);
    },

    ownKeys(): string[] {
      return getKeys(buffer, offset);
    },

    getOwnPropertyDescriptor(_target, prop: string | symbol) {
      if (typeof prop === 'symbol') return undefined;
      if (!hasKey(buffer, offset, prop)) return undefined;
      return {
        enumerable: true,
        configurable: true,
        writable: false,
      };
    },
  });
}

function createArrayProxy<T extends unknown[]>(state: Lite3ProxyState): T {
  const { buffer, offset, cache } = state;
  const length = getLength(buffer, offset);

  return new Proxy([] as unknown as T, {
    get(_target, prop: string | symbol): unknown {
      // Handle symbols
      if (prop === $buffer) return buffer;
      if (prop === $isLite3Buffer) return true;
      if (prop === $decode) {
        return () => decode(buffer);
      }

      // Handle array length
      if (prop === 'length') return length;

      // Handle symbol.iterator for for...of loops
      if (prop === Symbol.iterator) {
        return function* () {
          for (let i = 0; i < length; i++) {
            yield getElementAt(i);
          }
        };
      }

      // Handle numeric indices
      if (typeof prop === 'string') {
        const index = Number(prop);
        if (!Number.isNaN(index) && Number.isInteger(index) && index >= 0 && index < length) {
          return getElementAt(index);
        }
      }

      // Handle array methods that should work
      if (prop === 'map') {
        return (fn: (value: unknown, index: number) => unknown) => {
          const result = [];
          for (let i = 0; i < length; i++) {
            result.push(fn(getElementAt(i), i));
          }
          return result;
        };
      }

      if (prop === 'forEach') {
        return (fn: (value: unknown, index: number) => void) => {
          for (let i = 0; i < length; i++) {
            fn(getElementAt(i), i);
          }
        };
      }

      if (prop === 'filter') {
        return (fn: (value: unknown, index: number) => boolean) => {
          const result = [];
          for (let i = 0; i < length; i++) {
            const val = getElementAt(i);
            if (fn(val, i)) result.push(val);
          }
          return result;
        };
      }

      if (prop === 'find') {
        return (fn: (value: unknown, index: number) => boolean) => {
          for (let i = 0; i < length; i++) {
            const val = getElementAt(i);
            if (fn(val, i)) return val;
          }
          return undefined;
        };
      }

      if (prop === 'findIndex') {
        return (fn: (value: unknown, index: number) => boolean) => {
          for (let i = 0; i < length; i++) {
            if (fn(getElementAt(i), i)) return i;
          }
          return -1;
        };
      }

      if (prop === 'some') {
        return (fn: (value: unknown, index: number) => boolean) => {
          for (let i = 0; i < length; i++) {
            if (fn(getElementAt(i), i)) return true;
          }
          return false;
        };
      }

      if (prop === 'every') {
        return (fn: (value: unknown, index: number) => boolean) => {
          for (let i = 0; i < length; i++) {
            if (!fn(getElementAt(i), i)) return false;
          }
          return true;
        };
      }

      if (prop === 'reduce') {
        return (fn: (acc: unknown, value: unknown, index: number) => unknown, initial?: unknown) => {
          let acc = initial;
          let startIndex = 0;
          if (acc === undefined && length > 0) {
            acc = getElementAt(0);
            startIndex = 1;
          }
          for (let i = startIndex; i < length; i++) {
            acc = fn(acc, getElementAt(i), i);
          }
          return acc;
        };
      }

      if (prop === 'includes') {
        return (searchElement: unknown) => {
          for (let i = 0; i < length; i++) {
            if (getElementAt(i) === searchElement) return true;
          }
          return false;
        };
      }

      if (prop === 'indexOf') {
        return (searchElement: unknown) => {
          for (let i = 0; i < length; i++) {
            if (getElementAt(i) === searchElement) return i;
          }
          return -1;
        };
      }

      if (prop === 'at') {
        return (index: number) => {
          if (index < 0) index = length + index;
          if (index < 0 || index >= length) return undefined;
          return getElementAt(index);
        };
      }

      if (prop === 'slice') {
        return (start?: number, end?: number) => {
          const result = [];
          const s = start ?? 0;
          const e = end ?? length;
          const actualStart = s < 0 ? Math.max(length + s, 0) : Math.min(s, length);
          const actualEnd = e < 0 ? Math.max(length + e, 0) : Math.min(e, length);
          for (let i = actualStart; i < actualEnd; i++) {
            result.push(getElementAt(i));
          }
          return result;
        };
      }

      if (typeof prop === 'symbol') return undefined;
      return undefined;

      function getElementAt(index: number): unknown {
        // Check cache
        if (cache.has(index)) {
          return cache.get(index);
        }

        const type = getArrayType(buffer, offset, index);
        let result: unknown;

        if (type === 'object') {
          const childOffset = getArrayChildOffset(buffer, offset, index);
          result = createObjectProxy({
            buffer,
            offset: childOffset,
            isArray: false,
            cache: new Map(),
          });
        } else if (type === 'array') {
          const childOffset = getArrayChildOffset(buffer, offset, index);
          result = createArrayProxy({
            buffer,
            offset: childOffset,
            isArray: true,
            cache: new Map(),
          });
        } else {
          result = getArrayElement(buffer, offset, index);
        }

        cache.set(index, result);
        return result;
      }
    },

    has(_target, prop: string | symbol): boolean {
      if (typeof prop === 'symbol') {
        return prop === $buffer || prop === $decode || prop === $isLite3Buffer;
      }
      if (prop === 'length') return true;
      const index = Number(prop);
      return !Number.isNaN(index) && Number.isInteger(index) && index >= 0 && index < length;
    },

    ownKeys(): string[] {
      // Return numeric indices as strings
      const keys: string[] = [];
      for (let i = 0; i < length; i++) {
        keys.push(String(i));
      }
      return keys;
    },

    getOwnPropertyDescriptor(_target, prop: string | symbol) {
      if (prop === 'length') {
        return { value: length, writable: false, enumerable: false, configurable: false };
      }
      if (typeof prop === 'symbol') return undefined;
      const index = Number(prop);
      if (Number.isNaN(index) || !Number.isInteger(index) || index < 0 || index >= length) {
        return undefined;
      }
      return {
        enumerable: true,
        configurable: true,
        writable: false,
      };
    },
  });
}

/**
 * Lite3Buffer namespace with factory method
 */
export const Lite3Buffer = {
  /**
   * Create a lazy proxy from a POJO or existing Buffer
   *
   * Returns `unknown` by default for type safety. Provide a type parameter
   * when you trust the data source matches your expected type.
   *
   * @example
   * ```ts
   * // From object - type is inferred
   * const proxy = Lite3Buffer.from({ name: 'Alice', age: 30 });
   *
   * // From buffer - returns unknown by default (honest)
   * const data = Lite3Buffer.from(buffer);
   *
   * // From buffer with trusted type (convenient)
   * const data = Lite3Buffer.from<MyType>(buffer);
   * ```
   */
  from<T = unknown>(data: Lite3Serializable | Buffer): T {
    const buffer = Buffer.isBuffer(data) ? data : encode(data);
    const rootType = getRootType(buffer);

    const state: Lite3ProxyState = {
      buffer,
      offset: 0,
      isArray: rootType === 'array',
      cache: new Map(),
    };

    if (rootType === 'array') {
      return createArrayProxy(state) as T;
    } else {
      return createObjectProxy(state) as T;
    }
  },

  /**
   * Check if a value is a Lite3Buffer proxy
   */
  isLite3Buffer(value: unknown): boolean {
    return (
      value !== null &&
      typeof value === 'object' &&
      (value as Record<symbol, unknown>)[$isLite3Buffer] === true
    );
  },

  /**
   * Get the underlying buffer from a proxy
   */
  getBuffer(proxy: unknown): Buffer | undefined {
    if (!Lite3Buffer.isLite3Buffer(proxy)) return undefined;
    return (proxy as Record<symbol, Buffer>)[$buffer];
  },
};