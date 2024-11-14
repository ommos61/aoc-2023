#ifndef _DICT_H_
#define _DICT_H_
//================================================

struct dict *dictCreate(int size, int (*compare)(const void*, const void*));
void *dictGet(struct dict *d, void *key);
void dictPut(struct dict *d, void *key, void *value);
#endif /* _DICT_H_ */

// Dictionary implementation
#ifdef DICT_IMPLEMENTATION
#define DICT_DEFAULT_CAP 200000
struct dict {
    void **items;
    int size;
    int new_index;
    void *last;
    int last_index;
    int (*compare)(const void*, const void*);
};

struct dict *dictCreate(int size, int (*compare)(const void*, const void*)) {
    struct dict *result = malloc(sizeof(struct dict));
    result->size = (size != 0) ? size : DICT_DEFAULT_CAP;
    result->items = malloc(result->size * sizeof(void *));
    result->compare = compare;
    result->new_index = 0;
    result->last = NULL;
    result->last_index = -1;

    assert(result != NULL && result->items != NULL && result->compare != NULL);
    return result;
}

void *dictGet(struct dict *d, void *key) {
    // this returns a pointer to an existing element, the caller can modify
    // the contents of the element
    d->last_index = -1;
    d->last = NULL;
    for (int i = 0; i < d->new_index; i++) {
        if (d->compare(d->items[i], key) == 0) {
            d->last_index = i;
            d->last = d->items[i];
            break;
        }
    }
    return d->last;
}

void dictPut(struct dict *d, void *key, void *value) {
    assert((key == value) && "Only support 'key' and 'values' pointing to same memory");
    // assumption is that if the element already exists, it is searched for
    // just before and last_index points to it, otherwise it is a new entry
    if (d->last_index == -1) {
        assert((d->new_index < d->size) && "dict overflow");
        d->items[d->new_index] = key;
        d->new_index++;
    }
}
#endif
