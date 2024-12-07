
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output
#define MAX_BRICKS 1500
struct brick {
    int x1; int y1; int z1;
    int x2; int y2; int z2;
} bricks[MAX_BRICKS];
unsigned brick_count = 0;
int minx = 0, miny = 0;
int maxx = 0, maxy = 0;
int maxz = 0;
int map_width = 0, map_depth = 0;

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int line_count = 0;
    char line[LINE_LENGTH];
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        if (strlen(line) != 0) {
            // store the data
            sscanf(line, "%d,%d,%d", &bricks[brick_count].x1, &bricks[brick_count].y1, &bricks[brick_count].z1);
            minx = MIN(minx, MIN(bricks[brick_count].x1, bricks[brick_count].x2));
            maxx = MAX(maxx, MAX(bricks[brick_count].x1, bricks[brick_count].x2));
            miny = MIN(miny, MIN(bricks[brick_count].y1, bricks[brick_count].y2));
            maxy = MAX(maxy, MAX(bricks[brick_count].y1, bricks[brick_count].y2));
            maxz = MAX(maxz, MAX(bricks[brick_count].z1, bricks[brick_count].z2));
            char *end = strchr(line, '~');
            if (end != NULL) {
                sscanf(end + 1, "%d,%d,%d", &bricks[brick_count].x2, &bricks[brick_count].y2, &bricks[brick_count].z2);
            }
            brick_count++;
        } else if (errno != 0) {
            perror("sscanf");
        } else {
            fprintf(stderr, "Unexpected input format '%s'.\n", line);
        }

        line_count++;
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

void print_brick(struct brick b) {
    printf("(%d, %d, %d) -> ", b.x1, b.y1, b.z1);
    printf("(%d, %d, %d)", b.x2, b.y2, b.z2);
}

void print_bricks(void) {
    for (unsigned i = 0; i < brick_count; i++) {
        print_brick(bricks[i]);
        printf("\n");
    }
}

void verify_bricks(void) {
    for (unsigned i = 0; i < brick_count; i++) {
         assert(bricks[i].x1 <= bricks[i].x2);
         assert(bricks[i].y1 <= bricks[i].y2);
         assert(bricks[i].z1 <= bricks[i].z2);
    }
}

int compare_bricks_height(const void *e1, const void *e2) {
    struct brick *b1 = (struct brick *)e1;
    struct brick *b2 = (struct brick *)e2;
    return b1->z1 - b2->z1;
}

void sort_bricks(void) {
    qsort(bricks, brick_count, sizeof(struct brick), compare_bricks_height);
}

int overlapped_bricks(struct brick *a, struct brick *b) {
    return (MAX(a->x1, b->x1) <= MIN(a->x2, b->x2)) && (MAX(a->y1, b->y1) <= MIN(a->y2, b->y2));;
}

void drop_bricks(void) {
    for (unsigned i = 0; i < brick_count; i++) {
        int max_z = 1;
        for (unsigned j = 0; j < i; j++) {
            if (overlapped_bricks(bricks + i, bricks + j)) {
                max_z = MAX(max_z, bricks[j].z2 + 1);
            }
        }
        assert(bricks[i].z1 >= max_z);
        if (bricks[i].z1 != max_z) {
            bricks[i].z2 -= (bricks[i].z1 - max_z);
            bricks[i].z1 = max_z;
        }
    }
}

struct elem {
    unsigned brickid;
    struct elem *next, *prev;
} *supports[MAX_BRICKS], *supported[MAX_BRICKS];
void determine_support(void) {
    for (unsigned b = 0; b < brick_count; b++) {
        supports[b] = NULL;
        supported[b] = NULL;
    }
    for (unsigned b_high = 0; b_high < brick_count; b_high++) {
        for (unsigned b_low = 0; b_low < b_high; b_low++) {
            if (overlapped_bricks(bricks + b_high, bricks + b_low) && (bricks[b_high].z1 == bricks[b_low].z2 + 1)) {
                struct elem *sup1 = malloc(sizeof(struct elem));
                sup1->brickid = b_high; sup1->next = supports[b_low]; supports[b_low] = sup1;
                struct elem *sup2 = malloc(sizeof(struct elem));
                sup2->brickid = b_low; sup2->next = supported[b_high]; supported[b_high] = sup2;
            }
        }
    }
}
void print_support(struct elem **sups) {
    for (unsigned b = 0; b < brick_count; b++) {
        printf("%d: (", b);
        struct elem *s = sups[b];
        while (s != NULL) {
            printf("%d%s", s->brickid, (s->next != NULL) ? ", " : "");
            s = s->next;
        }
        printf(")%s", (b < brick_count - 1) ? ", " : "");
    }
    printf("\n");
}
unsigned count_support(struct elem **sups, unsigned brickid) {
    unsigned count = 0;
    struct elem *s = sups[brickid];
    while (s != NULL) {
        count += 1;
        s = s->next;
    }
    return count;
}

// Simple deque with only append at end and pop from front
struct deque_elem {
    void *value;
    struct deque_elem *next;
};
struct simple_deque {
    struct deque_elem *first;
    struct deque_elem *last;
    unsigned elem_count;
};
struct simple_deque *simple_deque_init(void) {
    struct simple_deque *q = malloc(sizeof(struct simple_deque));
    assert(q != NULL);
    q->first = NULL; q->last = NULL; q->elem_count = 0;
    return q;
}
void simple_deque_add(struct simple_deque *q, void *value) {
    struct deque_elem *e = malloc(sizeof(struct deque_elem));
    assert(e != NULL);
    e->value = value; e->next = NULL;
    if (q->elem_count == 0) {
        q->first = e;
    } else {
        q->last->next = e;
    }
    q->last = e;
    q->elem_count += 1;
}
void *simple_deque_popleft(struct simple_deque *q) {
    struct deque_elem *e = q->first;
    assert(q->elem_count > 0);
    void *value = e->value;
    q->first = q->first->next;
    q->elem_count -= 1;
    free(e);
    return value;
}
unsigned simple_deque_len(struct simple_deque *q) {
    return q->elem_count;
}


int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    printf("There are %d bricks in the air.\n", brick_count);
    //if (debug) print_bricks();
    printf("Minimum is %dx%d\n", minx, miny);
    printf("Maximum is %dx%d\n", maxx, maxy);
    map_width = maxx - minx + 1;
    map_depth = maxy - miny + 1;

    // sort the bricks in ascending 'z' order
    verify_bricks(); // make sure the z1 is always <= z2 for all bricks
    sort_bricks();
//    if (debug) { printf("Initial bricks snapshot(sorted):\n"); print_bricks(); }

    // drop all bricks so everything is stacked, and sort them again
    drop_bricks();
    sort_bricks();
//    if (debug) { printf("After initial dropping:\n"); print_bricks(); }

    determine_support();
    if (debug) print_support(supports);
    if (debug) print_support(supported);

#ifdef PART1
    // re-implement part 1 with the new structures
    int count = 0;
    for (unsigned b = 0; b < brick_count; b++) {
        struct elem *sup = supports[b];
        int all_valid = 1;
        while (sup != NULL) {
            if (count_support(supported, sup->brickid) < 2) {
                all_valid = 0;
                break;
            }
            sup = sup->next;
        }
        if (all_valid) count += 1;
    }
    printf("The count of individual bricks that can be disintegrated is %d\n", count);
#else
    int count = 0;
    int falling[MAX_BRICKS];
    for (unsigned b = 0; b < brick_count; b++) {
        if (debug) printf("Handling brick %d:\n", b);
        struct simple_deque *q = simple_deque_init();
        for (unsigned i = 0; i < brick_count; i++) falling[i] = 0;
        struct elem *p = supports[b];
        while (p != NULL) {
            if (debug) printf("  checking if %d should be added\n", p->brickid);
            // check if it is only supported by one brick
            if (supported[p->brickid]->next == NULL) {
                long val = p->brickid;
                simple_deque_add(q, (void *)val);
                falling[p->brickid] = 1;
                if (debug) printf("  falling: %d\n", p->brickid);
            }
            p = p->next;
        }
        falling[b] = 1;

        // determine the other bricks that are falling because of this
        while (simple_deque_len(q) != 0) {
            long candidate = (long)simple_deque_popleft(q);
            if (debug) printf("  popped: %ld\n", candidate);
            struct elem *k = supports[candidate];
            while (k != NULL) {
                if (!falling[k->brickid]) {
                    struct elem *l = supported[k->brickid];
                    int still_supported = 0; 
                    while (l != NULL) {
                        if (!falling[l->brickid]) {
                            still_supported = 1;
                        }
                        l = l->next;
                    }
                    if (!still_supported) {
                        long zzz = k->brickid;
                        simple_deque_add(q, (void *)zzz);
                        falling[k->brickid] = 1;
                        if (debug) printf("  falling: %d\n", k->brickid);
                    }
                }
                k = k->next;
            }
        }

        // count the falling blocks
        int falling_count = 0;
        for (unsigned i = 0; i < brick_count; i++) {
            if (falling[i]) falling_count += 1;
        }
        if (debug) printf("there are now %d brick falling\n", falling_count - 1);
        count += (falling_count - 1);
    }
    printf("The total sum of falling bricks is %d\n", count);
#endif

    printf("Info: the solution for the sample data should be %ld\n", 7L);
    printf("Info: the solution for the actual data should be %ld\n", 35654L);
    return EXIT_SUCCESS;
}

