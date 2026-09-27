/*
 * Correctness tests for prime_sieve.c
 * Compares count_primes(n) against a simple reference sieve for:
 *   - every n from 0 to 2500
 *   - n on both sides of every sieve-window boundary
 *   - thousands of random n up to REF_MAX
 * Build & run:  make test
 */
#define main prime_sieve_main
#include "../prime_sieve.c"
#undef main

#ifndef REF_MAX
#define REF_MAX 50000000u
#endif
#define NQ 6000

static int cmp_u64(const void *a, const void *b)
{
    uint64_t x = *(const uint64_t *)a, y = *(const uint64_t *)b;
    return (x > y) - (x < y);
}

int main(void)
{
    static uint64_t q[NQ];
    unsigned char *comp = (unsigned char *)calloc((size_t)REF_MAX + 1u, 1);
    uint64_t i, j, n = 0, pi = 0, rng = 88172645463325252u;
    int nq = 0, bad = 0, t;

    if (!comp) { fputs("reference allocation failed\n", stderr); return 1; }
    comp[0] = comp[1] = 1;
    for (i = 2; i * i <= REF_MAX; i++)
        if (!comp[i])
            for (j = i * i; j <= REF_MAX; j += i) comp[j] = 1;

    for (i = 0; i <= 2500; i++) q[nq++] = i;
    for (i = 1; nq < NQ - 8; i++) {
        uint64_t b = 2u * i * SEG_BITS;
        int d;
        if (b + 3u > REF_MAX) break;
        for (d = -3; d <= 3; d++) q[nq++] = b + (uint64_t)(int64_t)d;
    }
    while (nq < NQ) { /* xorshift64 */
        rng ^= rng << 13; rng ^= rng >> 7; rng ^= rng << 17;
        q[nq++] = rng % (REF_MAX + 1u);
    }
    qsort(q, NQ, sizeof q[0], cmp_u64);

    for (t = 0; t < NQ; t++) {
        uint64_t got; size_t mem; int th;
        while (n < q[t]) { n++; if (!comp[n]) pi++; }
        if (count_primes(q[t], &got, &mem, &th) != 0) {
            fputs("count_primes: allocation failed\n", stderr); return 1;
        }
        if (got != pi && bad++ < 10)
            printf("FAIL n=%llu got=%llu want=%llu\n", (unsigned long long)q[t],
                   (unsigned long long)got, (unsigned long long)pi);
    }
    free(comp);
    printf("%d cases, %d failures\n", NQ, bad);
    return bad ? 1 : 0;
}
