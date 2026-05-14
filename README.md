# makeprime

Generates a random prime number with a specified number of digits using the Miller-Rabin primality test.
Optionally generates twim primes if you're patient.

## Usage

```sh
Usage: makeprime <digits> [--twin] [[--lead digits] || [--message \"text\"]]
```

--twin  specifies that you want twin primes.

--lead dddddd specifies desired leading digits of the prime. Do not start with a 0.

--message "text" encodes text as octal decimal tripples. Leading and trailing 1s are added as delimeters.

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

