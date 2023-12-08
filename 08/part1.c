
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
};
#define MAX_NODES 1000
struct map_node nodes[MAX_NODES];
int node_count = 0;
int start = -1;
int end = -1;

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
                sscanf(line, "%[A-Z] = (%[A-Z], %[A-Z])", id, left, right);
                nodes[node_count].id = id;
                nodes[node_count].left = left;
                nodes[node_count].right = right;
                if (strcmp(id, "AAA") == 0) {
                    assert(start == -1);
                    start = node_count;
                } else if (strcmp(id, "ZZZ") == 0) {
                    assert(end == -1);
                    end = node_count;
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

void print_nodes(void) {
    for (int i = 0; i < node_count; i++) {
         printf("%s = (%s, %s)\n", nodes[i].id, nodes[i].left, nodes[i].right);
    }
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // walk from start to end
    assert(start != -1);
    assert(end != -1);
    long steps = 0;
    int current = start;
    char *direction = directions;
    while (current != end) {
        // TODO: actually move
        if (*direction == 'L') {
            current = find_node(nodes[current].left);
        } else if (*direction == 'R') {
            current = find_node(nodes[current].right);
        }
        direction++;
        if (*direction == '\000') {
            direction = directions;
        }
        steps++;
    }
    printf("Amount of steps is %ld\n", steps);

    return EXIT_SUCCESS;
}

