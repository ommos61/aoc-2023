
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
int debug = 0; // 0 = no debug output, 1 = some extra debug output
#define MAX_LINES 500
struct line {
    long px, py, pz;
    long vx, vy, vz;
} lines[MAX_LINES];
int line_count = 0;
long min = 7, max = 27;

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char line[LINE_LENGTH];
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        if (strlen(line) != 0) {
            // store the line data
            long px, py, pz, vx, vy, vz;
            sscanf(line, "%ld, %ld, %ld @ %ld, %ld, %ld", &px, &py, &pz, &vx, &vy, &vz);
            lines[line_count].px = px;
            lines[line_count].py = py;
            lines[line_count].pz = pz;
            lines[line_count].vx = vx;
            lines[line_count].vy = vy;
            lines[line_count].vz = vz;
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

void print_lines(void) {
    for (int l = 0; l < line_count; l++) {
         printf("(%ld, %ld, %ld) @ ", lines[l].px, lines[l].py, lines[l].pz);
         printf("(%ld, %ld, %ld)\n", lines[l].vx, lines[l].vy, lines[l].vz);
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
struct vec2 *intersect(int line1, int line2) {
    struct vec2 *res = malloc(sizeof(struct vec2));
    res->status = STATUS_UNKNOWN;
    res->px = 0; res->py = 0;

    // check if parallel
    long px1 = lines[line1].px, px2 = lines[line2].px;
    long py1 = lines[line1].py, py2 = lines[line2].py;
    long vx1 = lines[line1].vx, vx2 = lines[line2].vx;
    long vy1 = lines[line1].vy, vy2 = lines[line2].vy;
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

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }
    if (strcmp(fname, "input.txt") == 0) {
        min = 200000000000000L;
        max = 400000000000000L;
    }

    readData(fname);

    printf("There are %d lines.\n", line_count);
    printf("Min is %ld and max is %ld\n", min, max);
    if (debug) print_lines();

    // check the intersections
    int intersect_count = 0;
    int combinations = 0;
    for (int l1 = 0; l1 < line_count; l1++) {
        for (int l2 = l1 + 1; l2 < line_count; l2++) {
            combinations++;
            struct vec2 *i = intersect(l1, l2);
            switch (i->status) {
            case STATUS_PARALLEL:
                if (debug) printf("%d and %d are parralel\n", l1, l2);
                break;
            case STATUS_PASTBOTH:
                if (debug) printf("%d and %d are intersecting in both their past\n", l1, l2);
                break;
            case STATUS_PAST1:
                if (debug) printf("%d and %d are intersecting in the past of the first\n", l1, l2);
                break;
            case STATUS_PAST2:
                if (debug) printf("%d and %d are intersecting in the past of the second\n", l1, l2);
                break;
            case STATUS_INTERSECT:
                if ((i->px >= (float)min) && (i->px <= (float)max) &&
                    (i->py >= (float)min) && (i->py <= (float)max)) {
                    if (debug) printf("%d and %d are intersecting inside test area at (%.3f, %.3f)\n", l1, l2, i->px, i->py);
                    intersect_count++;
                } else {
                    if (debug) printf("%d and %d are intersecting outside test area at (%.3f, %.3f)\n", l1, l2, i->px, i->py);
                }
                break;
            }
        }
    }
    printf("Checked %d line combinations.\n", combinations);
    printf("The number of lines that intersect in the test area are %d\n", intersect_count);

    printf("Info: the solution for the sample data should be %ld\n", 2L);
    printf("Info: the solution for the actual data should be %ld\n", 11246L);
    return EXIT_SUCCESS;
}

