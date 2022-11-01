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

/* Enum containing the initial bucket count */
enum { BUCKET_COUNT = 509 };
/* Static array containing bucket sizes hash table can expand to */
static const size_t auBucketCounts[] = {509,  1021,  2039,  4093,
                                        8191, 16381, 32749, 65521};

/* shortened form for struct Binding */
typedef struct Binding Binding;

/* A Binding object consists of a unique key and value pair, and a pointer to
 * the next binding in the list. */
struct Binding {
    /* Key for the binding */
    const char *key;
    /* Value associated with the key */
    void *value;
    /* The next key-value pair in the bucket */
    struct Binding *next;
};

/* A SymTable object consists of an array of buckets (where each bucket
 * stores a linked list of key-value bindings), the number of bindings, and
 * the number of buckets in the table. */
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
        Binding *binding = oSymTable->buckets[i];
        Binding *next;
        while (binding != NULL) {
            next = binding->next;
            free((char *)binding->key);
            free(binding);
            binding = next;
        }
    }
    free(oSymTable->buckets);
    free(oSymTable);
}

int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    /* i is a counter variable keeping track of the index in auBucketCounts for
     * the new bucket size (if relevant). j is a counter variable used in
     * various loops */
    size_t i = 0, j = 0;
    size_t hash;
    size_t length = sizeof(auBucketCounts) / sizeof(auBucketCounts[0]);
    Binding *binding, *prev, *newBinding;
    SymTable_T tempSymTable;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Create a new binding and insert it at the end of the bucket */
    hash = SymTable_hash(pcKey, oSymTable->size);
    binding = oSymTable->buckets[hash];
    prev = binding;
    while (binding != NULL) {
        if (strcmp(binding->key, pcKey) == 0) return 0;
        prev = binding;
        binding = binding->next;
    }
    newBinding = (Binding *)malloc(sizeof(Binding));
    if (newBinding == NULL) return 0;

    newBinding->key = (const char *)malloc(strlen(pcKey) + 1);
    if (newBinding->key == NULL) return 0;
    strcpy((char *)newBinding->key, pcKey);
    newBinding->value = (void *)pvValue;
    newBinding->next = NULL;

    oSymTable->numBindings++;
    if (prev == binding) { /* the bucket is empty */
        oSymTable->buckets[hash] = newBinding;
    } else {
        prev->next = newBinding;
    }

    /* Check if the hash table should and can be expanded. */
    if (!(oSymTable->numBindings >= oSymTable->size &&
          oSymTable->size != auBucketCounts[length - 1]))
        return 1;

    /* Uncomment below to use non-expanding hash table implementation. */
    /* if(1) return 1; */

    /* Expand hash table and increase the of buckets if we're not already at the
     * maximum size and if the number of bindings equals the number of buckets.
     */
    /* Find how many buckets the expanded table should have */
    for (; i < length - 1; i++)
        if (oSymTable->numBindings == auBucketCounts[i]) break;

    tempSymTable = (struct SymTable *)malloc(sizeof(struct SymTable));

    /* If an expansion attempt fails because of insufficient memory,
     * simply proceed with the execution of SymTable_put. */
    if (tempSymTable == NULL) return 1;
    tempSymTable->buckets =
        (Binding **)calloc(auBucketCounts[i + 1], sizeof(Binding *));
    tempSymTable->numBindings = 0;
    tempSymTable->size = auBucketCounts[i + 1];

    /* For each binding in the original hash table, recalculate the
     * hashes for the keys using the larger bucket size, and add the
     * binding to the new hash table */
    for (j = 0; j < oSymTable->size; j++) {
        Binding *binding = oSymTable->buckets[j];
        while (binding != NULL) {
            int res = SymTable_put(tempSymTable, binding->key, binding->value);
            if (res == 0) return 1;
            binding = binding->next;
        }
    }
    /* Free the buckets and bindings from the original hash table. Point to the
     * new array of expanded buckets */
    j = 0;
    for (; j < oSymTable->size; j++) {
        Binding *binding = oSymTable->buckets[j];
        Binding *next;
        while (binding != NULL) {
            next = binding->next;
            free((char *)binding->key);
            free(binding);
            binding = next;
        }
    }
    free(oSymTable->buckets);
    oSymTable->buckets = tempSymTable->buckets;
    free(tempSymTable);
    oSymTable->size = auBucketCounts[i + 1];
    return 1;
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    size_t hash = SymTable_hash(pcKey, oSymTable->size);
    Binding *binding;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    binding = oSymTable->buckets[hash];
    while (binding != NULL) {
        if (strcmp(binding->key, pcKey) == 0) return binding->value;
        binding = binding->next;
    }
    return NULL;
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    size_t hash = SymTable_hash(pcKey, oSymTable->size);
    Binding *binding;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    binding = oSymTable->buckets[hash];
    while (binding != NULL) {
        if (strcmp(binding->key, pcKey) == 0) return 1;
        binding = binding->next;
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
    size_t i = 0;
    assert(oSymTable != NULL);
    assert(pfApply != NULL);
    for (; i < oSymTable->size; i++) {
        Binding *binding = oSymTable->buckets[i];
        while (binding != NULL) {
            (*pfApply)(binding->key, binding->value, (void *)pvExtra);
            binding = binding->next;
        }
    }
}

void *SymTable_replace(SymTable_T oSymTable, const char *pcKey,
                       const void *pvValue) {
    size_t hash = SymTable_hash(pcKey, oSymTable->size);
    Binding *binding;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    binding = oSymTable->buckets[hash];
    while (binding != NULL) {
        if (strcmp(binding->key, pcKey) == 0) {
            void *oldValue = binding->value;
            binding->value = (void *)pvValue;
            return oldValue;
        }
        binding = binding->next;
    }
    return NULL;
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
    size_t hash = SymTable_hash(pcKey, oSymTable->size);
    Binding *binding, *prev;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    binding = oSymTable->buckets[hash];
    prev = binding;
    /* go through the bindings in the bucket with the corresponding hash. change
     * the pointer for the previous binding (if it exists) */
    while (binding != NULL) {
        if (strcmp(binding->key, pcKey) == 0) {
            Binding *next = binding->next;
            void *value;
            if (prev == binding) {
                oSymTable->buckets[hash] = next;
            } else {
                prev->next = next;
            }
            value = binding->value;
            oSymTable->numBindings--;
            free((char *)binding->key);
            free(binding);
            return value;
        }
        prev = binding;
        binding = binding->next;
    }
    return NULL;
}
