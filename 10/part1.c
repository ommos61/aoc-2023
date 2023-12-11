
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
char *map = NULL;
int map_size = 0;

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
            // get one line of the map
            if (map_size == 0) {
                map_size = strlen(line);
                map = malloc(map_size * map_size * sizeof(char));
            }
            memcpy(map + line_count * map_size, line, map_size);
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

void print_map(void) {
    for (int y = 0; y < map_size; y++) {
        for (int x = 0; x < map_size; x++) {
            putchar(*(map + y * map_size + x));
        }
        printf("\n");
    }
}

int find_start(void) {
    int index = -1;
    for (int i = 0; i < map_size * map_size; i++) {
        if (map[i] == 'S') {
            return i;
        }
    }
    assert(index != -1);
    return index;
}

struct pos {
    int x; int y;
    struct pos *next;
};
struct pos *path = NULL;
#define get_pos(p) (*(map + p.x + p.y * map_size))
#define get_posxy(x,y) (*(map + x + y * map_size))
int *visited = NULL;
#define is_visited(x,y) (*(visited + x + y * map_size))
#define set_visited(x,y) (*(visited + x + y * map_size) = 1)
#define unset_visited(x,y) (*(visited + x + y * map_size) = 0)
char get_pos_test(int x, int y) {
    if ((x >=0) && (x < map_size) && (y >= 0) && (y < map_size)) {
        return get_posxy(x, y);
    } else {
        return '.';
    }
}

void print_path(void) {
    struct pos *p = path;
    printf("Path: ");
    while (p) {
        printf("(%d, %d) ", p->x, p->y);
        p = p->next;
    }
    printf("\n");
}

struct dir { int x; int y; };
//                      left,    right,  up,      down
struct dir dirs[4] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1}};
char *allow_dirs[4] = { "-FLS", "-J7S", "|F7S", "|JLS" };
char *dir_names[5] = { "LEFT", "RIGHT", "UP", "DOWN", "NONE" };
#define DIR_LEFT 0
#define DIR_RIGHT 1
#define DIR_UP 2
#define DIR_DOWN 3
#define DIR_NONE 4
int get_new_dir(struct dir curdir, char to) {
    int newdir = DIR_NONE;
    switch (to) {
    case '-':
        if (curdir.x == -1) newdir = DIR_LEFT; else newdir = DIR_RIGHT;
        break;
    case 'F':
        if (curdir.x == -1) newdir = DIR_DOWN; else newdir = DIR_RIGHT;
        break;
    case 'J':
        if (curdir.x == +1) newdir = DIR_UP; else newdir = DIR_LEFT;
        break;
    case '7':
        if (curdir.x == +1) newdir = DIR_DOWN; else newdir = DIR_LEFT;
        break;
    case 'L':
        if (curdir.x == -1) newdir = DIR_UP; else newdir = DIR_RIGHT;
        break;
    case '|':
        if (curdir.y == -1) newdir = DIR_UP; else newdir = DIR_DOWN;
        break;
    case '.':
        newdir = DIR_NONE;
        break;
    default:
        assert(0 && "Unknown new direction!");
        break;
    }
    return newdir;
}

int try_dir(struct pos *p, int dir_index, int steps) {
    char curpos_char = get_posxy(p->x, p->y);
//    if (debug) print_path();

    struct pos *newpos = malloc(sizeof(struct pos));
    newpos->x = p->x + dirs[dir_index].x;
    newpos->y =  p->y + dirs[dir_index].y;
    newpos->next = path;
    char newpos_char = get_posxy(newpos->x, newpos->y);
    char newpos_test = get_pos_test(newpos->x, newpos->y);
    if (debug) printf("Step %d: '%c' (%d, %d) -> '%c''%c' (%d, %d)\n",
                      steps, curpos_char, p->x, p->y, newpos_char, newpos_test, newpos->x, newpos->y);
    if (strchr(allow_dirs[dir_index], get_pos_test(newpos->x, newpos->y)) != NULL) {
        // check if we reached the end
        if ((steps > 2) && (get_posxy(newpos->x, newpos->y) == 'S')) {
            // found the end
            path = newpos;
            if (debug) print_path();
            if (debug) printf("Steps: %d\n", steps);
            return steps;
        } else {
            if (!is_visited(newpos->x, newpos->y)) {
                path = newpos;
                // try further on the path
                set_visited(newpos->x, newpos->y);
                int newdir = get_new_dir(dirs[dir_index], newpos_char);
                if (newdir != DIR_NONE) {
                    if (debug) printf("New direction is from %s to %s.\n", dir_names[dir_index], dir_names[newdir]);
                    int new_steps = try_dir(newpos, newdir, steps + 1);
                    if (debug) printf("Resulted in %d steps\n", new_steps);
                    if (new_steps == -1) {
                        // revert
                        printf("Backtracking...\n");
                        unset_visited(newpos->x, newpos->y);
                        path = path->next;
                        free(newpos);
                        if (debug) print_path();
                    } else {
                        // the 'try' actually reached the end
                        return new_steps;
                    }
                } else {
                    printf("New direction is DIR_NONE unexpectedly!\n");
                }
            } else {
               printf("Already visited %c (%d, %d)\n", get_posxy(newpos->x, newpos->y), newpos->x, newpos->y);
            }
        }
    }

    return -1;
}

int walk_loop(struct pos start) {
    int steps = 0;
    struct pos *cur = malloc(sizeof(struct pos));
    cur->x = start.x;
    cur->y = start.y;
    cur->next = NULL;
    path = cur;
    if (visited != NULL) free(visited);
    visited = malloc(map_size * map_size * sizeof(int));
    memset(visited, 0, map_size * map_size * sizeof(int));
    set_visited(cur->x, cur->y);

    // try all the directions from the start
    for (unsigned int i = 0; i < array_count(dirs); i++) {
        // preform one step
        if (debug) printf("Try direction: %s\n", dir_names[i]);
        steps = try_dir(cur, i, 1);
        if (steps != -1) break;
    }
    return steps;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // Find the loop on the map
    if (debug) print_map();
    int start_index = find_start();
    struct pos start = { start_index % map_size, start_index / map_size, NULL };
    printf("Start is (x = %d, y = %d)\n", start.x, start.y);

    int steps = walk_loop(start);
    printf("Farthest point steps count is %d\n", steps / 2);

    return EXIT_SUCCESS;
}

