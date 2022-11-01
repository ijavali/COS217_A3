/*--------------------------------------------------------------------*/
/* symtablelist.c                                                     */
/* Author: Ishaan Javali                                              */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "symtable.h"

enum { BUCKET_COUNT = 509 };
static size_t auBucketCounts[] = {509,  1021,  2039,  4093,
                                  8191, 16381, 32749, 65521};

typedef struct Node Node;
/* TODO: write definition here. talk about variable names */
struct Node {
    /* Key for the binding */
    const char *key;
    /* Value associated with the key */
    void *value;
    /* The next key-value pair in the linked list */
    struct Node *next;
};

struct SymTable {
    /* The first node in the linked list */
    struct Node *first;
    /* Numer of nodes/bindings in symbol table */
    size_t numBindings;
};

SymTable_T SymTable_new() {
    SymTable_T symtable = (struct SymTable *)malloc(sizeof(SymTable_T));
    if (symtable == NULL) return NULL;
    symtable->first = NULL;
    symtable->numBindings = 0;
    return symtable;
}

int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    Node *head = oSymTable->first;
    Node *prev;
    Node *temp = (Node *)malloc(sizeof(Node));
    if (temp == NULL) return 0;
    temp->key = (const char *)malloc(strlen(pcKey) + 1);
    strcpy((char *)temp->key, pcKey);
    temp->value = (void *)pvValue;
    temp->next = NULL;
    if (head == NULL) {
        oSymTable->first = temp;
        oSymTable->numBindings++;
        /* printf("Created %s %d %d %d\n", temp->key, temp, temp->key,
               temp->value); */
        return 1;
    }
    prev = head;
    while (head != NULL) {
        if (strcmp(head->key, pcKey) == 0) return 0;
        prev = head;
        head = head->next;
    }
    oSymTable->numBindings++;
   /*  printf("Created %s %d %d %d\n", temp->key, temp, temp->key, temp->value); */
    prev->next = temp;
    return 1;
}

void SymTable_free(SymTable_T oSymTable) {
    Node *head = oSymTable->first;
    Node *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        char *k = temp->key;
        /* printf(" removing %s %d %d %d\n", temp->key, temp, (char *)temp->key,
               temp->value); */
        free((char *)temp->key);
        /* printf("   Fails below:\n "); */
        /* free(temp->value); */
        /*         free(temp->next); */
        free(temp);
    }
    free(oSymTable);
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    Node *head = oSymTable->first;
    while (head != NULL) {
        if (strcmp(head->key, pcKey) == 0) return 1;
        head = head->next;
    }
    return 0;
}

size_t SymTable_getLength(SymTable_T oSymTable) {
    return oSymTable->numBindings;
}

void SymTable_map(SymTable_T oSymTable,
                  void (*pfApply)(const char *pcKey, void *pvValue,
                                  void *pvExtra),
                  const void *pvExtra) {
    Node *head = oSymTable->first;
    while (head != NULL) {
        pfApply(head->key, head->value, (void *)pvExtra);
        head = head->next;
    }
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    Node *head = oSymTable->first;
    while (head != NULL) {
        if (strcmp(head->key, pcKey) == 0) return head->value;
        head = head->next;
    }
    return NULL;
}

void *SymTable_replace(SymTable_T oSymTable, const char *pcKey,
                       const void *pvValue) {
    Node *head = oSymTable->first;
    while (head != NULL) {
        if (strcmp(head->key, pcKey) == 0) {
            void *original = head->value;
            head->value = (void *)pvValue;
            return original;
        }
        head = head->next;
    }
    return NULL;
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
    /* printf(" --------- \n"); */
    Node *head = oSymTable->first;
    Node *prev = head;
    while (head != NULL) {
        if (strcmp(head->key, pcKey) == 0) {
            void *original = head->value;
            if (head == prev) {
                oSymTable->first = head->next;
            } else {
                prev->next = head->next;
            }
            free((char *)head->key);
            /* free(head->value); */
            /* free(head->next); */
            free(head);
            oSymTable->numBindings--;
            return original;
        }
        prev = head;
        head = head->next;
    }
    return NULL;
}
