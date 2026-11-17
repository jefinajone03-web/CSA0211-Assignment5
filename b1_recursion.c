/*
 * Task B1 - Implement and instrument the recursive functions
 * Student : S Madhu Shri   Reg No: 1921260293   Course: CSA 0211   Slot: D
 *
 * Compile : gcc b1_recursion.c -o b1_recursion -Wall -Wextra
 * Run     : ./b1_recursion
 */
#include <stdio.h>

long long calls = 0;            /* counts every recursive call */

/* Linear recursion: T(n) = T(n-1) + 1  -> n + 1 calls */
long long power_linear(long long base, int exp)
{
    calls++;
    if (exp == 0)
        return 1;
    return base * power_linear(base, exp - 1);
}

/* Halving recursion: T(n) = T(n/2) + 1 -> about log2(n) + 2 calls */
long long power_fast(long long base, int exp)
{
    calls++;
    if (exp == 0)
        return 1;
    long long half = power_fast(base, exp / 2);   /* computed ONCE */
    if (exp % 2 == 0)
        return half * half;
    return base * half * half;
}

/* Experiment: the same subproblem is computed twice -> loses the speed-up */
long long power_fast_twice(long long base, int exp)
{
    calls++;
    if (exp == 0)
        return 1;
    long long r = power_fast_twice(base, exp / 2) *
                  power_fast_twice(base, exp / 2);
    if (exp % 2 == 1)
        r *= base;              /* extra multiplication for odd exponent */
    return r;
}

/* Iterative square-and-multiply: O(log n) time, O(1) extra space */
long long power_iterative(long long base, int exp)
{
    long long result = 1;
    while (exp > 0) {
        if (exp % 2 == 1)
            result *= base;
        base *= base;
        exp /= 2;
    }
    return result;
}

/* Digital root (recursive) */
int digitalRoot(int n)
{
    if (n < 10)
        return n;
    int sum = 0;
    while (n > 0) {
        sum += n % 10;
        n /= 10;
    }
    return digitalRoot(sum);
}

static int check_int(const char *name, long long got, long long expected)
{
    int ok = (got == expected);
    printf("  %-22s got %-6lld expected %-6lld %s\n",
           name, got, expected, ok ? "PASS" : "FAIL");
    return ok;
}

int main(void)
{
    int pass = 0, total = 0;

    printf("=== B1.1 Test cases ===\n");
    total++; pass += check_int("digitalRoot(9875)", digitalRoot(9875), 2);
    total++; pass += check_int("digitalRoot(123)",  digitalRoot(123),  6);
    total++; pass += check_int("digitalRoot(7)",    digitalRoot(7),    7);
    total++; pass += check_int("power_fast(2,0)",   power_fast(2, 0),  1);
    total++; pass += check_int("power_fast(2,1)",   power_fast(2, 1),  2);
    total++; pass += check_int("power_fast(2,5)",   power_fast(2, 5),  32);
    total++; pass += check_int("power_linear(2,5)", power_linear(2, 5), 32);
    total++; pass += check_int("power_iterative(2,5)", power_iterative(2, 5), 32);
    printf("  %d / %d tests passed\n\n", pass, total);

    printf("=== B1.3 Call counts (base 1 so nothing overflows) ===\n");
    printf("%10s %12s %12s %16s\n", "n", "linear", "halving", "halving (twice)");
    int sizes[] = {10, 100, 1000, 10000};
    for (int i = 0; i < 4; i++) {
        int n = sizes[i];
        long long c1, c2, c3;

        calls = 0; power_linear(1, n);      c1 = calls;
        calls = 0; power_fast(1, n);        c2 = calls;
        calls = 0; power_fast_twice(1, n);  c3 = calls;   /* ~ n calls: no speed-up */

        printf("%10d %12lld %12lld %16lld\n", n, c1, c2, c3);
    }

    printf("\n=== Main program from the assignment ===\n");
    calls = 0;
    long long a = power_linear(2, 30);
    printf("linear: %lld in %lld calls\n", a, calls);

    calls = 0;
    long long b = power_fast(2, 30);
    printf("fast  : %lld in %lld calls\n", b, calls);

    return 0;
}
