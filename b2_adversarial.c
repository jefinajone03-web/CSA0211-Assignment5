/*
 * Task B2 - Engineer adversarial inputs
 * Student : S Madhu Shri   Reg No: 1921260293   Course: CSA 0211   Slot: D
 *
 * Compile : gcc b2_adversarial.c -o b2_adversarial -Wall -Wextra -fsanitize=address -g
 *
 * Run one test at a time:
 *   ./b2_adversarial name                   (long donor name, safe input)
 *   ./b2_adversarial file                   (missing / read-only file)
 *   ./b2_adversarial file donors.txt        (try another file name)
 *   ./b2_adversarial cli find O+            (valid)
 *   ./b2_adversarial cli                    (invalid: nothing given)
 *   ./b2_adversarial cli find               (invalid: group missing)
 *   ./b2_adversarial cli find XYZ           (invalid: bad blood group)
 *   ./b2_adversarial cli issue A- two       (invalid: quantity not a number)
 *   ./b2_adversarial getline                (dynamic-buffer alternative)
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ---------- 1. Long donor name ------------------------------------ */

/* Reads one line safely. Extra characters are discarded so they do not
   stay in the input stream and break the next prompt. */
static int read_line(char *buf, size_t size)
{
    if (fgets(buf, (int)size, stdin) == NULL)
        return 0;

    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';                 /* complete line was read */
    } else {
        int ch;                              /* line was too long: drain it */
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
        printf("(input was longer than %zu characters and was truncated)\n",
               size - 1);
    }
    return 1;
}

static void test_name(void)
{
    char name[20];
    printf("Enter donor name (max 19 characters): ");
    if (!read_line(name, sizeof name)) {
        printf("No input received.\n");
        return;
    }
    printf("Donor name stored: \"%s\"\n", name);
}

/* ---------- 2. Missing or read-only file --------------------------- */

static void test_file(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Unable to open file for reading");
        return;
    }
    printf("File '%s' opened successfully for reading\n", filename);
    fclose(fp);

    /* A read-only file opens fine for "r" but fails for "a"/"w". */
    fp = fopen(filename, "a");
    if (fp == NULL) {
        perror("Unable to open file for writing");
        return;
    }
    printf("File '%s' is writable\n", filename);
    fclose(fp);
}

/* ---------- 3. Wrong command-line arguments ------------------------ */

static const char *GROUPS[] = {"A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-"};

static int valid_group(const char *g)
{
    for (size_t i = 0; i < sizeof GROUPS / sizeof GROUPS[0]; i++)
        if (strcmp(g, GROUPS[i]) == 0)
            return 1;
    return 0;
}

static int parse_positive(const char *s, long *out)
{
    char *end;
    if (*s == '\0')
        return 0;
    long v = strtol(s, &end, 10);
    if (*end != '\0' || v <= 0)
        return 0;
    *out = v;
    return 1;
}

static void usage(void)
{
    printf("Usage:\n");
    printf("  bloodbank find <group>\n");
    printf("  bloodbank issue <group> <units>\n");
    printf("  bloodbank register <id>\n");
    printf("Valid groups: A+ A- B+ B- AB+ AB- O+ O-\n");
}

static int test_cli(int argc, char *argv[])
{
    /* argv[0]="b2_adversarial" argv[1]="cli" argv[2]=command ... */
    int n = argc - 1;                        /* arguments after "cli" */
    char **a = argv + 1;                     /* a[0]=="cli" */

    if (n < 2) {
        printf("Error: no command given.\n");
        usage();
        return 1;
    }
    if (strcmp(a[1], "find") == 0) {
        if (n != 3) {
            printf("Error: 'find' needs exactly one blood group.\n");
            usage();
            return 1;
        }
        if (!valid_group(a[2])) {
            printf("Error: '%s' is not a valid blood group.\n", a[2]);
            usage();
            return 1;
        }
        printf("Searching blood group: %s\n", a[2]);
        return 0;
    }
    if (strcmp(a[1], "issue") == 0) {
        long units;
        if (n != 4) {
            printf("Error: 'issue' needs a blood group and a quantity.\n");
            usage();
            return 1;
        }
        if (!valid_group(a[2])) {
            printf("Error: '%s' is not a valid blood group.\n", a[2]);
            usage();
            return 1;
        }
        if (!parse_positive(a[3], &units)) {
            printf("Error: '%s' is not a positive whole number.\n", a[3]);
            usage();
            return 1;
        }
        printf("Issuing %ld unit(s) of %s\n", units, a[2]);
        return 0;
    }
    printf("Error: unknown command '%s'.\n", a[1]);
    usage();
    return 1;
}

/* ---------- 4. getline() alternative ------------------------------- */

static void test_getline(void)
{
    char *line = NULL;
    size_t cap = 0;

    printf("Enter a donor name (any length): ");
    ssize_t len = getline(&line, &cap, stdin);
    if (len == -1) {
        printf("No input received.\n");
    } else {
        if (len > 0 && line[len - 1] == '\n')
            line[--len] = '\0';
        printf("Read %zd characters: \"%s\"\n", len, line);
    }
    free(line);                              /* getline allocates: we must free */
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s name | file [filename] | cli ... | getline\n", argv[0]);
        return 1;
    }
    if (strcmp(argv[1], "name") == 0)    { test_name();    return 0; }
    if (strcmp(argv[1], "file") == 0)    { test_file(argc > 2 ? argv[2] : "donors.txt"); return 0; }
    if (strcmp(argv[1], "cli") == 0)     { return test_cli(argc, argv); }
    if (strcmp(argv[1], "getline") == 0) { test_getline(); return 0; }

    printf("Unknown test '%s'\n", argv[1]);
    return 1;
}
