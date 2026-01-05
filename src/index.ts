import bindings from 'bindings';

const addon = bindings('lite3.node');

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

/**
 * lite3 native addon interface.
 */
export interface Lite3Addon {
  /** Returns the addon version string. */
  version(): string;

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
}

export const { version, encode, decode } = addon as Lite3Addon;
export default addon as Lite3Addon;