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

/* shortened form for struct Node */
typedef struct Node Node;

/* A Node object consists of a unique key and value pair, and a pointer to the
 * next Node in the list. */
struct Node {
    /* Key for the binding */
    const char *key;
    /* Value associated with the key */
    void *value;
    /* The next key-value pair in the linked list */
    struct Node *next;
};

/* A SymTable object consists of the first node in the linked list and the
 * size of the list */
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
    Node *toInsert;
    assert(pcKey != NULL);

    /* Create the new node to be inserted */
    toInsert = (Node *)malloc(sizeof(Node));
    if (toInsert == NULL) return 0;
    toInsert->key = (const char *)malloc((strlen(pcKey) + 1));
    if (toInsert->key == NULL) return 0;
    strcpy((char *)toInsert->key, pcKey);
    toInsert->value = (void *)pvValue;
    toInsert->next = NULL;

    if (head == NULL) {
        oSymTable->first = toInsert;
        oSymTable->numBindings++;
        return 1;
    }
    prev = head;
    /* iterate through the linked list to get the last node, after which the new
     * node will be inserted */
    while (head != NULL) {
        if (strcmp(head->key, pcKey) == 0) {
            free((char *)toInsert->key);
            free(toInsert);
            return 0;
        }
        prev = head;
        head = head->next;
    }
    oSymTable->numBindings++;
    prev->next = toInsert;
    return 1;
}

void SymTable_free(SymTable_T oSymTable) {
    Node *head;
    Node *temp;
    assert(oSymTable != NULL);
    head = oSymTable->first;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free((char *)temp->key);
        free(temp);
    }
    free(oSymTable);
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    Node *head;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    head = oSymTable->first;
    while (head != NULL) {
        if (strcmp(head->key, pcKey) == 0) return 1;
        head = head->next;
    }
    return 0;
}

size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);
    return oSymTable->numBindings;
}

void SymTable_map(SymTable_T oSymTable,
                  void (*pfApply)(const char *pcKey, void *pvValue,
                                  void *pvExtra),
                  const void *pvExtra) {
    Node *head;
    assert(oSymTable != NULL);
    assert(pfApply != NULL);
    head = oSymTable->first;
    while (head != NULL) {
        (*pfApply)(head->key, head->value, (void *)pvExtra);
        head = head->next;
    }
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    Node *head;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    head = oSymTable->first;
    while (head != NULL) {
        if (strcmp(head->key, pcKey) == 0) return head->value;
        head = head->next;
    }
    return NULL;
}

void *SymTable_replace(SymTable_T oSymTable, const char *pcKey,
                       const void *pvValue) {
    Node *head;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    head = oSymTable->first;
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
    Node *head;
    Node *prev;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    head = oSymTable->first;
    prev = head;
    while (head != NULL) {
        if (strcmp(head->key, pcKey) == 0) {
            void *original = head->value;
            if (head == prev) {
                oSymTable->first = head->next;
            } else {
                prev->next = head->next;
            }
            free((char *)head->key);
            free(head);
            oSymTable->numBindings--;
            return original;
        }
        prev = head;
        head = head->next;
    }
    return NULL;
}
