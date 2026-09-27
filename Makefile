CC      ?= gcc
CFLAGS  ?= -O3 -march=native -std=c99 -Wall -Wextra -Wpedantic
OMP     ?= -fopenmp
LDLIBS  := -lm

.PHONY: all portable test sanitize clean

all: prime_sieve

prime_sieve: prime_sieve.c
	$(CC) $(CFLAGS) $(OMP) $< -o $@ $(LDLIBS)

# Single-threaded build without CPU-specific instructions
portable: prime_sieve.c
	$(CC) -O2 -std=c99 -Wall -Wextra $< -o prime_sieve $(LDLIBS)

test: tests/test_sieve.c prime_sieve.c
	$(CC) -O2 -std=c99 $(OMP) tests/test_sieve.c -o tests/test_sieve $(LDLIBS)
	./tests/test_sieve

# Run the test suite under AddressSanitizer + UndefinedBehaviorSanitizer
sanitize: tests/test_sieve.c prime_sieve.c
	$(CC) -O1 -g -std=c99 $(OMP) -fsanitize=address,undefined -fno-sanitize-recover=all \
	    -DREF_MAX=3000000u tests/test_sieve.c -o tests/test_sieve_san $(LDLIBS)
	./tests/test_sieve_san

clean:
	rm -f prime_sieve prime_sieve.exe tests/test_sieve tests/test_sieve_san
