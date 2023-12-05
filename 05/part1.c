
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

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

long do_map(struct map_el *me, long key) {
    long result = key;
    while (me) {
        if ((result >= me->source) && (result <= me->source + me->length - 1)) {
            result = (result - me->source) + me->dest;
            break;
        }
        me = me->next;
    }
    return result;
}

long get_location(long seedid) {
    long result = seedid;
    if (debug) printf("Seed %ld: ", result);
    for (int i = 0; i < map_count; i++) {
        result = do_map(maps[i], result);
        if (debug) printf("%ld ", result);
    }
    if (debug) printf("\n");
    return result;
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
        printf("Map count is %d\n", map_count);
    }
    // determine the location for each seed and collect the lowest
    long target_loc = -1;
    for (int i = 0; i < seed_count; i++) {
        long loc = get_location(seeds[i].id);
        if (debug) printf("Seed %ld: %ld\n", seeds[i].id, loc);
        if (target_loc == -1) {
            target_loc = loc;
        } else {
            target_loc = MIN(target_loc, loc);
        }
    }
    printf("Lowest location number is %ld\n", target_loc);

    return EXIT_SUCCESS;
}

