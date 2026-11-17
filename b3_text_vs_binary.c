/*
 * Task B3 - Text file vs binary file
 * Student : S Madhu Shri   Reg No: 1921260293   Course: CSA 0211   Slot: D
 *
 * Compile : gcc b3_text_vs_binary.c -o b3_text_vs_binary -Wall -Wextra -O2
 * Run     : ./b3_text_vs_binary
 *
 * B3.1  10 donors written to text + binary, read back and compared
 * B3.2  1,000 / 10,000 / 100,000 donors: file sizes + write/read times
 * B3.3  update_units(): in-place update of one stock record with fseek()
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    int  id;
    char name[30];
    char group[4];
    int  age;
} Donor;

typedef struct {
    int group_code;
    int units;
} Stock;

static const char *GROUPS[] = {"A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-"};

/* Fill d[i] with predictable data so we can compare after reading back */
static void make_donor(Donor *d, int i)
{
    d->id = 1000 + i;
    snprintf(d->name, sizeof d->name, "Donor_%d", i);
    strcpy(d->group, GROUPS[i % 8]);
    d->age = 18 + (i % 48);                   /* 18 .. 65 */
}

static int same_donor(const Donor *a, const Donor *b)
{
    return a->id == b->id && a->age == b->age &&
           strcmp(a->name, b->name) == 0 && strcmp(a->group, b->group) == 0;
}

static double seconds_since(clock_t start)
{
    return (double)(clock() - start) / CLOCKS_PER_SEC;
}

static long file_size(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (fp == NULL)
        return -1;
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fclose(fp);
    return size;
}

/* ---------------- text file helpers ---------------- */

static int write_text(const char *path, const Donor *d, int n)
{
    FILE *fp = fopen(path, "w");
    if (fp == NULL) { perror(path); return 0; }
    for (int i = 0; i < n; i++)
        fprintf(fp, "%d %s %s %d\n", d[i].id, d[i].name, d[i].group, d[i].age);
    fclose(fp);
    return 1;
}

static int read_text(const char *path, Donor *d, int n)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL) { perror(path); return 0; }
    for (int i = 0; i < n; i++) {
        if (fscanf(fp, "%d %29s %3s %d",
                   &d[i].id, d[i].name, d[i].group, &d[i].age) != 4) {
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    return 1;
}

/* ---------------- binary file helpers ---------------- */

static int write_binary(const char *path, const Donor *d, int n)
{
    FILE *fp = fopen(path, "wb");
    if (fp == NULL) { perror(path); return 0; }
    if (fwrite(d, sizeof(Donor), (size_t)n, fp) != (size_t)n) {
        perror("fwrite");
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

static int read_binary(const char *path, Donor *d, int n)
{
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) { perror(path); return 0; }
    size_t got = fread(d, sizeof(Donor), (size_t)n, fp);
    fclose(fp);
    return got == (size_t)n;
}

/* ---------------- B3.1 ---------------- */

static int task_b31(void)
{
    Donor orig[10], t[10], b[10];
    int ok_text = 1, ok_bin = 1;

    for (int i = 0; i < 10; i++)
        make_donor(&orig[i], i);

    if (!write_text("donors.txt", orig, 10) || !write_binary("donors.dat", orig, 10))
        return 0;
    if (!read_text("donors.txt", t, 10) || !read_binary("donors.dat", b, 10))
        return 0;

    for (int i = 0; i < 10; i++) {
        if (!same_donor(&orig[i], &t[i])) ok_text = 0;
        if (!same_donor(&orig[i], &b[i])) ok_bin = 0;
    }
    printf("=== B3.1 Correctness on 10 records ===\n");
    printf("Text   file : %s\n", ok_text ? "all 10 records match" : "MISMATCH");
    printf("Binary file : %s\n\n", ok_bin ? "all 10 records match" : "MISMATCH");
    return ok_text && ok_bin;
}

/* ---------------- B3.2 ---------------- */

static int task_b32(void)
{
    int sizes[] = {1000, 10000, 100000};

    printf("=== B3.2 Increasing data size (sizeof(Donor) = %zu bytes) ===\n",
           sizeof(Donor));
    printf("%8s | %10s %10s | %9s %9s | %9s %9s\n",
           "donors", "text size", "bin size",
           "txt write", "bin write", "txt read", "bin read");
    printf("---------+-----------------------+---------------------+--------------------\n");

    for (int s = 0; s < 3; s++) {
        int n = sizes[s];
        Donor *d = malloc((size_t)n * sizeof(Donor));
        Donor *r = malloc((size_t)n * sizeof(Donor));
        if (d == NULL || r == NULL) {
            printf("Out of memory\n");
            free(d); free(r);
            return 0;
        }
        for (int i = 0; i < n; i++)
            make_donor(&d[i], i);

        clock_t t0;
        double tw, bw, tr, br;

        t0 = clock(); write_text("big.txt", d, n);   tw = seconds_since(t0);
        t0 = clock(); write_binary("big.dat", d, n); bw = seconds_since(t0);
        t0 = clock(); int okt = read_text("big.txt", r, n);   tr = seconds_since(t0);
        t0 = clock(); int okb = read_binary("big.dat", r, n); br = seconds_since(t0);

        printf("%8d | %10ld %10ld | %8.4fs %8.4fs | %8.4fs %8.4fs%s\n",
               n, file_size("big.txt"), file_size("big.dat"),
               tw, bw, tr, br, (okt && okb) ? "" : "  (READ ERROR)");

        free(d);
        free(r);
    }
    remove("big.txt");
    remove("big.dat");
    printf("\n");
    return 1;
}

/* ---------------- B3.3 ---------------- */

/* Overwrite the units of record `index` without touching the other records. */
int update_units(FILE *fp, int index, int units)
{
    Stock s;

    if (fp == NULL || index < 0)
        return 0;

    long position = (long)index * (long)sizeof(Stock);

    if (fseek(fp, position, SEEK_SET) != 0)
        return 0;
    if (fread(&s, sizeof(Stock), 1, fp) != 1)      /* record does not exist */
        return 0;

    s.units = units;

    if (fseek(fp, position, SEEK_SET) != 0)         /* required between read & write */
        return 0;
    if (fwrite(&s, sizeof(Stock), 1, fp) != 1)
        return 0;
    fflush(fp);
    return 1;
}

static int task_b33(void)
{
    Stock stock[5] = {{1, 40}, {2, 25}, {3, 12}, {4, 8}, {5, 30}};

    FILE *fp = fopen("stock.dat", "wb");
    if (fp == NULL) { perror("stock.dat"); return 0; }
    fwrite(stock, sizeof(Stock), 5, fp);            /* one call for all records */
    fclose(fp);

    fp = fopen("stock.dat", "r+b");                 /* read + write, binary */
    if (fp == NULL) { perror("stock.dat"); return 0; }

    printf("=== B3.3 Update one record in place ===\n");
    printf("update record 3: %s\n", update_units(fp, 3, 20) ? "done" : "failed");
    printf("update record 9: %s\n", update_units(fp, 9, 99) ? "done" : "failed");

    rewind(fp);
    Stock s;
    while (fread(&s, sizeof(Stock), 1, fp) == 1)
        printf("%d:%d ", s.group_code, s.units);
    printf("\n");
    fclose(fp);
    remove("stock.dat");
    return 1;
}

int main(void)
{
    if (!task_b31()) return 1;
    if (!task_b32()) return 1;
    if (!task_b33()) return 1;
    remove("donors.txt");
    remove("donors.dat");
    return 0;
}
