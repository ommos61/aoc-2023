
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
#define MAX_GROUPS 10
struct row {
    char *map;
    int dmg_groups[MAX_GROUPS];
    int group_count;
};
#define MAX_ROWS 1024
struct row rows[MAX_ROWS];
int row_count = 0;

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
            // first get the map
            int maplen = strchr(line, ' ') - line;
            rows[row_count].map = malloc(maplen + 1);
            strncpy(rows[row_count].map, line, maplen);
            rows[row_count].map[maplen] = '\000';

            // next get the list of numbers
            char *p = line + maplen + 1;
            rows[row_count].group_count = 0;
            while (*p) {
                sscanf(p, "%d", &rows[row_count].dmg_groups[rows[row_count].group_count]);
                assert(rows[row_count].group_count <= MAX_GROUPS);
                rows[row_count].group_count++;
                // skip the numbers
                while (isdigit(*p)) p++;
                // skip the comma
                while (*p == ',') p++;
            }
            assert(row_count <= MAX_ROWS);
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

int count_occurences(char *map, char find, char end) {
    int count = 0;
    char *p = map;
    while (*p && (*p != end)) {
        if (*p == find) count++;
        p++;
    }
    return count;
}

int count_groups(char *map) {
    int groups = 0;
    char *p = map;
    int in_group = 0; // false
    while (*p) {
        if (in_group && (*p == '#')) {
            // stay in group
        } else if (*p == '#') {
            in_group = 1; // true
            groups++;
        } else {
            in_group = 0; // false
        }
        p++;
    }
    return groups;
}

int check_groups(char *map, int *groups, int group_count) {
    int check = 1; // true
    char *p = map;
    while (*p == '.') p++; // skip the non-group ones
    for (int i = 0; check && i < group_count; i++) {
        for (int j = 0; j < groups[i]; j++) {
            if (*p == '#') {
                // still OK
                p++;
            } else {
                // sill a # expected
                check = 0; // false
                break;
            }
        }
        // check for extra elements in the group, that should not be there
        if (*p == '#') {
            check = 0; // false
        }
        while (*p == '.') p++; // skip the non-group ones
    }
    // check for extra  groups
    if (strchr(p, '#') != NULL) {
        check = 0; // false
    }
    return check;
}

long try_some(char *map, int *groups, int group_count, char try) {
    long count = 0;
    char *m = map;
    //while (*m == '.') m++; // skip the non-group ones
    char *unknown;
    if ((unknown = strchr(m, '?')) == NULL) {
        // check if groups are in the map, so only 1 posibility
        //if (debug) printf("checking '%s'\n", map);
        if (check_groups(m, groups, group_count)) {
            // found match
            //printf("Found match: %s (first_group = %d, group_count = %d)\n", m, groups[0], group_count);
            if (debug) printf("Found match: %s\n", m);
            count = 1;
        } else {
            count = -1;
        }
    } else {
        // there was at least one unknown
        *unknown = try;
        int count1 = 0, count2 = 0;
        count1 = try_some(m, groups, group_count, '.');
        if (strchr(m, '?') != 0) {
            count2 = try_some(m, groups, group_count, '#');
        }
        *unknown = '?';
        if (count1 != -1) count += count1;
        if (count2 != -1) count += count2;
    }
    return count;
}

long count_arrangements(struct row *r) {
    // try something recursively
    long count1 = try_some(r->map, r->dmg_groups, r->group_count, '#');
    long count2 = try_some(r->map, r->dmg_groups, r->group_count, '.');
    if ((count1 == -1) && (count2 == -1)) printf("IT SEEMS THERE WAS NO SOLUTION.\n");

    return count1 + count2;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // check the rows for possible arrangements
    long arrangements = 0;
    printf("Number of rows: %d\n", row_count);
    for (int i = 0; i < row_count; i++) {
        long count = count_arrangements(rows + i);
        if (debug) printf("--> has %ld arrangements\n", count);
        arrangements += count;
    }
    printf("Total number of arrangements is %ld\n", arrangements);

    return EXIT_SUCCESS;
}

