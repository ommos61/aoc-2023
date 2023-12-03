
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
#define MAX_LINES 500
char *lines[MAX_LINES];
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
            // Store the data
            lines[line_count] = malloc(strlen(line) + 1);
            strcpy(lines[line_count], line);
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

int is_symbol_char(int row, int col) {
    // handle the edges
    if ((row < 0) || (row == line_count)) return 0;
    if ((col < 0) || (lines[row][col] == 0)) return 0;

    char c = lines[row][col];
    if (isdigit(c) || (c == '.')) return 0;
    else {
        if (debug) printf("Found: %c\n", c);
        return 1;
    }
}

int is_symbol_char_next(int row, int col) {
    // handle up and down
    if (is_symbol_char(row - 1, col) || is_symbol_char(row + 1, col)) return 1;
    // handle left and right
    if (is_symbol_char(row, col - 1) || is_symbol_char(row, col + 1)) return 1;
    // handle diagonale left up and left down
    if (is_symbol_char(row - 1, col - 1) || is_symbol_char(row + 1, col - 1)) return 1;
    // handle diagonale right up and right down
    if (is_symbol_char(row - 1, col + 1) || is_symbol_char(row + 1, col + 1)) return 1;
    return 0;
}

int is_symbol_next(char *str, int row, int col) {
    int num_index = 0;
    while (isdigit(str[num_index])) {
        if (debug) printf("checking %c at (%d, %d)\n", str[num_index], row, col + num_index);
        if (is_symbol_char_next(row, col + num_index)) return 1;
        num_index++;
    }
    return 0;
}

long get_partnumber(char *str, int row, int col) {
    if (isdigit(*str)) {
        long number;
        sscanf(str, "%ld", &number);
        if (debug) printf("partnumber = %ld\n", number);
        if (is_symbol_next(str, row, col)) {
            return number;
        }
    }

    return 0L;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // Determine the part numbers
    long sum = 0;
    for (int i = 0; i < line_count; i++) {
        char *line = lines[i];
        char *p = line;
        while (*p) {
            if (isdigit(*p)) {
                long partnumber = get_partnumber(p, i, p - line);
                if (partnumber != 0L) {
                    if (debug) printf("FOUND partnumber = %ld\n", partnumber);
                    sum += partnumber;
                }
                // skip all digits that are part of the number
                while (isdigit(*p)) p++;
            } else {
                p++;
            }
        }
    }
    printf("Sum of partnumbers is %ld\n", sum);

    return EXIT_SUCCESS;
}

