
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
#define MAX_ROWS 200
char *map_rows[MAX_ROWS];
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
                assert(((unsigned)row_width == strlen(line)) && "Rows must be equal width!");
            }
            // store the map rows
            map_rows[row_count] = malloc(row_width + 1);
            strcpy(map_rows[row_count], line);
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

#define DIR_RIGHT (0)
#define DIR_DOWN  (1)
#define DIR_LEFT  (2)
#define DIR_UP    (3)
struct dirs {
    int dx; int dy;
} dirs[] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
char dir_chars[] = ">v<^";
char *map_dirs = NULL;
int map_width = 0, map_height = 0;
void init_dirs(int width, int height) {
    if (map_dirs != NULL) free(map_dirs);
    map_dirs = malloc(width * height * sizeof(char));
    memset(map_dirs, 0, width * height * sizeof(char));
    map_width = width;
    map_height = height;
}
#define dir_mask(d) (1<<(d))
#define get_dir(x,y) (*(map_dirs + (x) + (y) * map_width))
#define add_dir(x,y,d) (*(map_dirs + (x) + (y) * map_width) |= (d))
#define get_map(x,y) (map_rows[(y)][(x)])

void print_map(void) {
    printf("Map is %dx%d\n", row_width, row_count);
    for (int y = 0; y < row_count; y++) {
        for (int x = 0; x < row_width; x++) {
            int dir_count = 0;
            char dir_char = ' ';
            int mask = get_dir(x, y);
            for (int d = 0; d < 4; d++) {
                if ((mask & dir_mask(d)) != 0) {
                    dir_count++;
                    dir_char = dir_chars[d];
                }
            }
            if ((dir_count == 0) || (get_map(x, y) != '.')) {
                putchar(get_map(x, y));
            } else if (dir_count == 1) {
                putchar(dir_char);
            } else {
                putchar('0' + dir_count);
            }
        }
        printf("\n");
    }
}

void propagate_light(int x, int y, int dir) {
    // validate that the new coord are sill in the map
    if ((x < 0) || (y < 0) || (x >= row_width) || (y >= row_count)) return;

    // now mark the remember the direction and propagate
    char mask = get_dir(x, y);
    if ((mask & dir_mask(dir)) == 0) {
        // there was no light in that direction yet
        add_dir(x, y, dir_mask(dir));
        switch (get_map(x, y)) {
        case '.':
            propagate_light(x + dirs[dir].dx, y + dirs[dir].dy, dir);
            break;
        case '\\': {
            int newdir = 0;
            switch (dir) {
            case DIR_RIGHT: newdir = DIR_DOWN; break;
            case DIR_DOWN:  newdir = DIR_RIGHT; break;
            case DIR_LEFT:  newdir = DIR_UP; break;
            case DIR_UP:    newdir = DIR_LEFT; break;
            }
            propagate_light(x + dirs[newdir].dx, y + dirs[newdir].dy, newdir);
            break;
        }
        case '/': {
            int newdir = 0;
            switch (dir) {
            case DIR_RIGHT: newdir = DIR_UP; break;
            case DIR_DOWN:  newdir = DIR_LEFT; break;
            case DIR_LEFT:  newdir = DIR_DOWN; break;
            case DIR_UP:    newdir = DIR_RIGHT; break;
            }
            propagate_light(x + dirs[newdir].dx, y + dirs[newdir].dy, newdir);
            break;
        }
        case '-': {
            if ((dir == DIR_LEFT) || (dir == DIR_RIGHT)) {
                // continue in same direction
                propagate_light(x + dirs[dir].dx, y + dirs[dir].dy, dir);
            } else {
                // split to right and left
                propagate_light(x + dirs[DIR_RIGHT].dx, y + dirs[DIR_RIGHT].dy, DIR_RIGHT);
                propagate_light(x + dirs[DIR_LEFT].dx, y + dirs[DIR_LEFT].dy, DIR_LEFT);
            }
            break;
        }
        case '|': {
            if ((dir == DIR_DOWN) || (dir == DIR_UP)) {
                // continue in same direction
                propagate_light(x + dirs[dir].dx, y + dirs[dir].dy, dir);
            } else {
                // split to down and up
                propagate_light(x + dirs[DIR_DOWN].dx, y + dirs[DIR_DOWN].dy, DIR_DOWN);
                propagate_light(x + dirs[DIR_UP].dx, y + dirs[DIR_UP].dy, DIR_UP);
            }
            break;
        }
        default:
          break;
        }
    } else {
        // ignore, because it seems to already have been added before
    }
}

int count_energized(void) {
    int count = 0;
    for (int y = 0; y < map_height; y++) {
        for (int x = 0; x < map_width; x++) {
            if (get_dir(x, y) != 0) count++;
        }
    }
    return count;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // do the light propagation
    init_dirs(row_width, row_count);
    if (debug) print_map();
    propagate_light(0, 0, DIR_RIGHT);
    if (debug) print_map();
    printf("Energized tiles count is %d\n", count_energized());

    return EXIT_SUCCESS;
}

