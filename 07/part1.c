
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 1; // 0 = no debug output, 1 = some extra debug output
#define TYPE_FIVE 0
#define TYPE_FOUR 1
#define TYPE_FULLHOUSE 2
#define TYPE_THREE 3
#define TYPE_PAIRS 4
#define TYPE_PAIR 5
#define TYPE_HIGH 6
struct hand {
    char cards[6];
    int bid;
    int type;
    int rank;
};
#define MAX_HANDS 1500
struct hand hands[MAX_HANDS];
int hand_count = 0;
const char *allcards = "23456789TJQKA";

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
            // get the information from the hand
            strncpy(hands[hand_count].cards, line, 5);
            hands[hand_count].cards[5] = '\000';
            sscanf(line + 6, "%d", &hands[hand_count].bid);
            hand_count++;
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

const char *type2str(int type) {
    switch (type) {
    case TYPE_FIVE:
        return "FIVE";
    case TYPE_FOUR:
        return "FOUR";
    case TYPE_FULLHOUSE:
        return "FULLHOUSE";
    case TYPE_THREE:
        return "THREE";
    case TYPE_PAIRS:
        return "PAIRS";
    case TYPE_PAIR:
        return "PAIR";
    case TYPE_HIGH:
        return "HIGH";
    default:
        return "UNKNOWN";
    }
}

int get_type(struct hand h) {
    int result = -1;
    int same1_count = 0, same2_count = 0;
    int same1 = '\000', same2 = '\000';

    // sort the cards
    char sorted[6]; memset(sorted, 0, 6);
    for (unsigned int i = 0; i < strlen(allcards); i++) {
        for (int j = 0; j < 5; j++) {
            if (allcards[i] == h.cards[j]) {
                strncat(sorted, allcards + i, 1);
            }
        }
    }
    //printf("Sort: %s -> %s\n", h.cards, sorted);

    // determine same cards for sorted
    same1 = sorted[0]; same1_count = 1;
    for (int i = 1; i < 5; i++) {
        char c = sorted[i];
        if (c == same1) {
            same1_count++;
        } else if (c == same2) {
            same2_count++;
        } else if (same2 == '\000') {
            same2 = c; same2_count = 1;
        } else if (same1_count == 1) {
            same1 = c;
        } else if (same2_count == 1) {
            same2 = c;
        }
    }
    //printf("same1 = %c(%d), same2 = %c(%d)\n", same1, same1_count, same2, same2_count);
    int max = MAX(same1_count, same2_count);
    int min = MIN(same1_count, same2_count);
    if (same1_count == 5) {
        result = TYPE_FIVE;
    } else if (max == 4) {
        result = TYPE_FOUR;
    } else if ((max == 3) && (min == 2)) {
        result = TYPE_FULLHOUSE;
    } else if (max == 3) {
        result = TYPE_THREE;
    } else  if ((same1_count == 2) && (same2_count == 2)) {
        result = TYPE_PAIRS;
    } else if (max == 2) {
        result = TYPE_PAIR;
    } else {
        result = TYPE_HIGH;
    }

    return result;
}

int card_value(char c) {
    return strchr(allcards, c) - allcards;
}

int is_hand_lower(struct hand h, struct hand target) {
    int result = 0; // false

    if (h.type > target.type) {
        // it is determined by the type
        result = 1; // true
    } else if (h.type == target.type) {
        // it is determined by card values
        for (int i = 0; i < 5; i++) {
            int cv_h = card_value(h.cards[i]);
            int cv_t = card_value(target.cards[i]);
            if (cv_h < cv_t) {
                result = 1; // true
                break;
            } if (cv_h != cv_t) {
                // the target if lower
                break;
            }
        }
    }

    return result;
}

int determine_lowest_unranked(void) {
    int lowest = -1;

    for (int i = 0; i < hand_count; i++) {
        if (hands[i].rank != 0) {
            // skip already ranked ones
            continue;
        } else if (lowest == -1) {
            // this is the first unranked, so for now it is the lowest
            lowest = i;
        } else if (is_hand_lower(hands[i], hands[lowest])) {
            lowest = i;
        } else {
            // skip it is not lower
        }
    }

    assert(lowest != -1);
    return lowest;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // First determine the type of all hands, also clear the ranks
    for (int i = 0; i < hand_count; i++) {
        hands[i].type = get_type(hands[i]);
        hands[i].rank = 0;
//        printf("%s: %d %s\n", hands[i].cards, hands[i].bid, type2str(hands[i].type));
    }
    
    // determine the ranks of the hands
    for (int rank = 1; rank <= hand_count; rank++) {
        int index_of_lowest = determine_lowest_unranked();
        hands[index_of_lowest].rank = rank;
    }
    // determine the total winnings
    long winnings = 0;
    for (int i = 0; i < hand_count; i++) {
 //       printf("%s: %d\n", hands[i].cards, hands[i].rank);
        winnings += hands[i].bid * hands[i].rank;
    }
    printf("Total winnings is %ld\n", winnings);

    return EXIT_SUCCESS;
}

