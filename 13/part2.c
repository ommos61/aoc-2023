
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
#define MAX_LINES 2000
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
            // store the data
            lines[line_count] = malloc(strlen(line) + 1);
            strcpy(lines[line_count], line);
            // make sure the next one is empty, will be overwritten when not empty
            lines[line_count + 1] = NULL;
        } else if (strlen(line) == 0) {
            lines[line_count] = NULL;;
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

void print_block(int line_index) {
    int i = 0;
    printf("    123456789ABCDEF0\n");
    while (lines[line_index + i] != NULL) {
        printf("%2d| %s \n", i+1, lines[line_index + i]);
        i++;
    }
}

int get_width(int line_index) {
    return strlen(lines[line_index]);
}

int get_height(int line_index) {
    int height = 0;
    while (lines[line_index + height] != NULL) height++;
    return height;
}

void str_reverse(char *str) {
    for (int i = 0, j = strlen(str) - 1; i <= j; i++, j--) {
        char c = str[i];
        str[i] = str[j];
        str[j] = c;
    }
}

int count_diffs(char *a, char *b) {
    int diffs = 0;
    for (unsigned int i = 0; i < strlen(a); i++) {
         if (a[i] != b[i]) diffs++;
    }
    return diffs;
}

#define MAX_LINE 100
int get_sym_line(int blk_index, int smudges) {
    int width = get_width(blk_index);
    int height = get_height(blk_index);

    if (debug) printf("Block: %d x %d\n", width, height);

    int sym_line = 0;
    // check for vertical symmetry
    for (int x = 0; x < width; x++) {
        int sym_width = MIN(1 + x, width - x - 1);
        if (debug) printf("x = %d, sym_width = %d\n", x, sym_width);

        if (sym_width > 0) {
            int diff_count = 0;
            // check each row
            for (int y = 0; y < height; y++) {
                char left[MAX_LINE], right[MAX_LINE];
                strncpy(left, lines[blk_index + y] + x + 1 - sym_width, sym_width);
                left[sym_width] = '\000';
                strncpy(right, lines[blk_index + y] + x  + 1, sym_width);
                right[sym_width] = '\000';
                str_reverse(right);
                if (debug) printf("Comparing '%s' and '%s'\n", left, right);
                diff_count += count_diffs(left, right);
            }
            if (diff_count == smudges) {
                sym_line = x + 1;
                if (debug) printf("Found at %d\n", sym_line);
            }
        }
    }

    // check for horizontal symmetry
    if (sym_line == 0) {
        for (int y = 0; y < height; y++) {
            int sym_width = MIN(1 + y, height - y - 1);
            if (debug) printf("y = %d, sym_width = %d\n", y, sym_width);

            if (sym_width > 0) {
                int diff_count = 0;
                // check each column
                for (int x = 0; x < width; x++) {
                    char top[MAX_LINE], bottom[MAX_LINE];
                    for (int i = 0; i < sym_width; i++) top[i] = lines[blk_index + y + 1 - sym_width + i][x];
                    top[sym_width] = '\000';
                    for (int i = 0; i < sym_width; i++) bottom[i] = lines[blk_index + y + 1 + i][x];
                    bottom[sym_width] = '\000';
                    str_reverse(bottom);
                    if (debug) printf("Comparing '%s' and '%s'\n", top, bottom);
                    diff_count += count_diffs(top, bottom);
                }
                if (diff_count == smudges) {
                    sym_line = (y + 1) * 100;
                    if (debug) printf("Found at %d\n", sym_line);
                }
            }

        }
    }

    //assert(sym_line != 0);
    return sym_line;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // check each block and determine its value
    long sum = 0;
    long old_sum = 0;
    int line_index = 0;
    while (line_index < line_count) {
        // determine where the mirror line is, without smudges
        int orig_line_val = get_sym_line(line_index, 0);
        if (orig_line_val == 0) {
            printf("ORIGINAL: NO NEW SYM LINE FOUND!!!\n");
        }
        old_sum += orig_line_val;
        int width = get_width(line_index);
        int height = get_height(line_index);
        if (debug) printf("Block: %d x %d\n", width, height);

        // determine where the mirror line is, for one smudge
        int line_val = line_val = get_sym_line(line_index, 1);
        sum += line_val;
        if (line_val == 0) {
            printf("NO NEW SYM LINE FOUND!!!\n");
            printf("Symline = %d, orig = %d\n", line_val, orig_line_val);
            print_block(line_index);
        }

        // advance to next block
        while (lines[line_index] != NULL) line_index++;
        if (line_index < line_count) line_index++;
    }
    printf("Original result is %ld\n", old_sum);
    printf("Smudged result is %ld\n", sum);

    return EXIT_SUCCESS;
}

