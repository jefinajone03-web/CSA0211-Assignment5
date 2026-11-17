/*
 * Task B4 - Final engineering decision: growable donor list
 * Student : S Madhu Shri   Reg No: 1921260293   Course: CSA 0211   Slot: D
 *
 * Compile : gcc b4_growable_list.c -o b4_growable_list -Wall -Wextra -fsanitize=address -g
 * Run     : ./b4_growable_list
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int  id;
    char group[4];                 /* "AB+" + '\0' */
} Donor;

typedef struct {
    Donor *data;
    int    size;
    int    capacity;
} DonorList;

/* Adds one donor, doubling the capacity when the list is full.
   Returns 1 on success, 0 if memory could not be allocated. */
int add_donor(DonorList *list, int id, const char *group)
{
    if (list->size == list->capacity) {
        int newCapacity = (list->capacity == 0) ? 2 : list->capacity * 2;

        /* Use a temporary pointer: if realloc fails, list->data is still valid */
        Donor *p = realloc(list->data, (size_t)newCapacity * sizeof(Donor));
        if (p == NULL)
            return 0;

        list->data     = p;
        list->capacity = newCapacity;
    }

    list->data[list->size].id = id;
    strncpy(list->data[list->size].group, group, 3);
    list->data[list->size].group[3] = '\0';
    list->size++;
    return 1;
}

void free_list(DonorList *list)
{
    free(list->data);
    list->data     = NULL;         /* no dangling pointer, safe against double free */
    list->size     = 0;
    list->capacity = 0;
}

int main(void)
{
    DonorList list = {NULL, 0, 0};

    int         ids[]    = {7001, 7002, 7003, 7004, 7005};
    const char *groups[] = {"O+", "A+", "B-", "AB+", "O-"};

    for (int i = 0; i < 5; i++) {
        if (!add_donor(&list, ids[i], groups[i])) {
            printf("Memory allocation failed\n");
            free_list(&list);
            return 1;
        }
        printf("added %d (%s): size = %d, capacity = %d\n",
               ids[i], groups[i], list.size, list.capacity);
    }

    free_list(&list);
    return 0;
}
