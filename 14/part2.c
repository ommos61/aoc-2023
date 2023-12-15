
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
                lines[y + dy][x + dx] = 'O';
                moved++;
            }
        }
    }
    return moved;
}

struct dir {
    int dx; int dy;
} dirs[] = { {0, -1}, {-1, 0}, {0, 1}, {1, 0}}; // North, West, South, East
void execute_cycle(char **lines, int width, int height) {
    for (unsigned int d = 0; d < array_count(dirs); d++) {
        while (move_rocks(lines, width, height, dirs[d].dx, dirs[d].dy) != 0) ;
    }
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

long find_repeat(long *loads, int load_count, int cycle_target) {
    (void) cycle_target;

    int repeat_size = 2;
    while (load_count > repeat_size * 2) {
        int is_repeating = 1; // true
        for (int i = 0; i < repeat_size; i++) {
            int index = load_count - 1 - i;
            int index2 = index - repeat_size;
            if (loads[index] != loads[index2]) {
                is_repeating = 0; // false
                break;
            }
        }
        if (is_repeating) {
            // found a loop
            printf("Found a repeat with size %d cycles at cycle %d\n", repeat_size, load_count);
            printf("repeat: ");
            for (int i = load_count - 2 * repeat_size - 1; i < load_count; i++) {
                printf("%ld ", loads[i]);
            }
            printf("\n");
            int remaining_repeats = (cycle_target - load_count) / repeat_size;
            int pos_after_repeats = load_count + remaining_repeats * repeat_size;
            int last_pos = (load_count - repeat_size) + (cycle_target - pos_after_repeats) - 1;
            return loads[last_pos];
        }

        // try next repeat size
        repeat_size++;
    }
    return 0;
}

#define CYCLE_TARGET 1000000000
#define MAX_HISTORY 10000
int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // Some debug printing to check sizes of long and int
    //printf("INT_MAX  = %d\nLONG_MAX = %ld\n           1000000000\n", INT_MAX, LONG_MAX);

    if (debug) print_map(map_lines, line_count);
    // loop until we get a situation we have already seen
    long load_history[MAX_HISTORY];
    int load_count = 0;
    while (load_count < MAX_HISTORY) {
        execute_cycle(map_lines, line_width, line_count);
        load_history[load_count++] = calculate_load(map_lines, line_width, line_count);

        long last_load = find_repeat(load_history, load_count, CYCLE_TARGET);
        if (last_load > 0) {
            // found the solution
            printf("Last cycle load is %ld\n", last_load);
            break;
        }
    }
    if (load_count >= MAX_HISTORY) {
        printf("DIDN'T FIND A LOOP IN THE FIRST %d CYCLES\n", load_count);
    }

    return EXIT_SUCCESS;
}

