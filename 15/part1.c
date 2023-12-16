
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 50000
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 1; // 0 = no debug output, 1 = some extra debug output
char *sequence = NULL;

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
            // store the data
            if (sequence == NULL) {
                sequence = malloc(strlen(line) + 1);
                strcpy(sequence, line);
            } else {
                printf("Only one line of input is supported.\n");
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

int calculate_hash(char *str, int len) {
    int hash = 0;
    for (int i = 0; i < len; i++) {
        hash += str[i];
        hash *= 17;
        hash %= 256;
    }

    return hash;
}
int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // test the hash function on the test string
    #define TEST_STRING "HASH"
    int hash = calculate_hash(TEST_STRING, strlen(TEST_STRING));
    printf("The test string \"%s\" has hash %d\n", TEST_STRING, hash);

    // calculate the sum of hashes on the input
    int sum = 0;
    char *p = sequence;
    while (*p != '\000') {
        char *separator = strchr(p, ',');
        if (separator == NULL) {
            sum += calculate_hash(p, strlen(p));
            p += strlen(p);
        } else {
            int len = separator - p;
            sum += calculate_hash(p, len);
            p += len + 1;
        }
    }
    printf("Sum of the hashes is %d\n", sum);

    return EXIT_SUCCESS;
}

