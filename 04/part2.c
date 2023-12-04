
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
    int match_count;
    int amount;
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
            cards[card_count]->match_count = 0;
            cards[card_count]->amount = 1;
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

int card_match_count(struct card *c) {
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
    return count;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // Determine the match count for all the cards and update the amount of other cards we have copies
    for (int i = 0; i < card_count; i++) {
        int count = card_match_count(cards[i]);
        if (count > 0) {
            for (int j = i + 1; (j <= i + count) && (j < card_count); j ++) {
                cards[j]->amount += cards[i]->amount;
            }
        }
    }
    long total_amount = 0;
    for (int i = 0; i < card_count; i++) {
        if (debug) printf("Card %d: %d\n", cards[i]->card_num, cards[i]->amount);
        total_amount += cards[i]->amount;
    }
    printf("Total amount of cards is %ld\n", total_amount);

    return EXIT_SUCCESS;
}

