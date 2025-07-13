# makeprime

Generates a random prime number with a specified number of digits using the Miller-Rabin primality test.
Optionally generates twim primes if you're patient. Just pass the --twin flag at the end.

## Usage

```sh
./makeprime <digits> [--twin]
```

## Dependencies

* C++20 compiler
* CMake 3.16+
* GMP library

## License

MIT

---

### 🧪 Build Instructions

```sh
mkdir build && cd build
cmake ..
make
./makeprime 100
```

