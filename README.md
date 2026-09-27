# prime-sieve

A fast, low-memory prime counter in portable C99.
It is a segmented, bit-packed Sieve of Eratosthenes with optional multithreading.

```
$ ./prime_sieve 1e9
Primes up to 1000000000: 50847534
Time taken: 0.2090 seconds (2 threads)
Sieve working memory: 132.2 KB
```

## Features

- **Low memory:** RAM use stays almost flat as N grows. The sieve works on one
  32 KB window at a time, plus about 12 bytes per prime below √N.
  That is under 1 MB even for N = 10¹¹.
- **Fast:**
  - The working window stays in the L1 cache.
  - Only odd numbers are stored, one bit each.
  - Multiples of 3, 5, 7, 11 and 13 are stamped 64 bits at a time from a
    precomputed pattern.
  - The crossing-off loop is unrolled.
  - Primes are counted with hardware popcount.
  - Work is spread across all cores with OpenMP.
- **Safe:**
  - Input parsing is strict.
  - All arithmetic is 64-bit unsigned with a hard upper bound, so nothing can
    overflow.
  - Every allocation is checked.
  - It runs clean under AddressSanitizer, UndefinedBehaviorSanitizer and Valgrind.
- **Portable:** plain C99. It builds with GCC, Clang and MSVC on Linux, macOS
  and Windows. Without OpenMP it simply runs single-threaded.

## Benchmarks

Measured on a 2-core Intel Xeon @ 2.1 GHz, compared with a plain
(non-segmented) bit sieve:

| N    | Plain sieve       | This, 1 thread | This, 2 threads | Peak RSS (plain → this) |
|------|-------------------|----------------|-----------------|-------------------------|
| 10⁸  | 0.233 s           | 0.034 s        | 0.015 s         | 7.6 MB → 2.4 MB         |
| 10⁹  | 2.90 s            | 0.33 s         | 0.21 s          | 61 MB → 2.4 MB          |
| 10¹⁰ | needs 625 MB      | —              | 2.0 s           | 2.5 MB                  |
| 10¹¹ | needs 6.25 GB     | —              | 26 s            | 2.5 MB                  |

## Build

**Linux / macOS (GCC or Clang):**

```sh
make              # optimised, multithreaded
make portable     # single-threaded, no CPU-specific instructions
```

To build by hand:

```sh
gcc -O3 -march=native -fopenmp -std=c99 prime_sieve.c -o prime_sieve -lm
```

On macOS with Apple Clang, install `libomp` (`brew install libomp`) for
threading, or use `make portable`.

**Windows (MSVC):**

```
cl /O2 /openmp prime_sieve.c
```

## Usage

```sh
./prime_sieve              # default limit: 1,000,000
./prime_sieve 1000000000
./prime_sieve 1e9          # scientific notation is accepted
OMP_NUM_THREADS=4 ./prime_sieve 1e10
```

Valid limits are 0 to 10¹⁴.
Anything else, such as negative numbers, trailing characters or values that are
too large, is rejected with an error message.

## Tests

```sh
make test       # 6,000 cases checked against a reference sieve
make sanitize   # the same tests under ASan + UBSan
```

The tests cover:

- every N from 0 to 2,500
- both sides of every sieve-window boundary
- thousands of random values

## How it works

Bit *i* of each window stands for the odd number 2*i* + 1.

For each 32 KB window (524,288 integers):

1. Fill the window from the 3·5·7·11·13 pre-sieve pattern.
2. For every prime *p* from 17 to √N, cross off its odd multiples, starting at
   *p*² or at the first multiple inside the window.
3. Count the zero bits with popcount.

The windows are grouped into chunks, and threads take chunks dynamically.
Each thread computes its own starting offsets, so threads never share
mutable state.

## License

MIT. See [LICENSE](LICENSE).
