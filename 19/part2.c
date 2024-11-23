
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
#define MAX_WORKFLOWS 1000
#define INDEX_REJECTED (-1)
#define INDEX_ACCEPTED (-2)
struct workflow {
    char *name;
    struct rule {
        char *check;
        char *target;
        int target_index;
        struct rule *next;
    } *rules;
} workflows[MAX_WORKFLOWS];
int workflow_count = 0;
int workflow_start = 0;

#define MAX_PARTS 1000
struct part {
    int x;
    int m;
    int a;
    int s;
} parts[MAX_PARTS];
int part_count = 0;

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

        static int finished_workflows = 0;
        if (strlen(line) != 0) {
            if (!finished_workflows) {
                char *p = line;
                // parse the workflow
                char buf[20] = {0};
                int buf_index = 0;
                while (*p != '{') buf[buf_index++] = *p++;
                p++; // skip the '{'
                workflows[workflow_count].name = malloc(strlen(buf) + 1);
                strcpy(workflows[workflow_count].name, buf);
                if (strcmp(buf, "in") == 0) workflow_start = workflow_count;
                struct rule **last_rule = &workflows[workflow_count].rules;
                while (*p != '}') {
                    struct rule *r = malloc(sizeof(struct rule));
                    r->target_index = -1;
                    r->next = NULL;
                    memset(buf, 0, array_count(buf));
                    buf_index = 0;
                    while ((*p != ',') && (*p != '}')) buf[buf_index++] = *p++;
                    if (*p == ',') p++; // skip the rule separator
                    char *sep = strchr(buf, ':');
                    if (sep == NULL) {
                        // last rule, only a target is present
                        r->check = NULL;
                        r->target = malloc(strlen(buf) + 1);
                        strcpy(r->target, buf);
                    } else {
                        r->target = malloc(strlen(sep + 1) + 1);
                        strcpy(r->target, sep + 1);
                        *sep = 0;
                        r->check = malloc(strlen(buf) + 1);
                        strcpy(r->check, buf);
                    }

                    // append at the end and update the end
                    *last_rule = r;
                    last_rule = &r->next;
                }

                workflow_count++;
            } else {
                // parse a part
                char *p = line;
                assert(*p == '{'); p++;
                while (*p != '}') {
                    char x = *p++;
                    assert(*p == '='); p++;
                    int val = 0;
                    while (isdigit(*p)) {
                        val = 10 * val + (*p - '0');
                        p++;
                    }
                    switch (x) {
                    case 'x': parts[part_count].x = val; break;
                    case 'm': parts[part_count].m = val; break;
                    case 'a': parts[part_count].a = val; break;
                    case 's': parts[part_count].s = val; break;
                    default: assert(0 && "unknown part attribute found");
                    }
                    if (*p == ',') p++; // skip separator

                }
                part_count++;
            }
        } else if (strlen(line) == 0) {
            finished_workflows = 1;
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

void print_workflows(void) {
    printf("WORKFLOWS (%d):\n", workflow_count);
    for (int i = 0; i < workflow_count; i++) {
         printf("%s: ", workflows[i].name);
         struct rule *r = workflows[i].rules;
         while (r != NULL) {
            if (r->check != NULL) {
                printf("%s -> ", r->check);
            }
            printf("%s", r->target);
            if (r->target_index >= -2) printf("(%d)", r->target_index);
            if (r->next != NULL) printf(", ");
            r = r->next;
         }
         printf("\n");
    }
}

void print_part(struct part p) {
    printf("{x = %d, m = %d, a = %d, s = %d}", p.x, p.m, p.a, p.s);
}

void print_parts(void) {
    printf("PARTS (%d):\n", part_count);
    for (int i = 0; i < part_count; i++) {
        print_part(parts[i]);
        printf("\n");
    }
}

int find_wf_index(char *wf_name) {
    int wf_index = -1;
    for (int i = 0; i < workflow_count; i++) {
         if (strcmp(workflows[i].name, wf_name) == 0) {
            wf_index = i;
            break;
         }
    }
    assert(wf_index != -1);
    return wf_index;
}

void fill_target_indices(void) {
    for (int wf = 0; wf < workflow_count; wf++) {
        struct rule *r = workflows[wf].rules;
        while (r != NULL) {
            if (r->target[0] == 'A') {
                r->target_index = INDEX_ACCEPTED;
            } else if (r->target[0] == 'R') {
                r->target_index = INDEX_REJECTED;
            } else {
                r->target_index = find_wf_index(r->target);
            }
            r = r->next;
        }
    }
}

int do_check(char *check, struct part p) {
    assert(check != NULL);
    if (debug) { printf("check = '%s', ", check); print_part(p); }
    int result = 0; // false

    char var = check[0];
    char operation = check[1];
    int var_value = 0;
    switch (var) {
    case 'x': var_value = p.x; break;
    case 'm': var_value = p.m; break;
    case 'a': var_value = p.a; break;
    case 's': var_value = p.s; break;
    default: assert(0 && "unknown part attribute in check expression");
    }
    int value = 0;
    char *cp = check + 2;
    while (isdigit(*cp)) { value = 10 * value + (*cp - '0'); cp++; }
    if (debug) printf(" [%d %c %d] ", var_value, operation, value);
    switch (operation) {
    case '>': result = (var_value > value); break;
    case '<': result = (var_value < value); break;
    default: assert(0 && "unknown operation in check expresssion");
    }

    if (debug) printf(" result = %d\n", result);
    return result;
}

struct range {
    int min, max;
};

struct part_ranges {
    struct range x_range;
    struct range m_range;
    struct range a_range;
    struct range s_range;
};

#define MAX_CHECKS 5000
static const char *check_stack[MAX_CHECKS];
static       int   invert_stack[MAX_CHECKS];
static const char *state_stack[MAX_CHECKS];
static int check_count = 0;
void check_push(const char *state, const char *check, int invert_check) {
    assert(check_count < MAX_CHECKS);
    check_stack[check_count] = check;
    invert_stack[check_count] = invert_check;
    state_stack[check_count] = state;
    check_count += 1;
}
const char *check_pop(void) {
    assert(check_count >= 1);
    return check_stack[--check_count];
}
void checks_print(void) {
    int index = 0;
    assert(check_count != 0);
    while (index < check_count) {
        if (invert_stack[index] != 0) printf("!");
        if (check_stack[index] == NULL) {
            printf("%s:ALWAYS", state_stack[index]);
        } else {
            printf("%s:%s", state_stack[index], check_stack[index]);
        }
        if (index < check_count - 1) {
             printf(", ");
        }
        index += 1;
    }
    printf("\n");
}

void print_ranges(const char *prefix, struct part_ranges ranges) {
    long total = 1;
    total *= (long)(ranges.x_range.max - ranges.x_range.min + 1);
    total *= (long)(ranges.m_range.max - ranges.m_range.min + 1);
    total *= (long)(ranges.a_range.max - ranges.a_range.min + 1);
    total *= (long)(ranges.s_range.max - ranges.s_range.min + 1);
    if (prefix != NULL) printf("%s", prefix);
    printf("[ ");
    printf("x = [ %d->%d ], ", ranges.x_range.min, ranges.x_range.max);
    printf("m = [ %d->%d ], ", ranges.m_range.min, ranges.m_range.max);
    printf("a = [ %d->%d ], ", ranges.a_range.min, ranges.a_range.max);
    printf("s = [ %d->%d ] ", ranges.s_range.min, ranges.s_range.max);
    printf("] = %ld\n", total);
}

int get_value(const char *str) {
    int value = 0;
    while (isdigit(*str)) {
        value = 10 * value + (*str - '0');
        str++;
    }
    return value;
}

void update_ranges(const char *check, int invert_check, struct part_ranges *ranges) {
    // update is only needed when there is a 'check'
    if (check != NULL) {
        struct range *update_range = NULL;
        switch (check[0]) {
            case 'x': update_range = &(ranges->x_range); break;
            case 'm': update_range = &(ranges->m_range); break;
            case 'a': update_range = &(ranges->a_range); break;
            case 's': update_range = &(ranges->s_range); break;
            default: fprintf(stderr, "Error: unknown part value '%c' in check '%s'.\n", check[0], check);
        }
        int value = get_value(check + 2);;
        if (value == 0) fprintf(stderr, "Error: could parse value in check '%s'.\n", check);
        //if (debug) printf("Debug: check = '%s', value = %d\n", check, value);
        switch (check[1]) {
            case '>': {
                if (!invert_check) {
                    if (update_range->min < value) update_range->min = value + 1;
                } else {
                    if (update_range->max > value) update_range->max = value;
                }
                //print_ranges(" --> ", *ranges);
                assert(update_range->min <= update_range->max);
            } break;
            case '<': {
                if (!invert_check) {
                    if (update_range->max > value) update_range->max = value - 1;
                } else {
                    if (update_range->min < value) update_range->min = value;
                }
                //print_ranges(" --> ", *ranges);
                assert(update_range->min <= update_range->max);
            } break;
            default: fprintf(stderr, "Error: unknown comparison '%c' in check '%s'.\n", check[1], check);
        }
    }
}

long count_combinations(struct part_ranges ranges) {
    long count = 1;
    count *= (long)(ranges.x_range.max - ranges.x_range.min + 1);
    count *= (long)(ranges.m_range.max - ranges.m_range.min + 1);
    count *= (long)(ranges.a_range.max - ranges.a_range.min + 1);
    count *= (long)(ranges.s_range.max - ranges.s_range.min + 1);
    return count;
}

long combinations;
long find_paths_to_accept(int wf_index, long path_count, struct part_ranges ranges) {
    // TODO: update 'ranges'
    struct rule *r = workflows[wf_index].rules;
    struct part_ranges saved_ranges = ranges;
    int inverse_checks_count = 0;
    while (r != NULL) {
        struct part_ranges new_ranges = saved_ranges;
        update_ranges(r->check, 0, &new_ranges);
        if (r->target_index == INDEX_ACCEPTED) {
            check_push(workflows[wf_index].name, r->check, 0);
            if (debug) checks_print();
            if (debug) print_ranges("Accept: ", new_ranges);
            // update the combinations count with the current range
            combinations += count_combinations(new_ranges);
            check_pop();
            path_count += 1;
        } else if (r->target_index == INDEX_REJECTED) {
            // do nothing because if reject
        } else {
            check_push(workflows[wf_index].name, r->check, 0);
            path_count = find_paths_to_accept(r->target_index, path_count, new_ranges);
            check_pop();
        }
        // for the next rule in this workflow, the current condition is inverted
        new_ranges = saved_ranges;
        update_ranges(r->check, 1, &saved_ranges);
        check_push(workflows[wf_index].name, r->check, 1);
        inverse_checks_count += 1;
        r = r->next;
    }
    while (inverse_checks_count != 0) {
        check_pop();
        inverse_checks_count -= 1;
    }
    return path_count;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // fill in the indices so targeting becomes faster
    fill_target_indices();
    if (debug) print_workflows();

    // find paths to Accept states
    combinations = 0;
    struct part_ranges ranges = { { 1, 4000 }, { 1, 4000 }, { 1, 4000 }, { 1 ,4000 } };
    long accept_paths = find_paths_to_accept(workflow_start, 0, ranges);
    printf("Found %ld paths from 'in' to an Accept state.\n", accept_paths);

    printf("              LONG_MAX is %ld\n", LONG_MAX);
    printf("distinct combinations is  %ld\n", combinations);

    printf("Info: the solution for the sample data is %ld\n", 167409079868000L);
    printf("Info: the solution for the actual data is %ld\n", 131550418841958L);
    return EXIT_SUCCESS;
}

