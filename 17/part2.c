
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
int debug = 1; // 0 = no debug output, 1 = some extra debug output
#define MAX_ROWS 150
char *rows[MAX_ROWS];
int row_width = 0;
int row_count = 0;
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
            if (row_width == 0) {
                row_width = strlen(line);
            } else {
                assert(((unsigned)row_width == strlen(line)) && "rows must have the same width");
            }
            // store the row data
            rows[row_count] = malloc(row_width + 1);
            strcpy(rows[row_count], line);
            row_count++;
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

#define DIR_RIGHT   0
#define DIR_DOWN    1
#define DIR_LEFT    2
#define DIR_UP      3
#define DIR_UNKNOWN 4
struct dir {
    int dx, dy;
} dirs[] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
char *dirchars = ">v<^?";
// new directions: RIGHT     DOWN       LEFT      UP         UNKNOWN(start)
int go_left[]  = { DIR_UP,   DIR_RIGHT, DIR_DOWN, DIR_LEFT,  DIR_RIGHT };
int go_right[] = { DIR_DOWN, DIR_LEFT,  DIR_UP,   DIR_RIGHT, DIR_DOWN };
struct state {
    int posx, posy;
    int dir, dirsteps;
    int cost;
    struct state *prev;
};

#define DIJKSTRA_IMPLEMENTATION
#include "../common/dijkstra.h"
#undef DIJKSTRA_IMPLEMENTATION
#define QUEUE_IMPLEMENTATION
#include "../common/queue.h"
#undef QUEUE_IMPLEMENTATION
#define DICT_IMPLEMENTATION
#include "../common/dict_hashed.h"
#undef DICT_IMPLEMENTATION

unsigned int stateKeyHash(const void *key) {
    unsigned char *s = (unsigned char *)key;
    unsigned int hashval = 0;
    for (unsigned i = 0; i < 4 * sizeof(int); i++) {
        hashval = s[i] + 31 * hashval;
    }
    return hashval;
}

int stateCompare(const void *v1, const void *v2) {
    struct state *s1 = (struct state *)v1;
    struct state *s2 = (struct state *)v2;

    // same time, position, direction and direction steps
    return !((s1->dirsteps == s2->dirsteps) &&
            (s1->posx == s2->posx) && (s1->posy == s2->posy) && (s1->dir == s2->dir));
}

int stateCost(struct state *from, struct state *to) {
    int cost = 0;
    if (from->posy == to->posy) {
        int step = (from->posx < to->posx) ? 1 : -1;
        for (int x = from->posx; x != to->posx; x += step) {
            cost += (rows[from->posy][x + step] - '0');
        }
    } else if (from->posx == to->posx) {
        int step = (from->posy < to->posy) ? 1 : -1;
        for (int y = from->posy; y != to->posy; y += step) {
            cost += (rows[y + step][from->posx] - '0');
        }
    }

    return cost;
}

int costCompare(const void *v1, const void *v2) {
    struct state *s1 = (struct state *)v1;
    struct state *s2 = (struct state *)v2;

    return (s1->cost - s2->cost);
}

void statePrint(const char *prefix, const void *v) {
    struct state *s = (struct state *)v;

    printf("%s State(posdir = (%d, %d, %c, %d), cost = %d)\n",
           prefix, s->posx, s->posy, dirchars[s->dir], s->dirsteps, s->cost);
}

struct state *nextState(struct state *current, struct state *next) {
    assert(current != NULL);
    static int maxxy = 0;
    static int state_count;
    static int next_index;
#define MAX_NEW_STATES 10
    static struct state *next_states[MAX_NEW_STATES] = {0};

    int debug_save = debug;
    debug = 1;
    if (debug && (current->posx + current->posy) > maxxy) {
        maxxy = current->posx + current->posy;
        printf("new maxxy = %d at (%d, %d, %c, %d)\n", maxxy, current->posx, current->posy, dirchars[current->dir], current->dirsteps);
    }
    debug = debug_save;
    if (next == NULL) {
        assert((next_states[next_index] == NULL) && "Not all generated next states are used!!!");
        // generate the possible next states
        // first clear the next_states
        for (unsigned int i = 0; i < array_count(next_states); i++) next_states[i] = NULL;
        state_count = 0; next_index = 0;

        // construct the possible directions
        int nextdirs[3] = { current->dir, go_left[current->dir], go_right[current->dir] };
        for (unsigned int di = 0; di < array_count(nextdirs); di++) {
            if (nextdirs[di] == DIR_UNKNOWN) continue;
            // limit going into a single direction to 10 steps
            if ((nextdirs[di] == current->dir) && (current->dirsteps >= 10)) continue;
            int do_steps = 1;
            if (nextdirs[di] != current->dir) {
                do_steps = 4;
            }

            int x = current->posx + do_steps * dirs[nextdirs[di]].dx;
            int y = current->posy + do_steps * dirs[nextdirs[di]].dy;
            if ((x >= 0) && (x < row_width) && (y >= 0) && (y < row_count)) {
                struct state *new_state = malloc(sizeof(struct state));
                new_state->posx = x; new_state->posy = y;
                new_state->dir = nextdirs[di];
                new_state->dirsteps = (nextdirs[di] == current->dir) ? current->dirsteps + 1 : do_steps;
                int extra_cost = stateCost(current, new_state);
                new_state->cost = current->cost + extra_cost;
                new_state->prev = NULL;
                if (extra_cost == 0) {
                    statePrint("from_state (extra_cost == 0):", current);
                    statePrint("generated (extra_cost == 0):", new_state);
                }
                if (debug) statePrint("generated:", new_state);
                assert(state_count < MAX_NEW_STATES);
                next_states[state_count] = new_state;
                state_count++;
            }
        }
        if (debug) printf("genstates: %d states created\n", state_count);
    }

    return next_states[next_index++];
}

int isEndState(struct state *s) {
    return ((s->posx == (row_width - 1)) && (s->posy == (row_count - 1)) && (s->dirsteps >= 4));
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    printf("The map is %dx%d\n", row_width, row_count);
    // handle the moving
    struct state start_state;
    start_state.posx = 0;
    start_state.posy = 0;
    start_state.dir = DIR_UNKNOWN;
    start_state.dirsteps = 0;
    start_state.cost = 0;
    start_state.prev = NULL;
    struct state end_state;
    end_state.posx = row_width - 1;
    end_state.posy = row_count -1;
    end_state.dir = DIR_UNKNOWN;
    end_state.dirsteps = 0;
    end_state.cost = INT_MAX;
    end_state.prev = NULL;

    // set the hash function to be used by the dictionary
    dictMyHashFunction = stateKeyHash;

    debug = 0;
    struct state *e = dijkstra(&start_state, &end_state, isEndState);
    statePrint("Calculated end state:", e);
    printf("Original cost was %d\n", INT_MAX);

    // print the path
    struct state *p = e;
    while (p != NULL) {
        statePrint("path:", p);
        p = p->prev;
    }

    statePrint("Calculated end state:", e);
    printf("Info: correct answer for the sample data is 94\n");
    printf("Info: correct answer for the sample2 data is 71\n");
    printf("Info: correct answer for the full problem is 822\n");
    return EXIT_SUCCESS;
}

