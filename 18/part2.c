
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output
#define DIR_RIGHT   0
#define DIR_DOWN    1
#define DIR_LEFT    2
#define DIR_UP      3
#define DIR_UNKNOWN 4
char *dir_chars = "RDLU?";
#define MAX_COMMANDS 1000
struct command {
    int dir;
    int count;
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
#if 0
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
#else
            // parse the command and store it
            char *p = strchr(line, '#');
            assert((p != NULL) && "No hex value found to parse");
            p++;
            int count = 0;
            char *hex_chars = "0123456789abcdef";
            for (int i = 0; i < 5; i++) {
                count = 16 * count + (strchr(hex_chars, *p) - hex_chars);
                p++;
            }
            commands[command_count].count = count;
            int dir = DIR_UNKNOWN;
            switch (*p) {
            case '0': dir = DIR_RIGHT; break;
            case '1': dir = DIR_DOWN; break;
            case '2': dir = DIR_LEFT; break;
            case '3': dir = DIR_UP; break;
            default: dir = DIR_UNKNOWN; break;
            }
            commands[command_count].dir = dir;
#endif
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


struct row {
    long y;
    struct elem {
        long start;
        long end;
        struct elem *next;
    } *elems;
    struct row *prev;
    struct row *next;
} *rows = NULL;

void print_row(long row_y, struct elem *elements) {
    printf("Row %ld: ", row_y);
    struct elem *el = elements;
    while (el != NULL) {
        printf("(%ld -> %ld) ", el->start, el->end);
        el = el->next;
    }
    printf("\n");
}

void print_map(void) {
    printf("--------- MAP -----------\n");
    struct row *r = rows;
    while (r != NULL) {
        print_row(r->y, r->elems);
        r = r->next;
    }
    printf("-------------------------\n");
}

struct row *make_new_row(long y) {
    struct row *new_row = malloc(sizeof(struct row));
    new_row->y = y;
    new_row->elems = NULL;
    new_row->prev = NULL;
    new_row->next = NULL;

    return new_row;
}
struct row *get_row(long y) {
    if (debug) printf("Getting row for y = %ld\n", y);
    // find the row
    struct row *r = rows;
    while (r != NULL) {
        if (r->y == y) {
            // found it
            break;
        } else if (r->y > y) {
            // we went past it, so we need to insert one
            struct row *new_row = make_new_row(y);
            new_row->next = r;
            new_row->prev = r->prev;
            if (r->prev == NULL) {
                // insert before first
                rows = new_row;
            } else {
                r->prev->next = new_row;
            }
            r->prev = new_row;
            r = new_row;
            break;
        }
        r = r->next;
    }
    if (r == NULL) {
        r = make_new_row(y);
        if (rows == NULL) {
            rows = r;
        } else {
            struct row *e = rows;
            while (e->next != NULL) e = e->next;
            e->next = r;
            r->prev = e;
        }
    }

    assert(r != NULL);
    return r;
}

void insert_row_el(long y, long startx, long endx) {
    if (debug) printf("Inserting y = %ld, x = (%ld, %ld)\n", y, startx, endx);
    struct elem *el = malloc(sizeof(struct elem));
    el->start = startx, el->end = endx;
    struct row *r = get_row(y);
    el->next = r->elems;
    r->elems = el;
}

void delete_el(struct elem **r, struct elem *e) {
    struct elem **d = r;
    while ((*d != NULL) && (*d != e)) {
        d = &((*d)->next);
    }
    *d = (*d)->next;
    free(e);
}

long count_row(struct elem *row) {
    long count = 0;
    struct elem *e = row;
    while (e != NULL) {
        count += e->end - e->start + 1;
        e = e->next;
    }

    return count;
}

void build_map(void) {
    long x = 0, y = 0;
    for (int i = 0; i < command_count; i++) {
        long newx = x, newy = y;
        switch (commands[i].dir) {
        case DIR_RIGHT:
            newx += commands[i].count;
            insert_row_el(newy, x, newx);
            break;
        case DIR_LEFT:
            newx -= commands[i].count;
            insert_row_el(newy, newx, x);
            break;
        case DIR_DOWN:
            newy += commands[i].count;
            break;
        case DIR_UP:
            newy -= commands[i].count;
            break;
        }
        x = newx; y = newy;
    }
}

long count_map(void) {
    long count = 0;
    assert(rows != NULL);

    // start with a copy of the first row
    long cury = rows->y;
    struct elem *row_elements = NULL, *el = rows->elems;
    while (el != NULL) {
        struct elem *e = malloc(sizeof(struct elem));
        e->start = el->start;
        e->end = el->end;
        e->next = row_elements;
        row_elements = e;

        el = el->next;
    }

    count += count_row(row_elements);
    if (debug) printf("At %ld, total count is %ld\n", rows->y, count);
    struct row *r = rows->next;
    while (r != NULL) {
        long row_count = count_row(row_elements);
        count += (r->y - cury - 1) * row_count;
        if (debug) printf("At %ld, total count is %ld (+%ld * %ld)\n", r->y, count, r->y - cury - 1, row_count);
        struct elem *e = r->elems;
        while (e != NULL) {
            if (debug) print_row(cury, row_elements);
            if (debug) printf("handling (y = %ld, (%ld -> %ld))\n", r->y, e->start, e->end);
            // TODO: actually handle the element
            struct elem *re = row_elements;
            int handled = 0;
            while (re != NULL) {
                if ((re->start == e->start) && (re->end == e->end)) {
                    // remove the 're'
                    if (debug) printf("XXX> removing (%ld -> %ld)\n", e->start, e->end);
                    delete_el(&row_elements, re);
                    handled = 1; break;
                } else if ((e->start > re->start) && (e->end < re->end)) {
                    // break up an element
                    struct elem *n = malloc(sizeof(struct elem));
                    n->start = e->end;
                    n->end = re->end;
                    n->next = re->next;
                    re->end = e->start;
                    re->next = n;
                    handled = 1; break;
                } else if (re->start == e->start) {
                    // shorten the element
                    re->start = e->end;
                    handled = 1; break;
                } else if (re->end == e->end) {
                    // shorten the element
                    re->end = e->start;
                    handled = 1; break;
                } else if (re->end == e->start) {
                    // extend the element
                    re->end = e->end;
                    row_count += e->end - e->start;
                    handled = 1; break;
                } else if (re->start == e->end) {
                    // extend the element
                    re->start = e->start;
                    row_count += e->end - e->start;
                    handled = 1; break;
                }
                re = re->next;
            }
            if (!handled) {
                if (debug) printf("XXX> adding (%ld -> %ld)\n", e->start, e->end);
                struct elem *n = malloc(sizeof(struct elem));
                n->start = e->start;
                n->end = e->end;
                n->next = row_elements;
                row_elements = n;
                row_count += e->end - e->start + 1;
            }
            // check if elements should be joined
            struct elem *e1 = row_elements;
            int deleted = 0;
            while ((!deleted) && (e1 != NULL)) {
                struct elem *e2 = row_elements;
                while ((!deleted) && (e2 != NULL)) {
                    if (e1 != e2) {
                        if (e1->end == e2->start) {
                            e1->end = e2->end;
                            delete_el(&row_elements, e2);
                            row_count -= 1;
                            deleted = 1;
                        } else if (e2->end == e1->start) {
                            e1->start = e2->start;
                            delete_el(&row_elements, e2);
                            row_count -= 1;
                            deleted = 1;
                        }
                    }
                    if (!deleted) e2 = e2->next;
                }
                e1 = e1->next;
            }
            if (debug) print_row(cury, row_elements);
            if (debug) printf("==========================\n");
            e = e->next;
        }
        count += row_count;
        if (debug) printf("At %ld, total count is %ld (+%ld)\n", r->y, count, row_count);
        cury = r->y;
        r = r->next;
    }

    // Make sure the row is empty when finished
    assert(row_elements == NULL);
    return count;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // walk the commands and determine the min and max x and y
    long minx = 0, maxx = 0;
    long miny = 0, maxy = 0;
    long curx = 0, cury = 0;
    if (debug) printf("Commands:\n");
    for (int i = 0; i < command_count; i++) {
        if (debug) printf("%c %d\n", dir_chars[commands[i].dir], commands[i].count);
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
    printf("End position is (%ld, %ld)\n", curx, cury);
    printf("Field is from (%ld, %ld) to (%ld, %ld)\n", minx, miny, maxx, maxy);
    long map_width = maxx - minx + 1;
    long map_height = maxy - miny + 1;
    printf("size = %ldx%ld\n", map_width, map_height);

    int debug_save = debug; debug = 0;
    build_map();
    if (debug) print_map();
    debug = debug_save;
    long count = count_map();
    printf("LONG_MAX     is %ld\n", LONG_MAX);
    printf("The solution is %ld\n", count);

    printf("Info: the solution for the sample data is 952408144115\n");
    printf("Info: the solution for the actual data is 85070763635666\n");
    return EXIT_SUCCESS;
}

