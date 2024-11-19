
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
#define DIR_RIGHT   0
#define DIR_DOWN    1
#define DIR_LEFT    2
#define DIR_UP      3
#define DIR_UNKNOWN 4
#define MAX_COMMANDS 1000
struct command {
    int dir;
    int count;
    int color;
};
struct command commands[MAX_COMMANDS];
int command_count = 0;

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
            // parse the command and store it
            int d = DIR_UNKNOWN;
            switch (line[0]) {
            case 'R': d = DIR_RIGHT; break;
            case 'D': d = DIR_DOWN; break;
            case 'L': d = DIR_LEFT; break;
            case 'U': d = DIR_UP; break;
            default: d = DIR_UNKNOWN; break;
            }
            commands[command_count].dir = d;

            int c = 0;
            char *p = line + 2;
            while (isdigit(*p)) {
                c = 10 * c + (*p - '0');
                p++;
            }
            commands[command_count].count = c;
            commands[command_count].color = 0;
            command_count++;
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

char *map;
int map_width, map_height;
#define map_get(x,y) (*(map + (y) * map_width + (x)))
#define map_put(x,y,c) (*(map + (y) * map_width + (x)) = (c))

struct dir { int dx, dy; } dirs[] = { {0, 1}, {1, 0}, {0, -1}, {-1, 0} };
void fill(int x, int y, char c) {
    if ((x >= 0) && (x < map_width) && (y >= 0) && (y < map_height)) {
        if (map_get(x, y) == '.') {
            map_put(x, y, c);
            for (unsigned int d = 0; d < array_count(dirs); d++) {
                int newx = x + dirs[d].dx;
                int newy = y + dirs[d].dy;
                fill(newx, newy, c);
            }
        }
    }
}

void paint_outside(char *map, int width, int height, char c) {
    for (int x = 0; x < width; x++) {
        if (map_get(x, 0) == '.') fill(x, 0, c);
        if (map_get(x, height - 1) == '.') fill(x, height - 1, c);
    }
    for (int y = 0; y < height; y++) {
        if (map_get(0, y) == '.') fill(0, y, c);
        if (map_get(width - 1, y) == '.') fill(width - 1, y, c);
    }
}

void print_map(char *map, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            printf("%c", map_get(x, y));
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // walk the commands and determine the min and max x and y
    int minx = 0, maxx = 0;
    int miny = 0, maxy = 0;
    int curx = 0, cury = 0;
    for (int i = 0; i < command_count; i++) {
        switch (commands[i].dir) {
        case DIR_RIGHT:
            curx += commands[i].count;
            if (curx > maxx) maxx = curx;
            break;
        case DIR_DOWN:
            cury += commands[i].count;
            if (cury > maxy) maxy = cury;
            break;
        case DIR_LEFT:
            curx -= commands[i].count;
            if (curx < minx) minx = curx;
            break;
        case DIR_UP:
            cury -= commands[i].count;
            if (cury < miny) miny = cury;
            break;
        }
    }
    printf("Field is from (%d, %d) to (%d, %d)\n", minx, miny, maxx, maxy);
    map_width = maxx - minx + 1;
    map_height = maxy - miny + 1;
    printf("size = %dx%d\n", map_width, map_height);
    map = malloc(map_width * map_height);
    memset(map, '.', map_width * map_height);

    // now do the actual digging
    int x = -minx, y = -miny;
    map_put(x, y, '#');
    for (int i = 0; i < command_count; i++) {
        switch (commands[i].dir) {
        case DIR_RIGHT:
            for (int j = 1; j <= commands[i].count; j++) map_put(x + j, y, '#');
            x += commands[i].count;
            break;
        case DIR_DOWN:
            for (int j = 1; j <= commands[i].count; j++) map_put(x, y + j, '#');
            y += commands[i].count;
            break;
        case DIR_LEFT:
            for (int j = 1; j <= commands[i].count; j++) map_put(x - j, y, '#');
            x -= commands[i].count;
            break;
        case DIR_UP:
            for (int j = 1; j <= commands[i].count; j++) map_put(x, y - j, '#');
            y -= commands[i].count;
            break;
        }
    }
    paint_outside(map, map_width, map_height, ' ');
    // print map
    //print_map(map, map_width, map_height);
    // count the #'s and remaining (inside) .'s
    int count = 0;
    for (int i = 0; i < map_width * map_height; i++) {
        if ((map[i] == '#') || (map[i] == '.')) count++;
    }
    printf("Count of dug holes is %d\n", count);
    printf("Info: the solution for the sample input is 62\n");
    printf("Info: the solution for the full input is 35244\n");
    return EXIT_SUCCESS;
}

