# @jaydeebee/lite3-native-addon

Node.js native addon bindings for [lite3](https://github.com/fastserial/lite3), a zero-copy binary serialization library.

## Disclaimer

This project is:

- **Unofficial** - Built independently of the lite3 project, with no affiliation to the original authors
- **A learning exercise** - Created primarily to explore Node.js native addon development
- **Not production-ready** - Use at your own risk
- **Potentially inefficient** - The JavaScript/N-API bridge may negate some of the performance benefits that lite3 offers in native contexts

## Installation

```bash
npm install @jaydeebee/lite3-native-addon
```

Prebuilt binaries are available for:
- Linux x64
- Linux arm64
- macOS arm64

Other platforms will fall back to source compilation (requires a C compiler and Python).

## Usage

```javascript
import { encode, decode, version } from '@jaydeebee/lite3-native-addon';

// Encode an object to a binary buffer
const buffer = encode({ hello: 'world', count: 42 });

// Decode back to JavaScript
const obj = decode(buffer);

console.log(obj); // { hello: 'world', count: 42 }
console.log(version()); // Addon version
```

## Supported Types

- Strings
- Numbers (stored as f64)
- Booleans
- Null
- Arrays
- Objects

Unsupported types (functions, undefined, symbols) are silently skipped during encoding.

## Dependencies

This addon uses a [forked version](https://github.com/jaydeebee/lite3) of the lite3 library as a git submodule. I hope to resolve this and use the canonical version from [fastserial/lite3](https://github.com/fastserial/lite3) in the future.

## License

MIT

## Contributing

Contributions are welcome.

- Open a pull request for bug fixes and minor improvements
- Use [Angular commit message conventions](https://github.com/angular/angular/blob/main/CONTRIBUTING.md#commit) (e.g., `feat:`, `fix:`, `docs:`)
- For significant or invasive changes, please open an issue first to discuss the approach