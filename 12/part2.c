
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output
#define MAX_GROUPS 100
struct row {
    char *map;
    int groups[MAX_GROUPS];
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
                sscanf(p, "%d", &rows[row_count].groups[rows[row_count].group_count]);
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

void print_row(char *m, int *groups, int group_count) {
    printf("%s ", m);
    for (int i = 0; i < group_count; i++) {
        printf("%d", groups[i]);
        if (i != group_count - 1) printf(",");
    }
    printf("\n");
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

// cache implementation
long *cache = NULL;
int cache_width = 0;
int cache_height = 0;
void init_cache(int width, int height) {
    if (cache != NULL) free(cache);

    cache = malloc(width * height * sizeof(long));
    cache_width = width;
    cache_height = height;
    for (int i = 0; i < width * height; i++) cache[i] = -1;
}
long cache_get(int x, int y) {
    assert(x < cache_width);
    assert(y < cache_height);
    return cache[x + y * cache_width];
}
void cache_put(int x, int y, long value) {
    assert(x < cache_width);
    assert(y < cache_height);
    cache[x + y * cache_width] = value;
}

#define is_char_in(c,s) (strchr((s), (c)) != NULL)
#define index_of(c,s) (strchr((s), (c)) - (s))

int sum_groups(int *groups, int group_count) {
    int sum = 0;
    for (int i = 0; i < group_count; i++) {
         sum += groups[i];
    }
    return sum;
}

int is_group_possible(char *map, int start_index, int end_index) {
    int possible = 1;
    for (int i = start_index; i < end_index; i++) {
        if (is_char_in(map[i], "#?")) continue;
        else {
            possible = 0;
        }
    }
    return possible;
}

long count_arrangements(char *map, int *groups, int group_count) {
    // no more groups
    if (group_count == 0) {
        //printf("No more groups left to check\n");
        if (!is_char_in('#', map)) {
            return 1;
        } else {
            return 0;
        }
    }

    //printf("checking: ");
    //print_row(map, groups, group_count);

    // check if there is still enough space for the groups and separators
    int min_start = 0;
    int max_start = strlen(map) - sum_groups(groups, group_count) - group_count + 1;
    if (is_char_in('#', map)) { // cannot start first damaged spring
        int dmgd_index = index_of('#', map);
        max_start = MIN(dmgd_index, max_start);
    }

    long count = 0;
    //printf(" trying from %d to %d\n", min_start, max_start);
    for (int gr_start = min_start; gr_start < max_start + 1; gr_start++) {
        int gr_end = gr_start + groups[0];
        //printf("Group: '");
        //for (int i = gr_start; i < gr_end; i++) putchar(map[i]);
        //printf("'\n");

        int is_possible_group = is_group_possible(map, gr_start, gr_end);
        int is_end_of_map = ((unsigned)gr_end) >= strlen(map);
        int is_group_separated = (is_end_of_map || is_char_in(map[gr_end], ".?"));
        //printf("bools = %d, %d, %d\n", is_possible_group, is_end_of_map, is_group_separated);
        if (is_possible_group && is_group_separated) {

            char *remaining_map = map + gr_end + 1;

            // now recurse, TODO: use cache
            long cache_val = cache_get(strlen(remaining_map), group_count - 1);
            if (cache_val == -1) {
                long val = count_arrangements(remaining_map, groups + 1, group_count - 1);
                cache_put(strlen(remaining_map), group_count - 1, val);
                count += val;
            } else {
                count += cache_val;
            }
        }
    }

    return count;
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
        if (debug) print_row(rows[i].map, rows[i].groups, rows[i].group_count);
        // replicate the data
        int repl_count = 5;
        int group_count = rows[i].group_count;
        int new_group_count = repl_count * group_count;
        // first the map
        int new_map_len = repl_count * strlen(rows[i].map) + (repl_count - 1);
        char *temp_map = malloc((new_map_len + 2) * sizeof(char));
        strcpy(temp_map, rows[i].map);
        for (int j = 1; j < repl_count; j++) {
            strcat(temp_map, "?");
            strcat(temp_map, rows[i].map);
        }
        temp_map[new_map_len + 1] = '\000';
        // then the group list
        int *temp_groups = malloc(new_group_count * sizeof(int));
        for (int j = 0; j < repl_count; j++) {
            for (int k = 0; k < group_count; k++) {
                temp_groups[group_count * j + k] = rows[i].groups[k];
            }
        }
        if (debug) print_row(temp_map, temp_groups, new_group_count);

        init_cache(strlen(temp_map) + 2, new_group_count + 2);
        long count = count_arrangements(temp_map, temp_groups, new_group_count);
        if (debug) printf("Arrangements: %d -> %ld\n", i+1, count);
        arrangements += count;
    }
    printf("Total number of arrangements is %ld\n", arrangements);

    return EXIT_SUCCESS;
}

