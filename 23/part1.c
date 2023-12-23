
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
int map_width = 0, map_height = 0;
#define get_map(x,y) (map_rows[(y)][(x)])

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
            // determine the map width
            if (map_width == 0) {
                map_width = strlen(line);
            } else {
                assert((strlen(line) == (unsigned)map_width) && "map rows must have equal width");
            }
            // store the map data
            map_rows[map_height] = malloc(map_width + 1);
            strcpy(map_rows[map_height], line);
            map_height++;
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

char *visited = NULL;
#define get_visited(x,y) (*(visited + (y) * map_width + (x)))
#define set_visited(x,y) (*(visited + (y) * map_width + (x)) = 1)
#define unset_visited(x,y) (*(visited + (y) * map_width + (x)) = 0)

int max_path = 0;
struct dir {
    int dx; int dy;
} dirs[] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
char *dir_chars = ">v<^";
#define slope_allowed(c,dc) ((c) == (dc))

void walk_path(int x, int y, int length) {
    if (debug) printf("At (%d, %d)\n", x, y);
    // found the exit
    if ((x == map_width - 2) && (y == map_height - 1)) {
        if (debug) printf("Found the exit with path length %d\n", length);
        max_path = MAX(max_path, length);
        return;
    }

    // try walking further
    // NOTE: a bounds chaeck is not needed, because the borders are always forrest,
    //       except for the entrance and exit
    set_visited(x, y);
    for (unsigned int d = 0; d < array_count(dirs); d++) {
        int newx = x + dirs[d].dx;
        int newy = y + dirs[d].dy;
        if (newy < 0) continue;
        char c = get_map(newx, newy);
        if (debug) printf("To (%d, %d) '%c'\n", newx, newy, c);
        if (((c == '.') || slope_allowed(c, dir_chars[d])) && !get_visited(newx, newy)) {
            walk_path(newx, newy, length + 1);
        }
    }
    unset_visited(x, y);
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // print some info
    printf("Map is %dx%d\n", map_width, map_height);

    // determine the max path by walking all paths recursively
    if (visited == NULL) {
        visited = malloc(map_width * map_height);
        memset(visited, 0, map_width * map_height);
    }
    walk_path(1, 0, 0);
    printf("Max path is %d tiles.\n", max_path);

    return EXIT_SUCCESS;
}

