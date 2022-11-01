/*--------------------------------------------------------------------*/
/* symtablehash.c                                                     */
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

typedef struct Binding Binding;
struct Binding {
    /* Key for the binding */
    const char *key;
    /* Value associated with the key */
    void *value;
    /* The next key-value pair in the bucket */
    struct Binding *next;
};

struct SymTable {
    /* array of buckets containig bindings (key-value pairs) */
    struct Binding **buckets;
    /* Numer of bindings in symbol table */
    size_t numBindings;
    /* Number of buckets in symbol table */
    size_t size;
};

/* Return a hash code for pcKey that is between 0 and uBucketCount-1,
   inclusive. */
static size_t SymTable_hash(const char *pcKey, size_t uBucketCount) {
    const size_t HASH_MULTIPLIER = 65599;
    size_t u;
    size_t uHash = 0;

    assert(pcKey != NULL);

    for (u = 0; pcKey[u] != '\0'; u++)
        uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];

    return uHash % uBucketCount;
}

SymTable_T SymTable_new() {
    SymTable_T symtable = (struct SymTable*) malloc(sizeof(struct SymTable));
    int i = 0;
    if (symtable == NULL) return NULL;
    symtable->buckets = (Binding **) calloc(BUCKET_COUNT, sizeof(Binding *));
    symtable->size = BUCKET_COUNT;
    symtable->numBindings = 0;
    return symtable;
}
void SymTable_free(SymTable_T oSymTable) {
    size_t i = 0;
    for (; i < oSymTable->size; i++) {
        /* for each bucket, get the first binding */
        Binding *b = oSymTable->buckets[i];
        Binding *next;
        while (b != NULL) {
            /* for each binding, free its key, value, then the binding */
            next = b->next;
            free((char *) b->key);
            free(b);
            b = next;
        }
    }
    free(oSymTable->buckets);
    /* printf("before\n"); */
    free(oSymTable);
    /* printf("after\n"); */
}

int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    size_t hash = SymTable_hash(pcKey, oSymTable->size);
    /* oSymTable->size = BUCKET_COUNT; */
    /* // printf("HELLO HERE\n"); */
    Binding *b = oSymTable->buckets[hash];
    Binding *prev = b;
    Binding *binding;
    while (b != NULL) {
        if (strcmp(b->key, pcKey) == 0) return 0;
        prev = b;
        b = b->next;
    }
    binding = (Binding *) malloc(sizeof(Binding));
    if (binding == NULL) return 0;

    /* Check the # of bindings and increase # of buckets as eeded */


    binding->key = (const char *)malloc(strlen(pcKey) + 1);
    strcpy((char *)binding->key, pcKey);
    binding->value = (void *) pvValue;
    binding->next = NULL;

    /* the bucket is empty */
    oSymTable->numBindings++;
    if (prev == b) {
        oSymTable->buckets[hash] = binding;
    } else {
        prev->next = binding;
    }
    return 1;
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    size_t hash = SymTable_hash(pcKey, oSymTable->size);
    /* printf(" -%ld\n", hash); */
    Binding *b = oSymTable->buckets[hash];
    /* printf("%d\n", b); */
    while (b != NULL) {
        /* printf("%s\n", b->key); */
        if (strcmp(b->key, pcKey) == 0) return 1;
        b = b->next;
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
    size_t i = 0;
    for (; i < oSymTable->size; i++) {
        /* for each bucket, get the first binding */
        Binding *b = oSymTable->buckets[i];
        while (b != NULL) {
            /* for each binding, apply the function */

            pfApply(b->key, b->value, (void *)pvExtra);
            b = b->next;
        }
    }
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    size_t hash = SymTable_hash(pcKey, oSymTable->size);
    Binding *b = oSymTable->buckets[hash];
    while (b != NULL) {
        if (strcmp(b->key, pcKey) == 0) return b->value;
        b = b->next;
    }
    return NULL;
}



void *SymTable_replace(SymTable_T oSymTable, const char *pcKey,
                       const void *pvValue) {
    size_t hash = SymTable_hash(pcKey, oSymTable->size);
    Binding *b = oSymTable->buckets[hash];
    while (b != NULL) {
        if (strcmp(b->key, pcKey) == 0) {
            void *oldValue = b->value;
            b->value = (void *) pvValue;
            return oldValue;
        }
        b = b->next;
    }
    return NULL;
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
    size_t hash = SymTable_hash(pcKey, oSymTable->size);
    Binding *b = oSymTable->buckets[hash];
    Binding *prev = b;
    while (b != NULL) {
        if (strcmp(b->key, pcKey) == 0) {
            Binding *next = b->next;
            void *value;
            /* The first binding in the bucket is being removed */
            if (prev == b) {
                oSymTable->buckets[hash] = next;
            } else {
                prev->next = next;
            }
            value = b->value;
            oSymTable->numBindings--;
            free((char *) b->key);
            /* free(b->value);
            free(b->next); */
            free(b);
            return value;
        }
        prev = b;
        b = b->next;
    }
    return NULL;
}
