
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 50000
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output
char *sequence = NULL;

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
            if (sequence == NULL) {
                sequence = malloc(strlen(line) + 1);
                strcpy(sequence, line);
            } else {
                printf("Only one line of input is supported.\n");
            }
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

int calculate_hash(char *str, int len) {
    int hash = 0;
    for (int i = 0; i < len; i++) {
        hash += str[i];
        hash *= 17;
        hash %= 256;
    }

    return hash;
}

#define MAX_LABEL_LEN 10
struct lens {
    char label[MAX_LABEL_LEN + 1];
    int focus;
    struct lens *next;
};
struct lens *boxes[256] = {0};

void print_lens(struct lens *l) {
    printf("[%s %d] ", l->label, l->focus);
}

void print_boxes(void) {
    for (int i = 0; i < 256; i++) {
         if (boxes[i] != NULL) {
            printf("Box %d: ", i);
            struct lens *l = boxes[i];
            while (l != NULL) {
                print_lens(l);
                l = l->next;
            }
            printf("\n");
         }
    }
}

void delete_lens(int hash, char *label) {
    struct lens **p = &boxes[hash];
    while (*p != NULL) {
        //printf("In delete: current lens is '%s'\n", (*p)->label);
        if (strcmp((*p)->label, label) == 0) {
            //printf("In delete: found the lens '%s'\n", (*p)->label);
            //if (debug) { printf("Deleting: "); print_lens(*p); printf("\n"); }
            struct lens *temp = *p;
            //print_lens(temp); printf("\n");
            *p = temp->next;
            //printf("*p = %p\n", (void*)*p);
            if (debug && (*p != NULL)) { print_lens(*p); printf("\n"); }
            free(temp);
        } else {
            p = &((*p)->next);
        }
    }
}

void add_lens(int hash, char *label, int focus) {
    struct lens *p = boxes[hash], *last = NULL;
    int replaced = 0; // false
    while (p != NULL) {
        if (strcmp(p->label, label) == 0) {
            p->focus = focus;
            replaced = 1; // true
        }
        last = p;
        p = p->next;
    }
    if (!replaced) {
        // add a new lens at the end
        struct lens *new = malloc(sizeof(struct lens));
        strcpy(new->label, label);
        new->focus = focus;
        new->next = NULL;
        if (last == NULL) {
            boxes[hash] = new;
        } else {
            last->next = new;
        }
    }
}

long calculate_lenses_power(struct lens *lenses) {
    int power = 0;
    int index = 1;
    struct lens *l = lenses;
    while (l != NULL) {
        power += index * l->focus;
        index++;
        l = l->next;
    }
    return power;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // test the hash function on the test string
    #define TEST_STRING "HASH"
    int hash = calculate_hash(TEST_STRING, strlen(TEST_STRING));
    printf("The test string \"%s\" has hash %d\n", TEST_STRING, hash);

    // execute all the commands, placing ande removing lenses
    char *p = sequence;
    while (p != NULL) {
        // calculate the hash of the label
        char label[MAX_LABEL_LEN + 1] = {0};
        int label_index = 0;
        while (isalpha(*p)) {
            label[label_index] = *p;
            label_index++; p++;
        }
        label[label_index] = 0;
        int hash = calculate_hash(label, strlen(label));
        if (*p == '-') {
            //if (debug) printf("box %d: delete lens %s\n", hash, label);
            // remove the labeled lens from the box
            delete_lens(hash, label);
        } else if (*p == '=') {
            //if (debug) printf("box %d: add/replace lens %s\n", hash, label);
            // add the labeled lens with focus length to the box
            int focus = p[1] - '0';
            add_lens(hash, label, focus);
        } else {
            printf("label = %s, command = %c\n", label, *p);
            assert(0 && "unreachable");
        }

        if (debug) {
            char buf[20];
            sprintf(buf, "%s%c%c", label, *p, (*p == '=') ? p[1] : '\000');
            printf("After \"%s\":\n", buf);
            print_boxes();
            printf("\n");
        }
        
        // search for next command
        p = strchr(p, ',');
        if (p != NULL) p++;
    }

    // calculate the focus power of all the lenses
    long focus_power = 0;
    for (int b = 0; b < 256; b++) {
        int box_power = calculate_lenses_power(boxes[b]);

        focus_power += (b + 1) * box_power;
    }
    printf("Total focus power is %ld\n", focus_power);

    return EXIT_SUCCESS;
}

