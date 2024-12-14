
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 1; // 0 = no debug output, 1 = some extra debug output
#define MAX_STONES 500
struct stone {
    long px, py, pz;
    long vx, vy, vz;
    long time;
} stones[MAX_STONES];
unsigned stone_count = 0;

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char line[LINE_LENGTH];
    unsigned line_count = 0;
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        if (strlen(line) != 0) {
            // store the line data
            long px, py, pz, vx, vy, vz;
            sscanf(line, "%ld, %ld, %ld @ %ld, %ld, %ld", &px, &py, &pz, &vx, &vy, &vz);
            stones[stone_count].px = px;
            stones[stone_count].py = py;
            stones[stone_count].pz = pz;
            stones[stone_count].vx = vx;
            stones[stone_count].vy = vy;
            stones[stone_count].vz = vz;
            stones[stone_count].time = 0;
            stone_count += 1;
        } else if (errno != 0) {
            perror("sscanf");
        } else {
            fprintf(stderr, "Unexpected input format '%s'.\n", line);
        }

        line_count += 1;
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

void print_stones(void) {
    for (unsigned l = 0; l < stone_count; l++) {
         printf("(%ld, %ld, %ld) @ ", stones[l].px, stones[l].py, stones[l].pz);
         printf("(%ld, %ld, %ld)\n", stones[l].vx, stones[l].vy, stones[l].vz);
    }
}

#define STATUS_INTERSECT 0
#define STATUS_PARALLEL  1
#define STATUS_PAST1     2
#define STATUS_PAST2     3
#define STATUS_PASTBOTH  4
#define STATUS_UNKNOWN   5
struct vec2 {
    float px, py;
    int status;
};
struct vec2 *intersect(int stone1, int stone2) {
    struct vec2 *res = malloc(sizeof(struct vec2));
    res->status = STATUS_UNKNOWN;
    res->px = 0; res->py = 0;

    // check if parallel
    long px1 = stones[stone1].px, px2 = stones[stone2].px;
    long py1 = stones[stone1].py, py2 = stones[stone2].py;
    long vx1 = stones[stone1].vx, vx2 = stones[stone2].vx;
    long vy1 = stones[stone1].vy, vy2 = stones[stone2].vy;
    float slope1 = ((float)vy1) / ((float)vx1);
    float slope2 = (float)vy2 / (float)vx2;
    if (fabsf(slope1 - slope2) < 0.00001) {
        res->status = STATUS_PARALLEL;
    }

    // not parallel, caclulate intersection
    if (res->status != STATUS_PARALLEL) {
        // t2 * (vx2 * vy1 - vy2 * vx1) = vx1 * py2 - vx1 * py1 - vy1 * px2 + vy1 * px1
        float t2 = (float)(vx1 * py2 - vx1 * py1 - vy1 * px2 + vy1 * px1) / (float)(vx2 * vy1 - vy2 * vx1);
        // t1 = (px2 - px1 + t2 * vx2) / vx1
        float t1 = ((float)px2 - (float)px1 + t2 * (float)vx2) / (float)vx1;
        //printf("t1 = %.3f\n", t1);
        //printf("t2 = %.3f\n", t2);
        res->px = (float)px1 + t1 * (float)vx1;
        res->py = (float)py1 + t1 * (float)vy1;
        if ((t1 < 0.0f) && (t2 < 0.0f)) {
            res->status = STATUS_PASTBOTH;
        } else if (t1 < 0.0f) {
            res->status = STATUS_PAST1;
        } else if (t2 < 0.0f) {
            res->status = STATUS_PAST2;
        } else {
            res->status = STATUS_INTERSECT;
        }
    }
    return res;
}

struct stone intersectXY(struct stone a, struct stone b) {
    struct stone intersect = {0};
    // (vby*vax - vay*vbx) * t1 = xb*vby - xa*vby + vbx*ya - vbx*yb
    long f = b.vy * a.vx - b.vx * a. vy;
    long t1 = b.px * b.vy - a.px * b.vy + b.vx * a.py - b.vx * b.py;
    if (f != 0) {
        if ((t1 % f) == 0) {
            t1 = t1 / f;
            if (t1 > 0) {
                intersect.px = a.px + t1 * a.vx;
                intersect.py = a.py + t1 * a.vy;
                intersect.time = t1;
            }
        }
    }

    return intersect;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("There are %d stones.\n", stone_count);
    //if (debug) print_stones();

    // check the intersections
    struct stone rock = {0};
#define MAX_TRIES 20
#define RANGE_MAX 500
    unsigned tries = MAX_TRIES;
    while (tries > 0) {
        // select 4 random hailstones
        unsigned stone_ids[4] = {0};
        for (unsigned i = 0; i < array_count(stone_ids); i++) {
            int selected = 0;
            while (!selected) {
                unsigned id = random() % stone_count;
                int same = 0;
                for (unsigned j = 0; j < i; j++) {
                    if (id == stone_ids[j]) same = 1;
                }
                if (!same) { stone_ids[i] = id; selected = 1; }
            }
        }
        if (debug) printf("Selected %d, %d, %d and %d\n", stone_ids[0], stone_ids[1], stone_ids[2], stone_ids[3]);
        int found_intersection = 0;
        for (long deltax = -RANGE_MAX; deltax <= RANGE_MAX; deltax++) {
            found_intersection = 0;
            for (long deltay = -RANGE_MAX; deltay <= RANGE_MAX; deltay++) {
                struct stone h0 = stones[stone_ids[0]];
                h0.vx += deltax; h0.vy += deltay;
                struct stone intersects[3] = {0};
                int all_intersect = 1;
                for (unsigned i = 0; i < array_count(intersects); i++) {
                    struct stone st = stones[stone_ids[i + 1]];
                    st.vx += deltax; st.vy += deltay;
                    intersects[i] = intersectXY(st, h0);
                    if ((intersects[i].px == 0) && (intersects[i].py == 0)) {
                        all_intersect = 0;
                    }
                }
                if (all_intersect &&
                    (intersects[0].px == intersects[1].px) && (intersects[0].py == intersects[1].py) &&
                    (intersects[0].px == intersects[2].px) && (intersects[0].py == intersects[2].py)) {

                    rock.px = intersects[0].px; rock.py = intersects[0].py;
                    printf("--> intersection at (%ld, %ld)\n", rock.px, rock.py);
                    for (long deltaz = -RANGE_MAX; deltaz < RANGE_MAX; deltaz++) {
                        long z1 = stones[stone_ids[1]].pz + intersects[0].time * (stones[stone_ids[1]].vz + deltaz);
                        long z2 = stones[stone_ids[2]].pz + intersects[1].time * (stones[stone_ids[2]].vz + deltaz);
                        long z3 = stones[stone_ids[3]].pz + intersects[2].time * (stones[stone_ids[3]].vz + deltaz);

                        if ((z1 == z2) && (z2 == z3)) {
                            rock.pz = z1;
                            found_intersection = 1;
                            break;
                        }
                    }
                    if (found_intersection) break;
                }
            }
            if (found_intersection) break;
        }
        if (found_intersection) break;

        // try again if no solution found
        tries -= 1;
    }
    long solution = rock.px + rock.py + rock.pz;
    printf("Intersection should be at (24, 13 ,10) for the sample data\n");
    printf("The sum of the rock coordinates is %ld\n", solution);

    printf("Info: the solution for the sample data should be %ld\n", 47L);
    printf("Info: the solution for the actual data should be %ld\n", 716599937560103L);
    return EXIT_SUCCESS;
}

