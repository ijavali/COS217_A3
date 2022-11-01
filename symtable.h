/*--------------------------------------------------------------------*/
/* symtable.h                                                         */
/* Author: Ishaan Javali                                              */
/*--------------------------------------------------------------------*/

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stddef.h>

struct SymTable;

/* The SymTable_T stores a collection of key-value bindings where
 * keys are unique. */
typedef struct SymTable *SymTable_T;

/* Return a new SymTable object that contains no bindings, or NULL if
 * insufficient memory is available.*/
SymTable_T SymTable_new(void);

/* Frees all memory occupied by oSymTable. */
void SymTable_free(SymTable_T oSymTable);

/* Returns the number of bindings in oSymTable. */
size_t SymTable_getLength(SymTable_T oSymTable);

/* Returns 1 and adds a new binding to oSymTable consisting of key pcKey
 * and value pvValue if oSymTable does not contain a binding with key pcKey
 * and if sufficient memory is available, otherwise returns 0. */
int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue);

/* If oSymTable contains a binding with key pcKey, replaces
 the binding's value with pvValue and returns the old value, otherwise it
 returns NULL. */
void *SymTable_replace(SymTable_T oSymTable, const char *pcKey,
                       const void *pvValue);

/* Returns 1 if oSymTable contains a binding whose key is pcKey, and 0
 * otherwise. */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey);

/* Returns the value of the binding within oSymTable whose key is pcKey, or NULL
 * if no such binding exists. */
void *SymTable_get(SymTable_T oSymTable, const char *pcKey);

/* Returns the value for the binding with key pcKey and removes the binding if
 * oSymTable contains the binding, otherwise returns NULL */
void *SymTable_remove(SymTable_T oSymTable, const char *pcKey);

/* Applies function *pfApply to each binding in oSymTable, using pcKey, pcValue,
 * and pvExtra as arguments to *pfApply */
void SymTable_map(SymTable_T oSymTable,
                  void (*pfApply)(const char *pcKey, void *pvValue,
                                  void *pvExtra),
                  const void *pvExtra);

#endif