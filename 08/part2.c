
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 1; // 0 = no debug output, 1 = some extra debug output
char *directions = NULL;
struct map_node {
    char *id;
    char *left;
    char *right;
    int left_index;
    int right_index;
};
#define MAX_NODES 1000
struct map_node nodes[MAX_NODES];
int node_count = 0;
int starts[MAX_NODES];
int ends[MAX_NODES];
int start_count = 0;
int end_count = 0;

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

        if (strlen(line) == 0) {
            // skip empty lines
        } else if (strlen(line) != 0) {
            // parse the directions and map nodes
            if (directions == NULL) {
                directions = malloc(strlen(line) + 1);
                strcpy(directions, line);
            } else {
                char *id = malloc(4);
                char *left = malloc(4);
                char *right = malloc(4);
                sscanf(line, "%[1-9A-Z] = (%[1-9A-Z], %[1-9A-Z])", id, left, right);
                nodes[node_count].id = id;
                nodes[node_count].left = left;
                nodes[node_count].right = right;
                nodes[node_count].left_index = -1;
                nodes[node_count].right_index = -1;
                if (id[2] == 'A') {
                    starts[start_count] = node_count;
                    start_count++;
                } else if (id[2] == 'Z') {
                    ends[end_count] = node_count;
                    end_count++;
                }
                node_count++;
            }
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

int find_node(const char *id) {
    for (int i = 0; i < node_count; i++) {
        if (strcmp(nodes[i].id, id) == 0) {
            return i;
        }
    }
    printf("Node '%s' not found on the map\n", id);
    exit(1);
    return -1;
}

int find_node_left(int current) {
    int index = nodes[current].left_index;
    if (index != -1) {
        return index;
    } else {
        int index = find_node(nodes[current].left);
        nodes[current].left_index = index;
        return index;
    }
}

int find_node_right(int current) {
    int index = nodes[current].right_index;
    if (index != -1) {
        return index;
    } else {
        int index = find_node(nodes[current].right);
        nodes[current].right_index = index;
        return index;
    }
}

void print_nodes(void) {
    for (int i = 0; i < node_count; i++) {
         printf("%s = (%s, %s)\n", nodes[i].id, nodes[i].left, nodes[i].right);
    }
}

void print_current_locations(int *curs, int count) {
    for (int i = 0; i < count; i++) {
         printf("%s ", nodes[curs[i]].id);
    }
    printf("\n");
}

void print_info(int *current_nodes, long *steps) {
    printf("Ends: ");
    print_current_locations(current_nodes, start_count);
    printf("Steps: ");
    for (int i = 0; i < start_count; i++) {
        printf("%ld ", steps[i]);
    }
    printf("\n");
}

long find_end_node(int *current_node, int *dir_index) {
    long steps = 0;

    do {
        // actually move
        char dir = directions[*dir_index];
        if (dir == 'L') {
                *current_node = find_node_left(*current_node);
        } else if (dir == 'R') {
                *current_node = find_node_right(*current_node);
        }
        *dir_index += 1;
        if (directions[*dir_index] == '\000') {
            *dir_index = 0;
        }
        steps++;
    } while (nodes[*current_node].id[2] != 'Z');

    return steps;
}

// use cached info for moving from endnode to endnode
long **steps_cache;
int **target_cache;
long next_move(int *current_node, int *dir_index) {
    // first find the correct row in the cache
    int end_index = 0;
    while (*current_node != ends[end_index]) end_index++;

    // now see if we have a cache hit
    if (steps_cache[end_index][*dir_index] != 0) {
        *current_node = target_cache[end_index][*dir_index];
        long steps = steps_cache[end_index][*dir_index];
        *dir_index = (*dir_index + steps) % strlen(directions);
        return steps;
    } else {
        // we must actually walk
        int current = *current_node;
        int dir = *dir_index;
        long steps = find_end_node(&current, &dir);
        // and store the result in the cache
        steps_cache[end_index][*dir_index] = steps;
        target_cache[end_index][*dir_index] = current;

        // update infor for caller
        *current_node = current;
        *dir_index = (*dir_index + steps) % strlen(directions);

        return steps;
    }
}

int get_lowest_end(long *steps, int count) {
    int equal = 1; // true
    int lowest = 0;
    for (int i = 1; i < count; i++) {
        if (steps[i] != steps[i-1]) equal = 0; // false
//        printf("Comparing steps [%d] = %ld and [lowest=%d] = %ld\n", i, steps[i], lowest, steps[lowest]);
        if (steps[i] < steps[lowest]) {
//            printf("setting lowest index to %d\n", i);
            lowest = i;
        }
    }

    return (equal) ? -1 : lowest;
    //return lowest;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // walk from start to end
    assert(start_count == end_count);
    printf("There are %d starting nodes.\n", start_count);

    // copy the starting node indices
    printf("Starts: ");
    print_current_locations(starts, start_count);
    //print_nodes();

    // now do the initial walk to an end node for every ghost
    long *steps = malloc(start_count * sizeof(long));
    int *current_nodes = malloc(start_count * sizeof(int));
    int *current_dirs = malloc(start_count * sizeof(int));
    for (int i = 0; i < start_count; i++) {
        current_nodes[i] = starts[i];
        current_dirs[i] = 0;
        steps[i] = find_end_node(current_nodes + i, current_dirs + i);
    }
    print_info(current_nodes, steps);

    // we need to setup a cache for moving from endnode to endnode, from different initial direction indeces
    steps_cache = malloc(end_count * sizeof(long*));
    target_cache = malloc(end_count * sizeof(int*));
    for (int i = 0; i < end_count; i++) {
        steps_cache[i] = malloc(strlen(directions) * sizeof(long));
        target_cache[i] = malloc(strlen(directions) * sizeof(int));
        for (unsigned int j = 0; j < strlen(directions); j++) steps_cache[i][j] = 0L;
    }
    printf("Caches created.\n");

    // now we move the lowest until all step counts ae equal
    int lowest_end = get_lowest_end(steps, start_count);
    while (lowest_end != -1) {
//        printf("Lowest step count is %ld(%d)\n", steps[lowest_end], lowest_end);
        steps[lowest_end] += next_move(current_nodes + lowest_end, current_dirs + lowest_end);
//        print_info(current_nodes, steps);
        lowest_end = get_lowest_end(steps, start_count);
    }
    printf("Final step count is %ld\n", steps[0]);

    return EXIT_SUCCESS;
}

