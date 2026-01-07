import bindings from 'bindings';
import { createRequire } from 'module';

const addon = bindings('lite3.node');

// Read version from package.json at runtime
const require = createRequire(import.meta.url);
const pkg = require('../package.json');

/** Returns the package version (set by semantic-release during CI) */
export function version(): string {
  return pkg.version;
}

/**
 * Constraint for values that can be serialized by lite3.
 * Excludes functions, symbols, undefined, and other non-serializable types.
 */
export type Lite3Serializable =
  | string
  | number
  | boolean
  | null
  | Lite3Serializable[]
  | { [key: string]: Lite3Serializable };

/** Type strings returned by getType/getArrayType/getRootType */
export type Lite3TypeString =
  | 'object'
  | 'array'
  | 'string'
  | 'number'
  | 'boolean'
  | 'null'
  | 'bytes'
  | 'undefined';

/**
 * lite3 native addon interface.
 */
export interface Lite3Addon {
  /** Returns the lite3 library version string (e.g., "1.0.0-81ed77a") */
  lite3Version(): string;

  /**
   * Encodes a JavaScript object or array into a lite3 binary buffer.
   * @param data - The object or array to encode
   * @returns A Buffer containing the lite3 binary representation
   */
  encode<T extends Lite3Serializable>(data: T): Buffer;

  /**
   * Decodes a lite3 binary buffer back into a JavaScript value.
   * @param buffer - The Buffer to decode
   * @returns The decoded JavaScript object or array
   */
  decode<T = unknown>(buffer: Buffer): T;

  // Proxy support functions for lazy access:

  /** Returns the type of a property at the given offset and key */
  getType(buffer: Buffer, offset: number, key: string): Lite3TypeString;

  /** Returns the type of an array element at the given offset and index */
  getArrayType(buffer: Buffer, offset: number, index: number): Lite3TypeString;

  /** Returns the value of a property (primitives) or child offset (objects/arrays) */
  getValue(buffer: Buffer, offset: number, key: string): unknown;

  /** Returns the value of an array element or child offset for nested structures */
  getArrayElement(buffer: Buffer, offset: number, index: number): unknown;

  /** Returns the offset of a nested object or array */
  getChildOffset(buffer: Buffer, offset: number, key: string): number;

  /** Returns the offset of a nested object or array within an array */
  getArrayChildOffset(buffer: Buffer, offset: number, index: number): number;

  /** Returns an array of keys for the object at the given offset */
  getKeys(buffer: Buffer, offset: number): string[];

  /** Returns the length of an array or object at the given offset */
  getLength(buffer: Buffer, offset: number): number;

  /** Returns true if the object has the given key */
  hasKey(buffer: Buffer, offset: number, key: string): boolean;

  /** Returns the type of the root element */
  getRootType(buffer: Buffer): Lite3TypeString;
}

export const {
  lite3Version,
  encode,
  decode,
  getType,
  getArrayType,
  getValue,
  getArrayElement,
  getChildOffset,
  getArrayChildOffset,
  getKeys,
  getLength,
  hasKey,
  getRootType,
} = addon as Lite3Addon;

export default addon as Lite3Addon;

// Re-export proxy API
export { Lite3Buffer, $buffer, $decode, $isLite3Buffer } from './proxy';