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

### Basic Encode/Decode

```javascript
import { encode, decode, version } from '@jaydeebee/lite3-native-addon';

// Encode an object to a binary buffer
const buffer = encode({ hello: 'world', count: 42 });

// Decode back to JavaScript
const obj = decode(buffer);

console.log(obj); // { hello: 'world', count: 42 }
console.log(version()); // Addon version
```

### Lazy Proxy Access (Lite3Buffer)

For better performance with large objects where you only need a few fields, use `Lite3Buffer.from()` to create a lazy proxy that decodes values on-demand:

```typescript
import { Lite3Buffer } from '@jaydeebee/lite3-native-addon';

// Create from an object (type is inferred)
const proxy = Lite3Buffer.from({
  users: [
    { name: 'Alice', age: 30 },
    { name: 'Bob', age: 25 }
  ],
  metadata: { count: 2 }
});

// Access properties naturally - decoded lazily
console.log(proxy.users[0].name);  // 'Alice' - only this field is decoded
console.log(proxy.metadata.count); // 2

// Array methods work as expected
const names = proxy.users.map(u => u.name);  // ['Alice', 'Bob']
const adults = proxy.users.filter(u => u.age >= 18);

// Works with JSON.stringify, spreading, for...of, etc.
console.log(JSON.stringify(proxy));
const copy = { ...proxy };
for (const user of proxy.users) {
  console.log(user.name);
}
```

#### Type Safety

When creating a proxy from a buffer, the return type defaults to `unknown` for safety (like `JSON.parse`). Provide a type parameter when you trust the data source:

```typescript
interface User {
  name: string;
  age: number;
}

// From buffer - returns unknown by default (safe)
const data = Lite3Buffer.from(buffer);
data.name;  // TS error: 'unknown' has no property 'name'

// With type parameter - returns User (trusted)
const user = Lite3Buffer.from<User>(buffer);
user.name;  // OK - full autocomplete and type checking

// For untrusted sources, consider runtime validation:
import { z } from 'zod';
const UserSchema = z.object({ name: z.string(), age: z.number() });
const validated = UserSchema.parse(Lite3Buffer.from(buffer));
```

#### Utility Functions

```typescript
import { Lite3Buffer, $buffer, $decode } from '@jaydeebee/lite3-native-addon';

// Check if a value is a Lite3Buffer proxy
Lite3Buffer.isLite3Buffer(proxy);  // true
Lite3Buffer.isLite3Buffer({});     // false

// Get the underlying buffer
const buffer = Lite3Buffer.getBuffer(proxy);

// Force full decode (escape hatch)
const pojo = proxy[$decode]();

// Access raw buffer via symbol (alternative)
const rawBuffer = proxy[$buffer];
```

#### Why Use Lite3Buffer?

| Scenario | `decode()` | `Lite3Buffer.from()` |
|----------|-----------|---------------------|
| Access all fields | Good | Similar |
| Access few fields from large object | Wasteful | Efficient |
| Repeated access to same field | Faster | Slightly slower (cached after first access) |
| Pass-through / routing | Decode + re-encode | Keep as buffer |

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