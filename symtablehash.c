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
    SymTable_T symtable = (struct SymTable *)malloc(sizeof(struct SymTable));
    if (symtable == NULL) return NULL;
    symtable->buckets = (Binding **)calloc(BUCKET_COUNT, sizeof(Binding *));
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
            free((char *)b->key);
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
    size_t i = 0, j = 0;
    int failed = 0;
    size_t hash;
    size_t length = sizeof(auBucketCounts) / sizeof(auBucketCounts[0]);
    Binding *b, *prev, *binding;
     hash = SymTable_hash(pcKey, oSymTable->size);
    b = oSymTable->buckets[hash];
    prev = b;
    while (b != NULL) {
        if (strcmp(b->key, pcKey) == 0) return 0;
        prev = b;
        b = b->next;
    }
    binding = (Binding *)malloc(sizeof(Binding));
    if (binding == NULL) return 0;

    binding->key = (const char *)malloc(strlen(pcKey) + 1);
    if (binding->key == NULL) return 0;
    strcpy((char *)binding->key, pcKey);
    binding->value = (void *)pvValue;
    binding->next = NULL;

    oSymTable->numBindings++;
    /* the bucket is empty */
    if (prev == b) {
        oSymTable->buckets[hash] = binding;
    } else {
        prev->next = binding;
    }
    /* Expand hash table (if we're not already at the maximum size) by
     * increasing # of buckets if the # of bindings = # of buckets. */
    if (oSymTable->numBindings >= oSymTable->size &&
        oSymTable->size != auBucketCounts[length - 1]) {
        for (; i < length - 1; i++) {
            if (oSymTable->numBindings == auBucketCounts[i]) {

                SymTable_T newSymTable =
                    (struct SymTable *)malloc(sizeof(struct SymTable));

                /* If an expansion attempt fails because of insufficient memory,
                 * simply proceed with the execution of SymTable_put. */
                if (newSymTable == NULL) break;
                newSymTable->buckets = (Binding **)calloc(auBucketCounts[i + 1],
                                                          sizeof(Binding *));
                newSymTable->numBindings = 0;
                newSymTable->size = auBucketCounts[i + 1];

                for (j = 0; j < oSymTable->size; j++) {
                    Binding *b = oSymTable->buckets[j];
                    while (b != NULL) {
                        int res = SymTable_put(newSymTable, b->key, b->value);
                 /*        printf(" put %s %d", b->key, SymTable_hash(b->key, newSymTable->size));
                        printf("  = %d \n", newSymTable->buckets[SymTable_hash(b->key, newSymTable->size)]); */
                        if (res == 0) {
                            failed = 1;
                            break;
                        }
                        b = b->next;
                    }
                }
                /* If the expansion attempt did not fail, free the old table and
                 * use the new table */
                if (!failed) {
                    /* // SymTable_free(oSymTable); */
                    oSymTable->buckets = newSymTable->buckets;
                    /* printf("\n  %s %s \n", newSymTable->buckets[297]->key , oSymTable->buckets[297]->key);
                    printf(" NOW %d = %d", oSymTable->buckets , newSymTable->buckets); */
                    oSymTable->size = auBucketCounts[i + 1];
                    /* printf("  RESIZED to %d\n", oSymTable->size); */
                    /* printf("  CHANGED %d\n" , oSymTable->size); */
                }
                break;
            }
        }
    } 
   
    /* printf(" size: %ld", SymTable_getLength(oSymTable)); */
    return 1;
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    size_t hash = SymTable_hash(pcKey, oSymTable->size);
    Binding *b = oSymTable->buckets[hash];
    while (b != NULL) {
        if (strcmp(b->key, pcKey) == 0) return b->value;
        b = b->next;
    }

    /* printf("RETURNING NULL %s %d = %d\n", pcKey, hash, oSymTable->buckets[hash]); */
    return NULL;
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

void *SymTable_replace(SymTable_T oSymTable, const char *pcKey,
                       const void *pvValue) {
    size_t hash = SymTable_hash(pcKey, oSymTable->size);
    Binding *b = oSymTable->buckets[hash];
    while (b != NULL) {
        if (strcmp(b->key, pcKey) == 0) {
            void *oldValue = b->value;
            b->value = (void *)pvValue;
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
            free((char *)b->key);
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
