
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
#define MAX_CARDS 300
struct number {
    int num; struct number *next;
};
struct card {
    int card_num;
    struct number *win;
    struct number *mine;
};
struct card *cards[MAX_CARDS];
int card_count = 0;

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

        int value = 0;
        if (sscanf(line, "Card %d:", &value) == 1) {
            // handle one line
            cards[card_count] = malloc(sizeof(struct card));
            cards[card_count]->card_num = value;
            cards[card_count]->win = NULL;
            cards[card_count]->mine = NULL;
            // read the win list
            char *p = strchr(line, ':') + 1;
            while (*p != '|') {
                while (*p == ' ') p++;
                sscanf(p, "%d", &value);
                struct number *a = malloc(sizeof(struct number));
                a->num = value;
                a->next = cards[card_count]->win;
                cards[card_count]->win = a;
                while (isdigit(*p)) p++;
                while (*p == ' ') p++;
            }
            // skip the separator
            p++;
            // read the card numbers
            while (*p) {
                while (*p == ' ') p++;
                sscanf(p, "%d", &value);
                struct number *a = malloc(sizeof(struct number));
                a->num = value;
                a->next = cards[card_count]->mine;
                cards[card_count]->mine = a;
                while (isdigit(*p)) p++;
                while (*p == ' ') p++;
            }
            card_count++;
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

long card_points(struct card *c) {
    int count = 0;
    struct number *w = c->win;
    while (w) {
        struct number *m = c->mine;
        while (m) {
            if (w->num == m->num) {
                count++;
                break;
            }
            m = m->next;
        }
        w = w->next;
    }
    if (debug) printf("match count = %d\n", count);
    return (count == 0) ? 0 : 1 << (count - 1);
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // Sum the point values of the card
    long sum = 0;
    for (int i = 0; i < card_count; i++) {
        long value = card_points(cards[i]);
        sum += value;
    }
    printf("Sum of the point values is %ld\n", sum);

    return EXIT_SUCCESS;
}

