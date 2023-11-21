#ifndef _QUEUE_H_
#define _QUEUE_H_
//================================================
#define QUEUE_DEFAULT_CAP 15000
typedef struct queue *queue;

queue queueCreate(int size, int(*compare)(const void*,const void*));
int queueLength(queue q);
void queuePush(queue q, const void *entry);
const void *queuePop(queue q);

#endif /* _QUEUE_H_ */

// Queue implementation
#ifdef QUEUE_IMPLEMENTATION
#include <assert.h>
struct queue {
    // items are sorted with largest first
    const void **items;
    int size;
    int end;
    int (*compare)(const void *, const void *);
};

queue queueCreate(int size, int(*compare)(const void*,const void*)) {
    queue result = malloc(sizeof(struct queue));
    result->size = (size != 0) ? size : QUEUE_DEFAULT_CAP;
    result->items = malloc(result->size * sizeof(void *));
    result->end = 0;
    result->compare = compare;

    return result;
}

int queueLength(queue q) {
    return q->end;
}

void queuePush(queue q, const void *entry) {
    assert((queueLength(q) < q->size) && "queue overflow");
    // find the place to insert
    if (q->end == 0) {
        q->items[q->end++] = entry;
    } else {
        // shift elements
        int i;
        for (i = q->end - 1; i >= 0; i--) {
            if (q->compare(q->items[i], entry) < 0) {
                q->items[i+1] = q->items[i];
            } else {
                break;
            }
        }
        // insert item
        q->items[i+1] = entry;
        q->end++;
    }
}

const void *queuePop(queue q) {
    assert((queueLength(q) > 0) && "queue underflow");
    // the end contains the item with the lowest key
    const void *result = q->items[--q->end];

    return result;
}
#endif
