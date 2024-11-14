#ifndef _DICT_HASHED_H_
#define _DICT_HASHED_H_
//================================================

struct dict *dictCreate(int size, int (*compare)(const void*, const void*));
void *dictGet(struct dict *d, void *element);
void dictPut(struct dict *d, void *key, void *value);

#ifdef DICT_STATISTICS
void dictPrintStatistics(struct dict *d);
#endif

typedef unsigned int (*dictHashFunction)(const void*);
extern dictHashFunction dictMyHashFunction;

// Dictionary implementation
//#define DICT_IMPLEMENTATION
#ifdef DICT_IMPLEMENTATION
#include <stdlib.h>
#include <assert.h>
#ifdef DICT_STATISTICS
  #include <stdio.h>
#endif

#ifndef DICT_DEFAULT_CAP
#define DICT_DEFAULT_CAP 200000
#endif

struct dict_elem {
    void *key;
    void *value;
    void *next;
};
struct dict {
    // the hash buckets
    struct dict_elem **items;
    int size;
#ifdef DICT_STATISTICS
    int item_count;
#endif

    // helper functions
    int (*compare)(const void*, const void*);
    unsigned int (*hash)(const void*);
};

dictHashFunction dictMyHashFunction = NULL;

struct dict *dictCreate(int size, int (*compare)(const void*, const void*)) {
    struct dict *result = (struct dict *)malloc(sizeof(struct dict));
    result->size = (size != 0) ? size : DICT_DEFAULT_CAP;
#ifdef DICT_STATITICS
    result->item_count = 0;
#endif
    // create the hash buckets
    result->items = (struct dict_elem **)malloc(result->size * sizeof(struct dict_elem*));
    memset(result->items, 0, result->size * sizeof(struct dict_elem*));
    // store the 'compare' and 'hash' functions
    result->compare = compare;
    result->hash = dictMyHashFunction;
    assert((result->hash != NULL) && "A hash-function should be assigned to 'dictMyHashFunction'");

    assert(result != NULL && result->items != NULL && result->compare != NULL);
    return result;
}

void *dictGet(struct dict *d, void *key) {
    assert(d->hash != NULL);
    int hash = d->hash(key) % d->size;
    struct dict_elem *hash_elems = d->items[hash];
    void *found_el = NULL;
    struct dict_elem *p = hash_elems;
    while (p != NULL) {
        if (d->compare(key, p->key) == 0) {
            found_el = p->value;
            break;
        }
        p = (struct dict_elem*)p->next;
    }
    return found_el;
}

void dictPut(struct dict *d, void *key, void *value) {
    assert(d->hash != NULL);
    int hash = d->hash(key) % d->size;
    struct dict_elem *p = d->items[hash];
    while (p != NULL) {
        if (d->compare(key, p->key) == 0) {
            p->key = key;
            p->value = value;
            break;
        }
        p = p->next;
    }
    if (p == NULL) {
        struct dict_elem *el = (struct dict_elem*)malloc(sizeof(struct dict_elem));
        el->key = key;
        el->value = value;
        el->next = d->items[hash];
        d->items[hash] = el;
    }
}

#ifdef DICT_STATISTICS
void dictPrintStatistics(struct dict *d) {
    printf("Info: no statistincs to show yet.\n");
}
#endif

#endif
#endif /* _DICT_H_ */
