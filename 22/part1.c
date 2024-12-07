
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output
#define MAX_BLOCKS 1500
struct block {
    int x1; int y1; int z1;
    int x2; int y2; int z2;
    int fallen;
    struct support {
        int id;
        struct support *next;
    } *supporting;
} blocks[MAX_BLOCKS];
int block_count = 0;
int minx = 0, miny = 0;
int maxx = 0, maxy = 0;
int maxz = 0;
int map_width = 0, map_depth = 0;

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
            // store the data
            sscanf(line, "%d,%d,%d", &blocks[block_count].x1, &blocks[block_count].y1, &blocks[block_count].z1);
            minx = MIN(minx, MIN(blocks[block_count].x1, blocks[block_count].x2));
            maxx = MAX(maxx, MAX(blocks[block_count].x1, blocks[block_count].x2));
            miny = MIN(miny, MIN(blocks[block_count].y1, blocks[block_count].y2));
            maxy = MAX(maxy, MAX(blocks[block_count].y1, blocks[block_count].y2));
            maxz = MAX(maxz, MAX(blocks[block_count].z1, blocks[block_count].z2));
            char *end = strchr(line, '~');
            if (end != NULL) {
                sscanf(end + 1, "%d,%d,%d", &blocks[block_count].x2, &blocks[block_count].y2, &blocks[block_count].z2);
            }
            block_count++;
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

void print_block(struct block b) {
    printf("(%d, %d, %d) -> ", b.x1, b.y1, b.z1);
    printf("(%d, %d, %d)", b.x2, b.y2, b.z2);
}

void print_blocks(void) {
    for (int i = 0; i < block_count; i++) {
        print_block(blocks[i]);
        printf("\n");
    }
}

int get_lowest(void) {
    int lowest = -1;
    int lowest_height = INT_MAX;
    for (int i = 0; i < block_count; i++) {
        if (blocks[i].fallen) continue;
        int height = MIN(blocks[i].z1, blocks[i].z2);
        if (height < lowest_height) {
            lowest = i;
            lowest_height = height;
        }
    }
    return lowest;
}

int *heights = NULL;
#define get_height(x,y) (*(heights + (y) * map_width + (x)))
#define set_height(x,y,z) (*(heights + (y) * map_width + (x)) = (z))
void print_heights(void) {
    for (int y = miny; y <= maxy; y++) {
        for (int x = minx; x <= maxx; x++) {
            printf("%d ", get_height(x, y));
        }
        printf("\n");
    }
}
int determine_highest(int x1, int y1, int x2, int y2) {
    int highest = 0;
    int minx = MIN(x1, x2), maxx = MAX(x1, x2);
    int miny = MIN(y1, y2), maxy = MAX(y1, y2);
    for (int x = minx; x <= maxx; x++) {
        for (int y = miny; y <= maxy; y++) {
            int height = get_height(x, y);
            //if (debug) printf("(%d, %d) = %d\n", x, y, height);
            if (height > highest) highest = height;
        }
    }
    return highest;
}
void update_heights(struct block b) {
    int minx = MIN(b.x1, b.x2), maxx = MAX(b.x1, b.x2);
    int miny = MIN(b.y1, b.y2), maxy = MAX(b.y1, b.y2);
    int maxz = MAX(b.z1, b.z2);
    for (int x = minx; x <= maxx; x++) {
        for (int y = miny; y <= maxy; y++) {
            //if (debug) printf("Updating (%d, %d) = %d\n", x, y, maxz);
            set_height(x, y, maxz);
        }
    }
}

void check_blocks(void) {
    // only one of the x, y, z between the start and end point may be different
    for (int b = 0; b < block_count; b++) {
        int count = 0;
        if (blocks[b].x1 != blocks[b].x2) count++;
        if (blocks[b].y1 != blocks[b].y2) count++;
        if (blocks[b].z1 != blocks[b].z2) count++;
        if (count > 1) {
            printf("The block %d is not one dimensional.\n", b);
            printf("The offending block is: "); print_block(blocks[b]); printf("\n");
        }
        assert(blocks[b].x1 <= blocks[b].x2);
        assert(blocks[b].y1 <= blocks[b].y2);
        assert(blocks[b].z1 <= blocks[b].z2);
    }
}

void fall_blocks(void) {
    // reset the state of all blocks to not fallen
    for (int i = 0; i < block_count; i++) blocks[i].fallen = 0; // false
    // create a height map for the fallen blocks
    if (heights == NULL) heights = malloc((maxx + 1) * (maxy + 1) * sizeof(int));
    memset(heights, 0, (maxx + 1) * (maxy + 1) * sizeof(int));

    // find the lowest block that is still falling
    int b = get_lowest();
    //if (debug) print_heights();
    while (b != -1) {
        //if (debug) { printf("Falling block "); print_block(blocks[b]); printf("\n"); }
        int target_height = determine_highest(blocks[b].x1, blocks[b].y1, blocks[b].x2, blocks[b].y2);
        //if (debug) printf("Highest point below = %d\n", target_height);
        int block_minz = MIN(blocks[b].z1, blocks[b].z2);
        //int block_maxz = MAX(blocks[b].z1, blocks[b].z2);
        int drop_length = block_minz - target_height - 1;
        //if (debug) printf("Drop length = %d\n", drop_length);
        assert((drop_length >= 0) && "cannot handle negative drops");
        if (drop_length > 0) {
            blocks[b].z1 -= drop_length;
            blocks[b].z2 -= drop_length;
        }
        blocks[b].fallen = 1; // true

        // update the height map for the fallen  block
        update_heights(blocks[b]);
        //if (debug) print_heights();
        //if (debug) { printf("Fallen block "); print_block(blocks[b]); printf("\n"); }

        // determine the next lowest block still falling
        b = get_lowest();
    }
}

int find_block(int x, int y, int z) {
    int found = -1;
    if (z < 1) return found;
    for (int b = 0; b < block_count; b++) {
        // determine is the x, y, z is contained in the block
        if ((x >= blocks[b].x1) && (x <= blocks[b].x2) &&
            (y >= blocks[b].y1) && (y <= blocks[b].y2) &&
            (z >= blocks[b].z1) && (z <= blocks[b].z2)) {
            found = b;
            break; // because thare should be only one
        }
    }
    return found;
}

void determine_support(void) {
    // reset the supporting state of all blocks
    for (int i = 0; i < block_count; i++) blocks[i].supporting = NULL;

    // for each block determine the blocks that are on top of it
    for (int b = 0; b < block_count; b++) {
        if (blocks[b].x1 != blocks[b].x2) {
            // it a block in the x axis
            for (int x = blocks[b].x1; x <= blocks[b].x2; x++) {
                // find the block index of the block below it
                int block = find_block(x, blocks[b].y1, blocks[b].z1 - 1);
                if (block != -1) {
                    // add it to the supporting list
                    struct support *s = malloc(sizeof(struct support));
                    s->id = block; s->next = blocks[b].supporting;

                    // check if already in the list
                    struct support *p = blocks[b].supporting;
                    while (p != NULL) {
                        if (p->id == block) break;
                        p = p->next;
                    }
                    if (p == NULL) {
                        blocks[b].supporting = s;
                    } else {
                        // it was already there
                        free(s);
                    }
                }
            }
        } else if (blocks[b].y1 != blocks[b].y2) {
            // it a block in the y axis
            for (int y = blocks[b].y1; y <= blocks[b].y2; y++) {
                // find the block index of the block below it
                int block = find_block(blocks[b].x1, y, blocks[b].z1 - 1);
                if (block != -1) {
                    // add it to the supporting list
                    struct support *s = malloc(sizeof(struct support));
                    s->id = block; s->next = blocks[b].supporting;

                    // check if already in the list
                    struct support *p = blocks[b].supporting;
                    while (p != NULL) {
                        if (p->id == block) break;
                        p = p->next;
                    }
                    if (p == NULL) {
                        blocks[b].supporting = s;
                    } else {
                        // it was already there
                        free(s);
                    }
                }
            }
        } else {
            // it a block in the z axis (possibly with height 1)
            // find the block index of the block below it
            int block = find_block(blocks[b].x1, blocks[b].y1, blocks[b].z1 - 1);
            if (block != -1) {
                // add it to the supporting list
                struct support *s = malloc(sizeof(struct support));
                s->id = block; s->next = blocks[b].supporting;

                // check if already in the list
                struct support *p = blocks[b].supporting;
                while (p != NULL) {
                    if (p->id == block) break;
                    p = p->next;
                }
                if (p == NULL) {
                    blocks[b].supporting = s;
                } else {
                    // it was already there
                    free(s);
                }
            }
        }
    }
}

int can_disintegrate(int b_id) {
    int is_supporting = 0; // false
    int can_disintegrate = 1;
    for (int b = 0; b < block_count; b++) {
        struct support *p = blocks[b].supporting;
        int found = 0; // false
        int support_count = 0;
        while (p != NULL) {
            if (p->id == b_id) {
                is_supporting = 1; // true
                found = 1; // true
            }
            support_count++;
            p = p->next;
        }
        if (found && (support_count == 1)) can_disintegrate = 0; // false
    }
    if (! is_supporting) {
        return 1; // true
    } else {
        return can_disintegrate;
    }
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    printf("There are %d blocks in the air.\n", block_count);
    //if (debug) print_blocks();
    printf("Minimum is %dx%d\n", minx, miny);
    printf("Maximum is %dx%d\n", maxx, maxy);
    map_width = maxx - minx + 1;
    map_depth = maxy - miny + 1;

    // check if all blocks are 1x1xn, 1xnx1 or nx1x1
    check_blocks();

    // let the blocks fall and build the tower
    fall_blocks();

    // determine which blocks can be vaporized
    determine_support();
#if 0
    printf("--- Showing the supporting blocks ---\n");
    for (int b = 0; b < block_count; b++) {
        printf("block %c ", b + 'A'); print_block(blocks[b]);
        printf(" is supported by block(s): ");
        struct support *p = blocks[b].supporting;
        while (p != NULL) {
            printf("%c", p->id + 'A');
            if (p->next != NULL) printf(", ");
            p = p->next;
        }
        printf("\n");
    }
#endif
    // determine which blocks are not singly supporting some other block
    int count = 0;
    for (int b = 0; b < block_count; b++) {
        int disintegrate = can_disintegrate(b);
        if (disintegrate == 1) {
            if (debug) printf("Block %d can be disintegrated\n", b);
            count++;
        }
    }
    printf("The count of individual blocks that can be disintegated is %d\n", count);

    printf("Info: the solution for the sample data should be %ld\n", 5L);
    printf("Info: the solution for the actual data should be %ld\n", 428L);
    return EXIT_SUCCESS;
}

