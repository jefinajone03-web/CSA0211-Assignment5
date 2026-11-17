/*
 * Task B1.4 - Find the limit of linear recursion
 * Student : S Madhu Shri   Reg No: 1921260293   Course: CSA 0211   Slot: D
 *
 * Compile WITHOUT optimisation (the compiler could otherwise turn the
 * recursion into a loop and it would never crash):
 *     gcc -O0 b1_crash_limit.c -o b1_crash_limit
 *
 * Run with an exponent, then keep increasing it until the program crashes:
 *     ./b1_crash_limit 10000
 *     ./b1_crash_limit 100000
 *     ./b1_crash_limit 1000000
 *     ./b1_crash_limit 10000000
 *
 * base = 1 is used so that long long never overflows - only the stack is tested.
 */
#include <stdio.h>
#include <stdlib.h>

long long power_linear(long long base, int exp)
{
    if (exp == 0)
        return 1;
    return base * power_linear(base, exp - 1);
}

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

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <exponent>\n", argv[0]);
        return 1;
    }

    char *end;
    long n = strtol(argv[1], &end, 10);
    if (*end != '\0' || n < 0 || n > 2000000000L) {
        printf("Invalid exponent: %s\n", argv[1]);
        return 1;
    }

    printf("iterative(1, %ld) = %lld\n", n, power_iterative(1, (int)n));
    fflush(stdout);
    printf("linear   (1, %ld) = %lld\n", n, power_linear(1, (int)n));
    return 0;
}
