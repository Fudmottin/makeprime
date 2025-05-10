# makeprime

Generates a random prime number with a specified number of digits using the Miller-Rabin primality test.

## Usage

```sh
./makeprime <digits>
```

## Dependencies

* C++20 compiler
* CMake 3.16+
* Boost libraries (multiprecision, random)

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

