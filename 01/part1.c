
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
int debug = 1; // 0 = no debug output, 1 = some extra debug output
#define MAX_LINES 1024
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
            // Put the line in the global list
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

int get_calibration_value(char * line) {
    int first, last;
    char *p = line;
    while (*p && (! isdigit(*p))) p++;
    first = *p - '0';
    while (*p) {
        if (isdigit(*p)) last = *p - '0';
        p++;
    }
    //printf("str = \'%s\", first = %d, last = %d\n", line, first, last);
    return first * 10 + last;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // Calculate to sum of the calibration value
    long sum = 0;
    for (int i = 0; i < line_count; i++) {
        sum += get_calibration_value(lines[i]);
    }
    printf("Sum of calibration values is %ld\n", sum);

    return EXIT_SUCCESS;
}

