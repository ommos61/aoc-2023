
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
#define get_pos(p) (*(map + (p)->x + (p)->y * map_size))
#define get_posxy(x,y) (*(map + (x) + (y) * map_size))

int *visited = NULL;
int *directions = NULL;
#define is_visited(x,y) (*(visited + (x) + (y) * map_size))
#define set_visited(x,y) (*(visited + (x) + (y) * map_size) = 1)
#define unset_visited(x,y) (*(visited + (x) + (y) * map_size) = 0)

int *sides = NULL;
const char *side_names[3] = { "NONE", "LEFT", "RIGHT" };
#define SIDE_NONE 0
#define SIDE_LEFT 1
#define SIDE_RIGHT 2
#define get_sidexy(x,y) (*(sides + (x) + (y) * map_size))
#define set_sidexy(x,y,side) (*(sides + (x) + (y) * map_size) = (side))

char get_pos_test(int x, int y) {
    if ((x >=0) && (x < map_size) && (y >= 0) && (y < map_size)) {
        return get_posxy(x, y);
    } else {
        // outside the field
        return 'X';
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
#define get_direction(x,y) (*(directions + (x) + (y) * map_size))
#define set_direction(x,y,dir) (*(directions + (x) + (y) * map_size) = (dir))

#define BOLD    "\033[1m"
#define INVERSE "\033[7m"
#define RED     "\033[41m"
#define GREEN   "\033[42m"
#define BLUE    "\033[44m"
#define RESET   "\033[0m"
void print_filled_map(void) {
    for (int y = 0; y < map_size; y++) {
        for (int x = 0; x < map_size; x++) {
            char c = get_posxy(x, y);
            if (is_visited(x, y)) {
                printf(INVERSE "%c" RESET, c);
            } else {
                switch (get_sidexy(x, y)) {
                case SIDE_NONE:
                    printf(RED "%c" RESET, c);
                    break;
                case SIDE_LEFT:
                    printf(BLUE "%c" RESET, 'O');
                    break;
                case SIDE_RIGHT:
                    printf(GREEN "%c" RESET, 'I');
                    break;
                }
            }
        }
        printf("\n");
    }
}

void fill_recurse(int x, int y, int side) {
//    printf("Filling from (%d, %d) to %s\n", x, y, side_names[side]);
    assert(!is_visited(x, y));
//    assert((get_sidexy(x, y) == SIDE_NONE) || (get_sidexy(x, y) == side));

//    if (get_sidexy(x, y) == SIDE_NONE) {
        set_sidexy(x, y, side);
        for (unsigned int i = 0; i < array_count(dirs); i++){
            int new_x = x + dirs[i].x;
            int new_y = y + dirs[i].y;
            if ((!is_visited(new_x, new_y)) && (get_sidexy(new_x, new_y) == SIDE_NONE)) {
                fill_recurse(new_x, new_y, side);
            }
        }
//    }
}

int determine_side(int x, int y, int loop_dir, int target_dir) {
    int side = SIDE_NONE;
    switch (get_posxy(x, y)) {
    case '-':
        if ((loop_dir == DIR_LEFT) && (target_dir == DIR_UP)) side = SIDE_RIGHT; else
        if ((loop_dir == DIR_LEFT) && (target_dir == DIR_DOWN)) side = SIDE_LEFT; else
        if ((loop_dir == DIR_RIGHT) && (target_dir == DIR_DOWN)) side = SIDE_RIGHT; else
        if ((loop_dir == DIR_RIGHT) && (target_dir == DIR_UP)) side = SIDE_LEFT; else SIDE_NONE;
        break;
    case 'F':
        if ((loop_dir == DIR_UP) && (target_dir == DIR_LEFT)) side = SIDE_LEFT; else
        if ((loop_dir == DIR_DOWN) && (target_dir == DIR_LEFT)) side = SIDE_RIGHT; else
        if ((loop_dir == DIR_DOWN) && (target_dir == DIR_UP)) side = SIDE_RIGHT; else
        if ((loop_dir == DIR_RIGHT) && (target_dir == DIR_LEFT)) side = SIDE_LEFT; else
        if ((loop_dir == DIR_RIGHT) && (target_dir == DIR_UP)) side = SIDE_LEFT; else SIDE_NONE;
        break;
    case 'J':
        if ((loop_dir == DIR_RIGHT) && (target_dir == DIR_DOWN)) side = SIDE_RIGHT; else
        if ((loop_dir == DIR_LEFT) && (target_dir == DIR_DOWN)) side = SIDE_LEFT; else
        if ((loop_dir == DIR_LEFT) && (target_dir == DIR_RIGHT)) side = SIDE_LEFT; else
        if ((loop_dir == DIR_UP) && (target_dir == DIR_RIGHT)) side = SIDE_RIGHT; else
        if ((loop_dir == DIR_UP) && (target_dir == DIR_DOWN)) side = SIDE_RIGHT; else
        if ((loop_dir == DIR_DOWN) && (target_dir == DIR_RIGHT)) side = SIDE_LEFT; else SIDE_NONE;
        break;
    case '7':
        if ((loop_dir == DIR_UP) && (target_dir == DIR_LEFT)) side = SIDE_RIGHT; else
        if ((loop_dir == DIR_DOWN) && (target_dir == DIR_LEFT)) side = SIDE_LEFT; else
        if ((loop_dir == DIR_DOWN) && (target_dir == DIR_UP)) side = SIDE_LEFT; else
        if ((loop_dir == DIR_DOWN) && (target_dir == DIR_RIGHT)) side = SIDE_LEFT; else
        if ((loop_dir == DIR_LEFT) && (target_dir == DIR_RIGHT)) side = SIDE_RIGHT; else
        if ((loop_dir == DIR_LEFT) && (target_dir == DIR_UP)) side = SIDE_RIGHT; else SIDE_NONE;
        break;
    case 'L':
        if ((loop_dir == DIR_DOWN) && (target_dir == DIR_LEFT)) side = SIDE_RIGHT; else
        if ((loop_dir == DIR_UP) && (target_dir == DIR_LEFT)) side = SIDE_LEFT; else
        if ((loop_dir == DIR_UP) && (target_dir == DIR_DOWN)) side = SIDE_LEFT; else
        if ((loop_dir == DIR_UP) && (target_dir == DIR_DOWN)) side = SIDE_LEFT; else
        if ((loop_dir == DIR_RIGHT) && (target_dir == DIR_LEFT)) side = SIDE_RIGHT; else
        if ((loop_dir == DIR_RIGHT) && (target_dir == DIR_DOWN)) side = SIDE_RIGHT; else SIDE_NONE;
        break;
    case '|':
        if ((loop_dir == DIR_UP) && (target_dir == DIR_RIGHT)) side = SIDE_RIGHT; else
        if ((loop_dir == DIR_UP) && (target_dir == DIR_LEFT)) side = SIDE_LEFT; else
        if ((loop_dir == DIR_DOWN) && (target_dir == DIR_LEFT)) side = SIDE_RIGHT; else
        if ((loop_dir == DIR_DOWN) && (target_dir == DIR_RIGHT)) side = SIDE_LEFT; else SIDE_NONE;
        break;
    case 'X':
        side = SIDE_NONE;
        break;
    case 'S':
        side = SIDE_NONE;
        break;
    default:
        printf("unhandled side filling for '%c' (%d, %d)\n", get_posxy(x, y), x, y);
        assert(0 && "unhandled side filling!");
        break;
    }
    return side;
}

void fill_insides(void) {
    for (int y = 0; y < map_size; y++) {
        for (int x = 0; x < map_size; x++) {
            if (is_visited(x, y)) {
                // check all neighbors that are not visited
                for (unsigned int d = 0; d < array_count(dirs); d++) {
                    int new_x = x + dirs[d].x;
                    int new_y = y + dirs[d].y;
                    if (!is_visited(new_x, new_y)) {
                        int side = determine_side(x, y, get_direction(x, y), d);
                        if (debug) printf("Determine_side(%d, %d, '%c', dir = %s, target = %s) gives %s\n",
                                       x, y, get_posxy(x, y), dir_names[get_direction(x, y)], dir_names[d], side_names[side]);
                        assert((side == SIDE_NONE) || (side != get_sidexy(x, y)));
                        if (side != SIDE_NONE) fill_recurse(new_x, new_y, side);
                    }
                }
            }
        }
    }
}

int map_side_count(int dir) {
    int count = 0;
    for (int i = 0; i < map_size * map_size; i++) {
        if (sides[i] == dir) count++;
    }
    return count;
}

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
    char curpos_char = get_pos(p);
//    if (debug) print_path();

    struct pos *newpos = malloc(sizeof(struct pos));
    newpos->x = p->x + dirs[dir_index].x;
    newpos->y =  p->y + dirs[dir_index].y;
    newpos->next = path;
    char newpos_char = get_pos(newpos);
    char newpos_tested = get_pos_test(newpos->x, newpos->y);
    if (debug) printf("Step %d: '%c' (%d, %d) -> '%c''%c' (%d, %d)\n",
                   steps, curpos_char, p->x, p->y, newpos_char, newpos_tested, newpos->x, newpos->y);
    if (strchr(allow_dirs[dir_index], newpos_tested) != NULL) {
        // check if we reached the end
        if ((steps > 2) && (get_pos(newpos) == 'S')) {
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
                    set_direction(newpos->x, newpos->y, newdir);
                    if (debug) printf("New direction is from %s to %s.\n", dir_names[dir_index], dir_names[newdir]);
                    int new_steps = try_dir(newpos, newdir, steps + 1);
                    if (debug) printf("Resulted in %d steps\n", new_steps);
                    if (new_steps == -1) {
                        // revert
                        printf("Backtracking...\n");
                        unset_visited(newpos->x, newpos->y);
                        set_direction(newpos->x, newpos->y, DIR_NONE);
                        path = path->next;
                        free(newpos);
                        if (debug) print_path();
                    } else {
                        // the 'try' actually reached the end
                        // fill the sides if empty
#if 0
                        printf("Filling sides for '%c' (%d, %d) with direction %s\n", get_pos(newpos), newpos->x, newpos->y, dir_names[newdir]);
                        fill_sides(newpos, newdir);
#endif
                        // report the number of steps back up
                        return new_steps;
                    }
                } else {
                    printf("New direction is DIR_NONE unexpectedly!\n");
                }
            } else {
               printf("Already visited %c (%d, %d)\n", get_pos(newpos), newpos->x, newpos->y);
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
    directions = malloc(map_size * map_size * sizeof(int));
    sides = malloc(map_size * map_size * sizeof(int));

    // try all the directions from the start
    for (unsigned int i = 0; i < array_count(dirs); i++) {
        // preform one step
        if (debug) printf("Try direction: %s\n", dir_names[i]);
        // clear visited status and direction
        memset(visited, 0, map_size * map_size * sizeof(int));
        set_visited(cur->x, cur->y);
        for (int i = 0; i < map_size * map_size; i++) {
            directions[i] = DIR_NONE;
        }
        // clear the LEFT and RIGHT status
        memset(sides, 0, map_size * map_size * sizeof(int));
        // now try to walk the loop
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

    // now fill the insides
    fill_insides();
    if (debug) print_filled_map();
    int count_left = map_side_count(SIDE_LEFT);
    int count_right = map_side_count(SIDE_RIGHT);
    printf("LEFT (O) count = %d, RIGHT (I) count = %d\n", count_left, count_right);

    return EXIT_SUCCESS;
}

