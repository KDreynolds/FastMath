# FastMath Library

FastMath is a high-performance mathematics library that provides optimized implementations of common mathematical operations using CPU-specific optimizations (SSE/AVX on x86_64, NEON on ARM).

## Features

- Fast inverse square root implementation using SIMD instructions
- Hardware-accelerated random number generation
- Platform-specific optimizations (x86_64 and ARM support)
- Minimal dependencies
- Thoroughly tested with benchmarks

## Requirements

- C++11 compatible compiler
- x86_64 or ARM processor
- Linux/Unix-like operating system
- Make build system

## Building

To build the library and test executables:

```bash
make            # Build everything
make clean      # Clean build artifacts
```

## Testing

Run the test suite to verify correct operation:

```bash
./test_fastmath
```

Run benchmarks to measure performance:

```bash
./benchmark_fastmath
```

## Usage Example

```cpp
#include "FastMath.h"

// Initialize the subsystems you need
FastMath::Random::initialize();
FastMath::Vector::initialize();

// Create a random number generator
FastMath::Random rng(12345);  // Seed with any integer

// Generate random numbers
float random_value = rng.getFloat();  // Returns [0.0f, 1.0f)

// Use fast inverse square root
float x = 16.0f;
float inv_sqrt_x = FastMath::Vector::inv_sqrt(x);  // Fast 1/sqrt(x)

// Clean up when done
FastMath::Random::shutdown();
FastMath::Vector::shutdown();
```

## Performance

The library provides significant performance improvements over standard library implementations:

- Inverse square root: Up to 3x faster than standard `1.0f/std::sqrt(x)`
- Random number generation: Hardware-accelerated when available (RDRAND)

## Implementation Details

- Uses SIMD instructions for vector operations (SSE/AVX on x86_64, NEON on ARM)
- Implements optimized xoshiro256** algorithm for random number generation
- Hardware random number generation on supported platforms (RDRAND on x86_64)
- Newton-Raphson iteration for improved inverse square root accuracy

## License

MIT License

Copyright (c) 2024

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## Safety Notes

- The library uses hardware-specific optimizations. Always test thoroughly on your target platform.
- The random number generator is not cryptographically secure by default.
- Remember to call initialize() before using any subsystem and shutdown() when done.
