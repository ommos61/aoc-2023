
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
struct grab {
    int red; int green; int blue;
    struct grab *next;
};
struct game {
    int id;
    struct grab *grabs;
};
#define MAX_GAMES 500
struct game games[MAX_GAMES];
int game_count = 0;

struct grab *create_grab(void) {
    struct grab *result = malloc(sizeof(struct grab));
    result->red = 0;
    result->green = 0;
    result->blue = 0;
    result->next = NULL;

    return result;
}

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

        int value = 0;
        if (sscanf(line, "Game %d:", &value) == 1) {
            // store the new game
            games[game_count].id = value;
            games[game_count].grabs = NULL;
            char *p = strchr(line, ':') + 1;
            struct grab *current_grab = create_grab();
            while (*p) {
                switch (*p) {
                case ' ':
                case ',':
                    p++;
                    break;
                case ';':
                    current_grab->next = games[game_count].grabs;
                    games[game_count].grabs = current_grab;
                    current_grab = create_grab();
                    p++;
                    break;
                case 'r':
                    current_grab->red = value;
                    while (isalpha(*p)) p++;
                    break;
                case 'g':
                    current_grab->green = value;
                    while (isalpha(*p)) p++;
                    break;
                case 'b':
                    current_grab->blue = value;
                    while (isalpha(*p)) p++;
                    break;
                default:
                    assert(isdigit(*p));
                    sscanf(p, "%d", &value);
                    while (*p && isdigit(*p)) p++;
                }
            }
            current_grab->next = games[game_count].grabs;
            games[game_count].grabs = current_grab;
            game_count++;
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

void print_grab(struct grab *gr) {
    printf("red: %d, green: %d, blue: %d", gr->red, gr->green, gr->blue);
}

int game_valid(struct game g, int red, int green, int blue) {
    int valid = 1;

    struct grab *gr = g.grabs;
    while (valid && (gr != NULL)) {
        if (debug) { printf("  "); print_grab(gr); }
        if ((gr->red > red) || (gr->green > green) || (gr->blue > blue)) {
            if (debug) printf("  INVALID");
            valid = 0;
        }
        if (debug) printf("\n");
        gr = gr->next;
    }

    return valid;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // calculate the sum of possible game ids
    int sum = 0;
    for (int i = 0; i < game_count; i++) {
        if (debug) printf("Game %d:\n", games[i].id);
        if (game_valid(games[i], 12, 13, 14)) {
            sum += games[i].id;
        }
    }
    printf("Sum of game ids is %d.\n", sum);

    return EXIT_SUCCESS;
}

