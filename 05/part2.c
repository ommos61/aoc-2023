
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output

// Global data for the seeds
#define MAX_SEEDS 50
struct seed {
    long id;
};
struct seed seeds[MAX_SEEDS];
int seed_count = 0;

// Global data for the maps (in order)
struct map_el {
    long dest;
    long source;
    long length;
    struct map_el *next;
};
#define MAX_MAPS 10
struct map_el *maps[MAX_MAPS];
int map_count = 0;

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    // clear the maps
    for (unsigned int i = 0; i < array_count(maps); i ++) maps[i] = NULL;
    if (debug) printf("Size of long is %ld\n", sizeof(long));

    int line_count = 0;
    char line[LINE_LENGTH];
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        if (strlen(line) == 0) {
            // skip empty lines
        } else if (strncmp(line, "seeds: ", 7) == 0) {
            // parse the seeds
            char *p = line + strlen("seeds:");
            // skip spaces
            while (isspace(*p)) p++;
            while (*p) {
                // read the seed number
                long value = 0;
                sscanf(p, "%ld", &value);
                seeds[seed_count++].id = value;
                while (isdigit(*p)) p++;
                // skip spaces
                while (isspace(*p)) p++;
            }
        } else if (isdigit(*line)) {
            // parse a map element to the current map
            struct map_el *el = malloc(sizeof(struct map_el));
            sscanf(line, "%ld %ld %ld", &el->dest, &el->source, &el->length);
            el->next = maps[map_count - 1];
            maps[map_count - 1] = el;
        } else if (strcmp(line + strlen(line) - strlen("map:"), "map:") == 0) {
            // proceed to next map
            map_count++;
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

struct map_el *find_map(struct map_el *me, long key) {
    while (me) {
        if ((key >= me->source) && (key <= me->source + me->length - 1)) {
            return me;
        }
        me = me->next;
    }
    return NULL;
}

struct range {
    long start;
    long length;
    struct range *next;
};
struct range *new_range(long start, long length, struct range *next) {
    struct range *r = malloc(sizeof(struct range));
    r->start = start; r->length = length;
    r->next = next;
    return r;
}
void free_ranges(struct range *ranges) {
    struct range *r = ranges;
    while (r) {
        struct range *tmp = r;
        r = r->next;
        free(tmp);
    }
}
void print_ranges(struct range *ranges) {
    long count = 0;
    struct range *r = ranges;
    printf("Ranges: ");
    while (r) {
        count++;
        printf("(%ld, %ld) ", r->start, r->length);
        r = r->next;
    }
    printf("\nCount = %ld\n", count);
}

long get_location(long seedid, long length) {
    if (debug) printf("Seed %ld(%ld): ", seedid, length);
    struct range *ranges = new_range(seedid, length, NULL);
    for (int i = 0; i < map_count; i++) {
        // find the lowest for the ranges (start, length)
        //   by chipping away ranges from the map ranges found
        struct range *r = ranges, *new_ranges = NULL;
        while (r) {
            long start = r->start, length = r->length;
            while (length > 0) {
                struct map_el *m = find_map(maps[i], start);
                if (debug) printf("%ld ", start);
                if (m == NULL) {
                    if (new_ranges == NULL) {
                        new_ranges = new_range(start, 1, NULL);
                    } else if (start == new_ranges->start + new_ranges->length) {
                        new_ranges->length++;
                    } else {
                        new_ranges = new_range(start, 1, new_ranges);
                    }
                    length--; start++;
                } else {
                    // TODO: chip away the longest possible range
                    long len = m->length - (start - m->source);
                    if (len > length) {
                        len = length;
                    }
                    new_ranges = new_range(m->dest + (start - m->source), len, new_ranges);
                    length -= len;
                    start += len;
                }
            }
            r = r->next;
        }
        free_ranges(ranges);
        ranges = new_ranges;
    }
    if (debug) printf("\n");
    if (debug) print_ranges(ranges);

    // determine the lowest in the calculated location ranges
    long lowest = LONG_MAX;
    struct range *r = ranges;
    while (r) {
        lowest = MIN(lowest, r->start);
        r = r->next;
    }
    free_ranges(ranges);
    return lowest;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // Map the seeds to locations
    if (debug) {
        printf("Seeds: ");
        for (int i = 0; i < seed_count; i++) printf("%ld ", seeds[i].id);
        printf("\n");
        printf("Seed count is %d\n", seed_count);
        printf("Map count is %d\n", map_count);
    }
    // determine the location for each seed and collect the lowest
    long target_loc = LONG_MAX;
    assert((seed_count % 2) == 0);
    for (int i = 0; i < seed_count; i += 2) {
        if (debug) printf("Handling range %d (start = %ld, length = %ld)\n", i / 2, seeds[i].id, seeds[i+1].id);
        long loc = get_location(seeds[i].id, seeds[i+1].id);
        if (debug) printf("Seed %ld: %ld\n", seeds[i].id, loc);

        // determine the lowest
        target_loc = MIN(target_loc, loc);
    }
    printf("Lowest location number is %ld\n", target_loc);

    return EXIT_SUCCESS;
}

