
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
int debug = 0; // 0 = no debug output, 1 = some extra debug output
#define MAX_LINES 200
char *map_lines[MAX_LINES];
unsigned int line_width = 0;
int line_count = 0;

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char line[LINE_LENGTH];
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        if (strlen(line) != 0) {
            // put the data in the map
            if (line_width == 0) {
                line_width = strlen(line);
            } else {
                assert((line_width == strlen(line)) && "map lines must be the same size");
            }
            map_lines[line_count] = malloc(line_width * sizeof(char) + 1);
            strcpy(map_lines[line_count], line);
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

void print_map(char **lines, int count) {
    for (int i = 0; i < count; i++) {
         printf("%s\n", lines[i]);
    }
}

int is_free(char **lines, int width, int height, int x, int y) {
    int free = 1;
    if ((y < 0) || (x < 0) || (y >= height) || (x >= width)) {
        free = 0;
    } else {
        free = (lines[y][x] == '.');
    }
    return free;
}

int move_rocks(char **lines, int width, int count, int dx, int dy) {
    int moved = 0;
    for (int y = 0; y < count; y++) {
        for (int x = 0; x < width; x++) {
            // try to move the rocks
            if ((lines[y][x] == 'O') && is_free(lines, line_width, line_count, x + dx, y + dy)) {
                lines[y][x] = '.';
                lines[y - 1][x] = 'O';
                moved++;
            }
        }
    }
    return moved;
}

long calculate_load(char **lines, int width, int height) {
    long load = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (lines[y][x] == 'O') {
                load += (height - y);
            }
        }
        if (debug) printf("Load for line %i is %ld\n", y+1, load);
    }

    return load;
}
int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // move the rock north
    if (debug) print_map(map_lines, line_count);
    while (move_rocks(map_lines, line_width, line_count, 0, -1) != 0);
    if (debug) printf("====================================\n");
    if (debug) print_map(map_lines, line_count);

    long load = calculate_load(map_lines, line_width, line_count);
    printf("Total load is %ld\n", load);

    return EXIT_SUCCESS;
}

