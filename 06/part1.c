
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
#define MAX_RACES 20
struct race {
    long duration;
    long distance;
};
struct race races[MAX_RACES];
int race_count = 0;

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
            // handle the race stats
            if (strncmp(line, "Time:", strlen("Time:")) == 0) {
                char *p = strchr(line, ':') + 1;
                // skip spaces
                while (isspace(*p)) p++;
                while (*p) {
                    long value = 0;
                    sscanf(p, "%ld", &value);
                    races[race_count].duration = value;
                    race_count++;
                    // skip digits and spaces
                    while (isdigit(*p)) p++;
                    while (isspace(*p)) p++;
                }
            } else if (strncmp(line, "Distance:", strlen("Distance:")) == 0) {
                char *p = strchr(line, ':') + 1;
                // skip spaces
                while (isspace(*p)) p++;
                int index = 0;
                while (*p) {
                    long value = 0;
                    sscanf(p, "%ld", &value);
                    races[index].distance = value;
                    index++;
                    // skip digits and spaces
                    while (isdigit(*p)) p++;
                    while (isspace(*p)) p++;
                }
                assert(index == race_count);
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

int get_first_win(int duration, int distance) {
    int push = 0;
    int win = 0; // false
    while ((push < duration) && !win) {
        int speed = push;
        int travel = (duration - push) * speed;
        if (travel > distance) {
            win = 1; // true
        } else {
            push++;
        }
    }
    return push;
}

int get_last_win(int duration, int distance) {
    int push = duration;
    int win = 0; // false
    while ((push > 0) && !win) {
        int speed = push;
        int travel = (duration - push) * speed;
        if (travel > distance) {
            win = 1; // true
        } else {
            push--;
        }
    }
    return push;
}
int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // loop through the races
    int result = 1;
    for (int i = 0; i < race_count; i++) {
        if (debug) printf("Time %ld: %ld distance\n", races[i].duration, races[i].distance);
        int first_win = get_first_win(races[i].duration, races[i].distance);
        if (debug) printf("  first win: %d\n", first_win);
        int last_win = get_last_win(races[i].duration, races[i].distance);
        if (debug) printf("  last win: %d\n", last_win);
        int win_count = last_win - first_win + 1;
        if (debug) printf("  ways to win: %d\n", win_count);
        result *= win_count;
    }
    printf("The result is %d\n", result);

    return EXIT_SUCCESS;
}

