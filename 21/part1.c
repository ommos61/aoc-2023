
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
#define MAX_ROWS 200
char *map_rows[MAX_ROWS];
int map_width = 0;
int row_count = 0;
int start_row = 0, start_col = 0;

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
                assert(((unsigned)map_width == strlen(line)) && "rows should be equal size");
            }

            // store the map rows
            map_rows[row_count] = malloc(map_width + 1);
            strcpy(map_rows[row_count], line);

            char * start = strchr(line, 'S');
            if (start != NULL) {
                start_row = row_count;
                start_col = start - line;
            }

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

struct pos {
    int x, y;
    struct pos *next;
};
struct dir {
    int dx, dy;
} dirs[] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };

int is_present(struct pos *positions, int x, int y) {
    int present = 0; // false
    struct pos *p = positions;
    while (p != NULL) {
        if ((p->x == x) && (p->y == y)) {
            present = 1; // true
            break;
        }
        p = p->next;
    }
    return present;
}

struct pos *do_one_step(struct pos *positions) {
    struct pos *p = positions;
    struct pos *r = NULL;

    while (p != NULL) {
        for (unsigned int d = 0; d < array_count(dirs); d++) {
            int newx = p->x + dirs[d].dx;
            int newy = p->y + dirs[d].dy;
            if ((newx >= 0) && (newx < map_width) && (newy >= 0) && (newy < row_count)) {
                char c = map_rows[newy][newx];
                if ((c != '#') && !is_present(r, newx, newy)) {
                    struct pos *np = malloc(sizeof(struct pos));
                    np->x = newx; np->y = newy;
                    np->next = r;
                    r = np;
                }
            }
        }
        struct pos *temp = p;
        p = p->next;
        free(temp);
    }

    return r;
}

int count_positions(struct pos *positions) {
    int count = 0;
    struct pos *p = positions;
    while (p != NULL) {
        count++;
        p = p->next;
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

    // do the stepping
    printf("Start is at (%d, %d)\n", start_col, start_row);
    struct pos *positions = malloc(sizeof(struct pos));
    positions->x = start_col; positions->y = start_row; positions->next = NULL;

    int max_steps = 64;
    for (int step = 1; step <= max_steps; step++) {
        struct pos *new_positions = do_one_step(positions);
        if (step == max_steps) {
            printf("Position count after %d steps: %d\n", step, count_positions(new_positions));
        }
        positions = new_positions;
    }

    return EXIT_SUCCESS;
}

