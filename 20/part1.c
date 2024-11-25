
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
#define TYPE_BROADCASTER 0
#define TYPE_FLIPFLIP    1
#define TYPE_CONJUNCTION 2
#define TYPE_UNKNOWN     3
#define STATE_OFF        0
#define STATE_ON         1
char *type_chars = " %&?";
#define MAX_MODULES 100
struct module {
    char *name;
    int type;
    int state;
    struct input {
        int index;
        int level;
        struct input *next;
    } *inputs;
    struct dest {
        char *name;
        int index;
        struct dest *next;
    } *destinations;
} modules[MAX_MODULES];
int module_count = 0;
int module_start = -1;

#define LEVEL_LOW     0
#define LEVEL_HIGH    1
#define LEVEL_UNKNOWN 2
char *level_names[] = { "low", "high", "unknown" };
struct signal {
    int from;
    int to;
    int level;
    struct signal *next;
};

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
            char *p = line;
            // parse the module definition
            int type = TYPE_UNKNOWN;
            switch (*p) {
            case '%': type = TYPE_FLIPFLIP; p++; break;
            case '&': type = TYPE_CONJUNCTION; p++; break;
            default: type = TYPE_BROADCASTER; break;
            }
            modules[module_count].type = type;

            char buf[20];
            memset(buf, 0, array_count(buf));  int buf_index = 0;
            while (*p != ' ') { buf[buf_index++] = *p++; }
            modules[module_count].name = malloc(strlen(buf) + 1);
            strcpy(modules[module_count].name, buf);
            if (strcmp(buf, "broadcaster") == 0) module_start = module_count;

            if (strncmp(p, " -> ", 4) == 0) p += 4; else assert(0 && "expected ' -> '");

            struct dest **last_destination = &modules[module_count].destinations;
            while (*p) {
                memset(buf, 0, array_count(buf));  int buf_index = 0;
                while (isalpha(*p)) buf[buf_index++] = *p++;
                struct dest *d = malloc(sizeof(struct dest));
                d->name = malloc(strlen(buf) + 1);
                strcpy(d->name, buf);
                d->index = -1;
                d->next = NULL;

                // skip separators
                while ((*p == ',') || (*p == ' ')) p++;

                // store the destination at end of list
                *last_destination = d;
                last_destination = &d->next;
            }

            modules[module_count].state = STATE_OFF;
            modules[module_count].inputs = NULL;

            module_count++;
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

void convert_dest_to_index(void) {
    for (int i = 0; i < module_count; i++) {
         struct dest *d = modules[i].destinations;
         while (d != NULL) {
            for (int j = 0; j < module_count; j++) {
                if (strcmp(d->name, modules[j].name) == 0) {
                    d->index = j;
                }
            }
            if (d->index == -1) printf("Warning: could not find module '%s'\n", d->name);
            d = d->next;
        }
    }
}

void fill_conjunction_inputs(void) {
    for (int m = 0; m < module_count; m++) {
         if (modules[m].type == TYPE_CONJUNCTION) {
            for (int m2 = 0; m2 < module_count; m2++) {
                struct dest *d = modules[m2].destinations;
                while (d != NULL) {
                    if (d->index == m) {
                        struct input *i = malloc(sizeof(struct input));
                        i->index = m2;
                        i->level = LEVEL_LOW;
                        i->next = modules[m].inputs;
                        modules[m].inputs = i;
                    }
                    d = d->next;
                }
            }
         }
    }
}

void print_modules(void) {
    printf("MODULES (%d):\n", module_count);
    for (int i = 0; i < module_count; i++) {
         printf("%c%s -> ", type_chars[modules[i].type], modules[i].name);
         struct dest *d = modules[i].destinations;
         while (d != NULL) {
            printf("%s", (d->index != -1) ? modules[d->index].name : d->name);
            if (d->next != NULL) printf(", ");
            d = d->next;
         }
         if (modules[i].type == TYPE_CONJUNCTION) {
            printf(" [ ");
            struct input *in = modules[i].inputs;
            while (in != NULL) {
                printf("%s", modules[in->index].name);
                if (in->next != NULL) printf(", ");
                in = in->next;
            }
            printf(" ]");
         }
         printf("\n");
    }
}

void print_signals(struct signal *sig) {
    struct signal *s = sig;
    while (s != NULL) {
        printf("%s -%s-> %s\n", modules[s->from].name, level_names[s->level], modules[s->to].name);
        s = s->next;
    }
}

void reset_states(void) {
    for (int m = 0; m < module_count; m++) {
        switch (modules[m].type) {
        case TYPE_BROADCASTER:
            break;
        case TYPE_FLIPFLIP:
            modules[m].state = STATE_OFF;
            break;
        case TYPE_CONJUNCTION: {
                struct input *i = modules[m].inputs;
                while (i != NULL) {
                    i->level = LEVEL_LOW;
                    i = i->next;
                }
            }
            break;
        }
    }
}

struct signal *new_signal(int from, int to, int level) {
    struct signal *n = malloc(sizeof(struct signal));
    n->from = from; n->to = to; n->level = level;
    n->next = NULL;

    return n;
}

struct signal *get_signals(int moduleid, int from, int level) {
    (void)level;
    struct signal *signals = NULL;
    struct signal **last = &signals;
    switch (modules[moduleid].type) {
    case TYPE_BROADCASTER: {
        struct dest *d = modules[moduleid].destinations;
        while (d != NULL) {
            struct signal *n = new_signal(moduleid, d->index, LEVEL_LOW);

            *last = n;
            last = &n->next;

            d = d->next;
        }
        }
        break;
    case TYPE_FLIPFLIP: {
            if (level == LEVEL_LOW) {
                // if the pulse level is low, the state is toggled, and a new pulse is sent to all destinations
                modules[moduleid].state = (modules[moduleid].state == STATE_OFF) ? STATE_ON : STATE_OFF;
                int pulse_level = (modules[moduleid].state == STATE_ON) ? LEVEL_HIGH : LEVEL_LOW;
                struct dest *d = modules[moduleid].destinations;
                while (d != NULL) {
                    struct signal *n = new_signal(moduleid, d->index, pulse_level);

                    *last = n;
                    last = &n->next;

                    d = d->next;
                }
            } else {
                // LEVEL_HIGH is ignored
            }
        }
        break;
    case TYPE_CONJUNCTION: {
            // update the input levels
            struct input *in = modules[moduleid].inputs;
            while (in != NULL) {
                if (in->index == from) in->level = level;
                in = in->next;
            }
            // check if all inputs are high
            in = modules[moduleid].inputs;
            int all_high = 1; // true
            if (debug) printf("inputs = [");
            while (in != NULL) {
                if (debug) printf("%s = %s, ", modules[in->index].name, level_names[in->level]);
                if (in->level == LEVEL_LOW) all_high = 0; // flse
                in = in->next;
            }
            if (debug) printf("]");
            // if all_high, send a low pulse, otherwise send a high pulse
            int pulse_level = (all_high) ? LEVEL_LOW : LEVEL_HIGH;
            struct dest *d = modules[moduleid].destinations;
            while (d != NULL) {
                struct signal *n = new_signal(moduleid, d->index, pulse_level);

                *last = n;
                last = &n->next;

                d = d->next;
            }
        }
        break;
    default:
        if (debug) printf("Unknown module type for module '%s'\n", modules[moduleid].name);
        break;
    }
    return signals;
}

void press_button(struct signal *start) {
    struct signal **signals_end = &start->next;
    struct signal *current = start;
    while (current != NULL) {
        struct signal *add = get_signals(current->to, current->from, current->level);
        if (add != NULL) {
            *signals_end = add;
            while (add != NULL) {
                signals_end = &add->next;
                add = add->next;
            }
        }
        assert(*signals_end == NULL);
        current = current->next;
    }
    if (debug) printf("----------Signals----------\n");
    if (debug) print_signals(start);
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    convert_dest_to_index();
    fill_conjunction_inputs();
    if (debug) print_modules();
    assert(module_start != -1);
    // implement progressing the pulses
    int sum_low = 0, sum_high = 0;
    for (int i = 0; i < 1000; i++) {
        struct signal *start = malloc(sizeof(struct signal));
        start->from = -1; start->to = module_start; start->level = LEVEL_LOW; start->next = NULL;

        if (debug) printf("Press %d: ", i + 1);
        press_button(start);

        int low_count = 0, high_count = 0;
        struct signal *s = start;
        while (s != NULL) {
            if (s->level == LEVEL_LOW) low_count++; else high_count++;
            s = s->next;
        }
        sum_low += low_count;
        sum_high += high_count;

        // cleanup the pulses list
        s = start;
        while (s != NULL) {
            struct signal *t = s;
            s = s->next;
            free(t);
        }
    }
    printf("low_count = %d, high_count = %d\n", sum_low, sum_high);
    printf("multiply gives %d\n", sum_low * sum_high);

    printf("Info: the solution for the sample data is %ld\n", 11687500L);
    printf("Info: the solution for the actual data is %ld\n", 818723272L);
    return EXIT_SUCCESS;
}

