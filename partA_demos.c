/*
 * Part A - Working versions of the design code (A1.3, A2, A3, A5.2)
 * Student : S Madhu Shri   Reg No: 1921260293   Course: CSA 0211   Slot: D
 *
 * Compile : gcc partA_demos.c -o partA_demos -Wall -Wextra
 * Run     : ./partA_demos
 */
#include <stdio.h>

struct Donor {
    int id;
    int age;
};

/* ---------- A1.3  returning more than one value ---------- */

/* Alternative 1: pointer (output) parameters */
void eligibleCount(const struct Donor d[], int n, int *count, int *youngestID)
{
    int youngestAge = 1000;
    *count = 0;
    *youngestID = -1;

    for (int i = 0; i < n; i++) {
        if (d[i].age >= 18 && d[i].age <= 65) {
            (*count)++;
            if (d[i].age < youngestAge) {
                youngestAge = d[i].age;
                *youngestID = d[i].id;
            }
        }
    }
}

/* Alternative 2: return a structure */
struct Result {
    int count;
    int youngestID;
};

struct Result eligibleCountStruct(const struct Donor d[], int n)
{
    struct Result r = {0, -1};
    int youngestAge = 1000;

    for (int i = 0; i < n; i++) {
        if (d[i].age >= 18 && d[i].age <= 65) {
            r.count++;
            if (d[i].age < youngestAge) {
                youngestAge = d[i].age;
                r.youngestID = d[i].id;
            }
        }
    }
    return r;
}

/* ---------- A2  recursion ---------- */

int digitalRoot(int n)                     /* recursive digit sum */
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

int digitalRootFormula(int n)              /* O(1): only for n > 0 */
{
    return 1 + (n - 1) % 9;
}

long long powerFast(long long base, int n)
{
    if (n == 0)
        return 1;
    long long half = powerFast(base, n / 2);
    return (n % 2 == 0) ? half * half : base * half * half;
}

/* ---------- A3  pointers ---------- */

int *lowestStock(int stock[], int n)       /* returns pointer INTO the array - safe */
{
    int *min = &stock[0];
    for (int i = 1; i < n; i++)
        if (stock[i] < *min)
            min = &stock[i];
    return min;
}

int lowestStockIndex(int stock[], int n)   /* alternative: return the index */
{
    int min = 0;
    for (int i = 1; i < n; i++)
        if (stock[i] < stock[min])
            min = i;
    return min;
}

int main(void)
{
    /* A1.3 */
    struct Donor donors[] = {{101, 25}, {102, 17}, {103, 66}, {104, 19}, {105, 40}};
    int count, youngest;
    eligibleCount(donors, 5, &count, &youngest);
    printf("A1.3 pointers : eligible = %d, youngest eligible ID = %d\n", count, youngest);
    struct Result r = eligibleCountStruct(donors, 5);
    printf("A1.3 struct   : eligible = %d, youngest eligible ID = %d\n\n", r.count, r.youngestID);

    /* A2 */
    printf("A2 digitalRoot(9875) = %d (formula: %d)\n", digitalRoot(9875), digitalRootFormula(9875));
    printf("A2 powerFast(2, 8)   = %lld\n\n", powerFast(2, 8));

    /* A3.1 pointer expressions */
    int stock[8] = {40, 25, 12, 8, 30, 15, 20, 10};
    int *p = stock;
    printf("A3.1 p + 3 = %p (address of stock[3] = %p)\n", (void *)(p + 3), (void *)&stock[3]);
    printf("A3.1 *(p+3) = %d, p[3] = %d\n", *(p + 3), p[3]);
    int old = *p++;                        /* use current value, then move forward */
    printf("A3.1 *p++ gave %d, now p points to stock[%ld] = %d\n\n", old, (long)(p - stock), *p);

    /* A3.2 */
    int *low = lowestStock(stock, 8);
    printf("A3.2 Lowest stock = %d\n", *low);
    *low = 5;
    printf("A3.2 Updated stock = %d\n", *low);

    int idx = lowestStockIndex(stock, 8);
    printf("A3.2 index version: lowest is stock[%d] = %d\n", idx, stock[idx]);
    return 0;
}
