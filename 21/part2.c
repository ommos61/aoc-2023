
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 1; // 0 = no debug output, 1 = some extra debug output
#define MAX_ROWS 200
char *map_rows[MAX_ROWS];
int map_width = 0;
int row_count = 0;
int start_row = 0, start_col = 0;

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
            // determine the map width
            if (map_width == 0) {
                map_width = strlen(line);
            } else {
                assert(((unsigned)map_width == strlen(line)) && "rows should be equal size");
            }

            // store the map rows
            map_rows[row_count] = malloc(map_width + 1);
            strcpy(map_rows[row_count], line);

            char * start = strchr(line, 'S');
            if (start != NULL) {
                start_row = row_count;
                start_col = start - line;
            }

            row_count++;
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

struct pos {
    int x, y;
    struct pos *next;
};
struct pos *new_pos(int x, int y) {
    struct pos *p = malloc(sizeof(struct pos));
    p->x = x; p->y = y; p->next = NULL;
    return p;
}

struct dir {
    int dx, dy;
} dirs[] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };

unsigned char *seen = NULL;
int seen_size = 0;
int seen_count = 0;
void init_seen(int size, int count) {
    assert((size > 0) && (count > 0));
    if (seen != NULL) {
        assert((size == seen_size) && (count == seen_count));
    } else {
        seen_size = size;
        seen_count = count;
        size_t alloc_size = sizeof(unsigned char) * seen_size * seen_size * seen_count * seen_count;
        if (debug) printf("Allocating %ld bytes for 'seen' buffer\n", alloc_size);
        seen = malloc(alloc_size);
        assert(seen != NULL);
    }
    memset(seen, 0, sizeof(unsigned char) * seen_size * seen_size * seen_count * seen_count);
}
void set_seen(int x, int y) {
    int offset_x = (seen_count / 2) * seen_size;
    int offset_y = (seen_count / 2) * seen_size;
    *(seen + (offset_y + y) * (seen_count * seen_size) + (offset_x + x)) = 1;;
}
int is_seen(int x, int y) {
    int offset_x = (seen_count / 2) * seen_size;
    int offset_y = (seen_count / 2) * seen_size;
    assert(offset_x + x >= 0); assert(offset_y + y >= 0);
    assert((offset_x + x) < (seen_count * seen_size));
    assert((offset_y + y) < (seen_count * seen_size));
    unsigned char my_seen = *(seen + (offset_y + y) * (seen_count * seen_size) + (offset_x + x));
    return (my_seen != 0);
}
void free_seen(void) {
}
void print_seen(void) {
}

int positive_modulo(int a, int n) {
    return (a % n + n) % n;
}

struct pos *do_one_step(struct pos *positions) {
    struct pos *p = positions;
    struct pos *r = NULL;

    print_seen();
    init_seen(map_width, 21);
    while (p != NULL) {
        for (unsigned int d = 0; d < array_count(dirs); d++) {
            int newx = p->x + dirs[d].dx;
            int newy = p->y + dirs[d].dy;

            char c = map_rows[positive_modulo(newy, row_count)][positive_modulo(newx, map_width)];
            if ((c != '#') && !is_seen(newx, newy)) {
                set_seen(newx, newy);
                print_seen();
                struct pos *np = malloc(sizeof(struct pos));
                np->x = newx; np->y = newy;
                np->next = r;
                r = np;
            }
        }
        struct pos *temp = p;
        p = p->next;
        free(temp);
    }
    free_seen();

    return r;
}

long count_positions(struct pos *positions) {
    long count = 0;
    struct pos *p = positions;
    while (p != NULL) {
        count++;
        p = p->next;
    }
    return count;
}

long do_steps(long step_count) {
    static long steps = 0;
    static struct pos *positions = NULL;
    if (positions == NULL) positions = new_pos(start_col, start_row);

    while (steps < step_count) {
        positions = do_one_step(positions);
        steps += 1;
    }
    long count = count_positions(positions);
    if (debug) printf("Steps(%ld) = %ld\n", steps, count);
    return count;
}

long formula(long x, long a, long b, long c) {
    return (a * x * x + b * x + c);
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("Map is %dx%d\n", map_width, row_count);

    // do the stepping
    printf("Start is at (%d, %d)\n", start_col, start_row);
    long count = do_steps(64);
    if (debug) printf("Reachable positions in 64 steps: %ld\n", count);

    int size = map_width;
    assert(map_width == row_count);
    long steps = 26501365L;
    long x = steps % (2 * size);
    long x_0 = steps % (2 * size);
    long a[4];
    unsigned a_cnt = 0;
    unsigned same_count = 1;
    while (1) {
        if (a_cnt < array_count(a)) {
            a[a_cnt++] = do_steps(x);
        } else { 
            for (unsigned i = 0; i < array_count(a) - 1; i++) {
                a[i] = a[i+ 1];
            }
            a[3] = do_steps(x);
            x_0 += 2 * size;
            long d0 = a[1] - a[0]; long d1 = a[2] - a[1]; long d2 = a[3] - a[2];
            long sd0 = d1 - d0; long sd1 = d2 - d1;
            if (debug) printf("[%ld, %ld, %ld, %ld] -> [%ld, %ld, %ld] -> [%ld, %ld]\n",
                              a[0], a[1], a[2], a[3], d0, d1, d2, sd0, sd1);
            if (sd0 == sd1) {
                same_count -= 1;
                long C = a[0];
                long A = a[2] - 2 * a[1] + C; assert((A & 2) == 0); A = A / 2;
                long B = a[1] - A - C;
                if (debug) printf("formula = [ ");
                for (unsigned i = 0; i < 4; i++) {
                    if (debug) printf("%ld, ", formula(i, A, B, C));
                }
                if (debug) printf("]\n");
                long newx = (steps - x_0) / (2 * size);
                printf("Answer is %ld\n", formula(newx, A, B, C));
            }
            if (same_count == 0) break;
        }
        x += 2 * size;
    }

    printf("Info: the solution for the actual data is %ld\n", 609012263058042L);
    return EXIT_SUCCESS;
}

