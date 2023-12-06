
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

long get_first_win(long duration, long distance) {
    long push = 0;
    int win = 0; // false
    while ((push < duration) && !win) {
        long speed = push;
        long travel = (duration - push) * speed;
        if (travel > distance) {
            win = 1; // true
        } else {
            push++;
        }
    }
    return push;
}

long get_last_win(long duration, long distance) {
    long push = duration;
    int win = 0; // false
    while ((push > 0) && !win) {
        long speed = push;
        long travel = (duration - push) * speed;
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

    // construct the race info
    long duration = races[0].duration;
    long distance = races[0].distance;
    for (int i = 1; i < race_count; i++) {
        char buf[1000];
        sprintf(buf, "%ld", races[i].duration);
        for (unsigned int j = 0; j < strlen(buf); j++) duration *= 10;
        duration += races[i].duration;
        sprintf(buf, "%ld", races[i].distance);
        for (unsigned int j = 0; j < strlen(buf); j++) distance *= 10;
        distance += races[i].distance;
    }
    printf("Time %ld: %ld distance\n", duration, distance);

    long first_win = get_first_win(duration, distance);
    if (debug) printf("  first win: %ld\n", first_win);

    long last_win = get_last_win(duration, distance);
    if (debug) printf("  last win: %ld\n", last_win);

    long win_count = last_win - first_win + 1;
    printf("The result is %ld\n", win_count);

    return EXIT_SUCCESS;
}

