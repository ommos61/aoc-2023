
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
struct galaxy {
    int x;
    int y;
};
#define MAX_GALAXIES 2000
struct galaxy galaxies[MAX_GALAXIES];
int galaxy_count = 0;
int row_count = 0;
int col_count = 0;

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
            if (col_count == 0) {
                col_count = strlen(line);
            } else {
                // all rows should have equal length
                assert((unsigned)col_count == strlen(line));
            }
            // get the location of the galaxies
            for (int i = 0; i < col_count; i++) {
                if (line[i] == '#') {
                    galaxies[galaxy_count].x = i;
                    galaxies[galaxy_count].y = line_count;
                    assert(galaxy_count < MAX_GALAXIES);
                    galaxy_count++;
                } else {
                    assert(line[i] == '.');
                }
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

#define MAX_EMPTY 200
int rows_empty[MAX_EMPTY], cols_empty[MAX_EMPTY];
int row_empty_count = 0, col_empty_count = 0;
void determine_empties(void) {
    // determine the empty rows
    for (int y = 0; y < row_count; y++) {
        int empty = 1; // true
        for (int x = 0; (x < col_count) && empty; x++) {
            for (int i = 0; i < galaxy_count; i++) {
                if ((galaxies[i].y == y) && (galaxies[i].x == x)) {
                    //printf("row %d is not empty, galaxy at %d\n", y, x);
                    empty = 0;
                    break;
                }
            }
        }
        if (empty) {
            rows_empty[row_empty_count] = y;
            row_empty_count++;
        }
    }
    // determine the empty cols
    for (int x = 0; x < col_count; x++) {
        int empty = 1; // true
        for (int y = 0; (y < row_count) && empty; y++) {
            for (int i = 0; i < galaxy_count; i++) {
                if ((galaxies[i].x == x) && (galaxies[i].y == y)) {
                    //printf("col %d is not empty, galaxy at %d\n", x, y);
                    empty = 0;
                    break;
                }
            }
        }
        if (empty) {
            cols_empty[col_empty_count] = x;
            col_empty_count++;
        }
    }
}

int count_empty_cols(int x1, int x2) {
    int count = 0;
    for (int i = 0; i < col_empty_count; i++) {
         if ((cols_empty[i] > MIN(x1, x2)) && (cols_empty[i] < MAX(x1, x2))) {
            count++;
         }
    }
    return count;
}

int count_empty_rows(int y1, int y2) {
    int count = 0;
    for (int i = 0; i < row_empty_count; i++) {
         if ((rows_empty[i] > MIN(y1, y2)) && (rows_empty[i] < MAX(y1, y2))) {
            count++;
         }
    }
    return count;
}

long get_x_distance(int x1, int x2) {
    return abs(x2 - x1) + count_empty_cols(x1, x2);
}

long get_y_distance(int y1, int y2) {
    return abs(y2 - y1) + count_empty_rows(y1, y2);
}

long get_distance(struct galaxy a, struct galaxy b) {
    return get_x_distance(a.x, b.x) + get_y_distance(a.y, b.y);
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // determine the free rows and columns
    printf("There are %d galaxies.\n", galaxy_count);
    printf("Galaxy %d: (%d, %d)\n", 0, galaxies[0].x, galaxies[0].y);
    determine_empties();
    printf("Empty rows: %d\n", row_empty_count);
    printf("Empty cols: %d\n", col_empty_count);

    // determine the dirtances between the galaxies
    long sum = 0;
    for (int i = 0; i < galaxy_count; i++) {
        for (int j = i + 1; j < galaxy_count; j++) {
            long distance = get_distance(galaxies[i], galaxies[j]);
            if (debug) printf("distance between %d and %d is %ld\n", i+1, j+1, distance);
            sum += distance;
        }
    }
    printf("Sum of the distances is %ld\n", sum);

    return EXIT_SUCCESS;
}

