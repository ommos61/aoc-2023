
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
struct value {
    int val;
    struct value *next;
};
#define MAX_HISTORIES 500
#define MAX_VALUES 25
int histories[MAX_HISTORIES][MAX_VALUES];
int history_count = 0;
int value_count = 0;

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
            // parse the data
            int index = 0;
            char *p = line;
            while (*p) {
                int number = 0;
                sscanf(p, "%d", &number);
                histories[history_count][index] = number;
                index++;
                // skip the number
                while (isdigit(*p) || (*p == '-')) p++;
                // skip spaces
                while (isspace(*p)) p++;
            }
            history_count++;
            if (value_count == 0) {
                value_count = index;
            } else {
                assert(value_count == index);
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

void print_history(int *hist, int count) {
    printf("values: ");
    for (int i = 0; i < count; i++) {
        printf("%d ", hist[i]);
    }
    printf("\n");
}

long determine_prev(int *vals, int count) {
    // keep record if all results are zero
    int is_zero = 1; // true
    int *new_vals = malloc((count - 1) * sizeof(int));
//    printf("new_vals: ");
    for (int i = 0; i < count - 1; i++) {
        new_vals[i] = vals[i + 1] - vals[i];
//        printf("%d ", new_vals[i]);
        if (new_vals[i] != 0) is_zero = 0; // false
    }
//    printf("\n");

    long prev = 0;
    long first = new_vals[0];
    if (!is_zero) {
        prev = first - determine_prev(new_vals, count - 1);
        free(new_vals);
    }
//    printf("last is %ld\n", last);
//    printf("prediction is %ld\n", next);
    return prev;
}

long get_prev_prediction(int *history, int count) {
//    printf("Element count: %d\n", count);
//    print_history(history, count);

    // now recursively determine the next one for the history
    long first = history[0];
    long prev = first - determine_prev(history, count); 
    if (debug) printf("prediction is %ld\n", prev);

    return prev;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // determine the next for each history line and sum them
    long sum = 0;
    for (int i = 0; i < history_count; i++) {
        long prediction = get_prev_prediction(histories[i], value_count);
        sum += prediction;
    }
    printf("Sum of predicted values is %ld\n", sum);

    return EXIT_SUCCESS;
}

