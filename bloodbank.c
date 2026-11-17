/*
 * District Blood Bank Network - command-line program
 * Student : S Madhu Shri   Reg No: 1921260293   Course: CSA 0211   Slot: D
 *
 * Donor details  -> text file   (donors.txt, one donor per line)
 * Blood stock    -> binary file (stock.dat, 8 fixed-size records)
 *
 * Compile : gcc bloodbank.c -o bloodbank -Wall -Wextra -fsanitize=address -g
 *
 * Commands:
 *   ./bloodbank register <id>        add a new donor (asks for the details)
 *   ./bloodbank find <group>         list donors of a blood group
 *   ./bloodbank issue <group> <n>    take n units out of stock
 *   ./bloodbank receive <group> <n>  add n units to stock
 *   ./bloodbank report               show all donors and the stock
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DONOR_FILE  "donors.txt"
#define STOCK_FILE  "stock.dat"
#define NUM_GROUPS  8
#define MIN_AGE     18
#define MAX_AGE     65

typedef struct {
    int  id;
    char name[30];
    char group[4];
    int  age;
    char phone[16];
} Donor;

typedef struct {
    int group_code;                /* index into GROUPS[] */
    int units;
} Stock;

typedef struct {
    Donor *data;
    int    size;
    int    capacity;
} DonorList;

static const char *GROUPS[NUM_GROUPS] = {"A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-"};

/* ------------------------------------------------------------------ */
/* helpers                                                             */
/* ------------------------------------------------------------------ */

static int group_index(const char *g)
{
    for (int i = 0; i < NUM_GROUPS; i++)
        if (strcmp(g, GROUPS[i]) == 0)
            return i;
    return -1;
}

static int parse_positive(const char *s, long *out)
{
    char *end;
    if (*s == '\0')
        return 0;
    long v = strtol(s, &end, 10);
    if (*end != '\0' || v <= 0 || v > 1000000)
        return 0;
    *out = v;
    return 1;
}

/* Reads a line into buf (max size-1 chars), drains the rest, strips '\n'. */
static int read_line(const char *prompt, char *buf, size_t size)
{
    printf("%s", prompt);
    if (fgets(buf, (int)size, stdin) == NULL)
        return 0;
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
    }
    return 1;
}

static void usage(void)
{
    printf("Usage:\n");
    printf("  bloodbank register <id>\n");
    printf("  bloodbank find <group>\n");
    printf("  bloodbank issue <group> <units>\n");
    printf("  bloodbank receive <group> <units>\n");
    printf("  bloodbank report\n");
    printf("Valid groups: A+ A- B+ B- AB+ AB- O+ O-\n");
}

/* ------------------------------------------------------------------ */
/* growable donor list                                                 */
/* ------------------------------------------------------------------ */

static int add_donor(DonorList *list, const Donor *d)
{
    if (list->size == list->capacity) {
        int newCap = (list->capacity == 0) ? 2 : list->capacity * 2;
        Donor *p = realloc(list->data, (size_t)newCap * sizeof(Donor));
        if (p == NULL)
            return 0;
        list->data = p;
        list->capacity = newCap;
    }
    list->data[list->size++] = *d;
    return 1;
}

static void free_list(DonorList *list)
{
    free(list->data);
    list->data = NULL;
    list->size = list->capacity = 0;
}

/* ------------------------------------------------------------------ */
/* donor file (text)                                                   */
/* ------------------------------------------------------------------ */

static int checkEligibility(const Donor *d)
{
    return d->age >= MIN_AGE && d->age <= MAX_AGE;
}

/* Loads donors.txt. A missing file is not an error (no donors yet). */
static int loadDonors(DonorList *list)
{
    FILE *fp = fopen(DONOR_FILE, "r");
    if (fp == NULL)
        return 1;

    char line[256];
    while (fgets(line, sizeof line, fp) != NULL) {
        Donor d;
        if (sscanf(line, "%d|%29[^|]|%3[^|]|%d|%15[^\n]",
                   &d.id, d.name, d.group, &d.age, d.phone) != 5)
            continue;                                   /* skip damaged lines */
        if (!add_donor(list, &d)) {
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    return 1;
}

static int saveDonor(const Donor *d)                    /* append one donor */
{
    FILE *fp = fopen(DONOR_FILE, "a");
    if (fp == NULL) {
        perror("Cannot open " DONOR_FILE " for writing");
        return 0;
    }
    fprintf(fp, "%d|%s|%s|%d|%s\n", d->id, d->name, d->group, d->age, d->phone);
    fclose(fp);
    return 1;
}

/* ------------------------------------------------------------------ */
/* stock file (binary)                                                 */
/* ------------------------------------------------------------------ */

static int saveStock(const Stock s[], int n)
{
    FILE *fp = fopen(STOCK_FILE, "wb");
    if (fp == NULL) {
        perror("Cannot open " STOCK_FILE " for writing");
        return 0;
    }
    if (fwrite(s, sizeof(Stock), (size_t)n, fp) != (size_t)n) {
        perror("Write error");
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

/* Loads stock; creates the file with 0 units if it does not exist yet. */
static int loadStock(Stock s[])
{
    FILE *fp = fopen(STOCK_FILE, "rb");
    if (fp == NULL) {
        for (int i = 0; i < NUM_GROUPS; i++) {
            s[i].group_code = i;
            s[i].units = 0;
        }
        return saveStock(s, NUM_GROUPS);
    }
    size_t got = fread(s, sizeof(Stock), NUM_GROUPS, fp);
    fclose(fp);
    if (got != NUM_GROUPS) {
        printf("Error: " STOCK_FILE " is damaged.\n");
        return 0;
    }
    return 1;
}

/* Changes the units of ONE record in place using fseek(). */
static int update_units(int index, int units)
{
    FILE *fp = fopen(STOCK_FILE, "r+b");
    if (fp == NULL) {
        perror("Cannot open " STOCK_FILE);
        return 0;
    }
    Stock s;
    long pos = (long)index * (long)sizeof(Stock);
    int ok = fseek(fp, pos, SEEK_SET) == 0 &&
             fread(&s, sizeof s, 1, fp) == 1;
    if (ok) {
        s.units = units;
        ok = fseek(fp, pos, SEEK_SET) == 0 &&
             fwrite(&s, sizeof s, 1, fp) == 1;
    }
    fclose(fp);
    return ok;
}

/* ------------------------------------------------------------------ */
/* commands                                                            */
/* ------------------------------------------------------------------ */

static int cmd_register(const char *idText)
{
    long id;
    if (!parse_positive(idText, &id)) {
        printf("Error: '%s' is not a valid donor ID.\n", idText);
        return 1;
    }

    DonorList list = {NULL, 0, 0};
    if (!loadDonors(&list)) {
        printf("Error: out of memory while loading donors.\n");
        free_list(&list);
        return 1;
    }
    for (int i = 0; i < list.size; i++) {
        if (list.data[i].id == id) {
            printf("Error: donor ID %ld already exists.\n", id);
            free_list(&list);
            return 1;
        }
    }
    free_list(&list);

    Donor d;
    char buf[32];
    d.id = (int)id;

    if (!read_line("Name  : ", d.name, sizeof d.name) || d.name[0] == '\0') {
        printf("Error: name is required.\n");
        return 1;
    }
    if (strchr(d.name, '|') != NULL) {
        printf("Error: name must not contain '|'.\n");
        return 1;
    }
    if (!read_line("Blood group: ", d.group, sizeof d.group) || group_index(d.group) < 0) {
        printf("Error: invalid blood group.\n");
        return 1;
    }
    long age;
    if (!read_line("Age   : ", buf, sizeof buf) || !parse_positive(buf, &age)) {
        printf("Error: age must be a positive number.\n");
        return 1;
    }
    d.age = (int)age;
    if (!read_line("Phone : ", d.phone, sizeof d.phone) || d.phone[0] == '\0' ||
        strchr(d.phone, '|') != NULL) {
        printf("Error: phone is required.\n");
        return 1;
    }

    if (!checkEligibility(&d)) {
        printf("Donor is not eligible (age must be %d-%d). Not registered.\n",
               MIN_AGE, MAX_AGE);
        return 1;
    }
    if (!saveDonor(&d))
        return 1;
    printf("Donor %d (%s, %s) registered.\n", d.id, d.name, d.group);
    return 0;
}

static int cmd_find(const char *group)
{
    DonorList list = {NULL, 0, 0};
    if (!loadDonors(&list)) {
        printf("Error: out of memory while loading donors.\n");
        free_list(&list);
        return 1;
    }
    int found = 0;
    for (int i = 0; i < list.size; i++) {
        if (strcmp(list.data[i].group, group) == 0) {
            printf("%d  %-20s %-4s age %d  %s%s\n",
                   list.data[i].id, list.data[i].name, list.data[i].group,
                   list.data[i].age, list.data[i].phone,
                   checkEligibility(&list.data[i]) ? "" : "  (not eligible)");
            found++;
        }
    }
    printf("%d donor(s) found with group %s.\n", found, group);
    free_list(&list);
    return 0;
}

static int cmd_stock_change(const char *group, long units, int issue)
{
    Stock stock[NUM_GROUPS];
    int idx = group_index(group);

    if (!loadStock(stock))
        return 1;

    if (issue && stock[idx].units < units) {
        printf("Error: only %d unit(s) of %s in stock, cannot issue %ld.\n",
               stock[idx].units, group, units);
        return 1;
    }
    int newUnits = stock[idx].units + (issue ? -(int)units : (int)units);
    if (!update_units(idx, newUnits))
        return 1;
    printf("%s %ld unit(s) of %s. Stock is now %d.\n",
           issue ? "Issued" : "Received", units, group, newUnits);
    return 0;
}

static int cmd_report(void)
{
    DonorList list = {NULL, 0, 0};
    Stock stock[NUM_GROUPS];

    if (!loadDonors(&list)) {
        printf("Error: out of memory while loading donors.\n");
        free_list(&list);
        return 1;
    }
    printf("--- Donors (%d) ---\n", list.size);
    for (int i = 0; i < list.size; i++)
        printf("%d  %-20s %-4s age %d  %s\n", list.data[i].id, list.data[i].name,
               list.data[i].group, list.data[i].age, list.data[i].phone);
    free_list(&list);

    if (!loadStock(stock))
        return 1;
    printf("--- Blood stock ---\n");
    for (int i = 0; i < NUM_GROUPS; i++)
        printf("%-4s : %d unit(s)\n", GROUPS[stock[i].group_code], stock[i].units);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Error: no command given.\n");
        usage();
        return 1;
    }

    if (strcmp(argv[1], "register") == 0) {
        if (argc != 3) { printf("Error: 'register' needs a donor ID.\n"); usage(); return 1; }
        return cmd_register(argv[2]);
    }
    if (strcmp(argv[1], "find") == 0) {
        if (argc != 3) { printf("Error: 'find' needs a blood group.\n"); usage(); return 1; }
        if (group_index(argv[2]) < 0) {
            printf("Error: '%s' is not a valid blood group.\n", argv[2]);
            usage();
            return 1;
        }
        return cmd_find(argv[2]);
    }
    if (strcmp(argv[1], "issue") == 0 || strcmp(argv[1], "receive") == 0) {
        int issue = (strcmp(argv[1], "issue") == 0);
        long units;
        if (argc != 4) {
            printf("Error: '%s' needs a blood group and a quantity.\n", argv[1]);
            usage();
            return 1;
        }
        if (group_index(argv[2]) < 0) {
            printf("Error: '%s' is not a valid blood group.\n", argv[2]);
            usage();
            return 1;
        }
        if (!parse_positive(argv[3], &units)) {
            printf("Error: '%s' is not a positive whole number.\n", argv[3]);
            usage();
            return 1;
        }
        return cmd_stock_change(argv[2], units, issue);
    }
    if (strcmp(argv[1], "report") == 0) {
        if (argc != 2) { usage(); return 1; }
        return cmd_report();
    }

    printf("Error: unknown command '%s'.\n", argv[1]);
    usage();
    return 1;
}
