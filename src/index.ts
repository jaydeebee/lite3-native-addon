// eslint-disable-next-line @typescript-eslint/no-require-imports
const addon = require('../build/Release/lite3.node');

export interface Lite3Addon {
  version(): string;
}

export default addon as Lite3Addon;