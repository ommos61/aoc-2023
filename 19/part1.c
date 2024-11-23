
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
#define MAX_WORKFLOWS 1000
struct workflow {
    char *name;
    struct rule {
        char *check;
        char *target;
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

int do_workflow(int wf_index, struct part p) {
    int accepted = 0;
    if (debug) printf("%s ", workflows[wf_index].name);
    struct rule *r = workflows[wf_index].rules;
    char *next_workflow = NULL;
    while (r != NULL) {
        if (r->check == NULL) {
            // end of rules
            next_workflow = r->target;
        } else if (do_check(r->check, p)) {
            next_workflow = r->target;
            break;
        }

        r = r->next;
    }
    assert(next_workflow != NULL);
    if (strcmp(next_workflow, "A") == 0) {
        accepted = 1;
    } else if (strcmp(next_workflow, "R") == 0) {
        accepted = 0;
    } else {
        accepted = do_workflow(find_wf_index(next_workflow), p);
    }

    if (strcmp(workflows[wf_index].name, "in") == 0) if (debug) printf("\n");
    return accepted;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    if (debug) print_workflows();
    if (debug) print_parts();
    // execute the workflows for all parts
    long rating_sum = 0;
    for (int p = 0; p < part_count; p++) {
        int accepted = do_workflow(workflow_start, parts[p]);
        if (accepted) {
            long rating = parts[p].x + parts[p].m + parts[p].a + parts[p].s;
            rating_sum += rating;
        }
    }
    printf("Sum of all accepted ratings is %ld\n", rating_sum);

    return EXIT_SUCCESS;
}

