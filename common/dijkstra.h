#ifndef _DIJKSTRA_H_
#define _DIJKSTRA_H_
//==================================================
// Implementation of the Dijkstra search algorithm
#include "dict.h"
#include "queue.h"
#include <stdio.h>
#include <limits.h>

// funtions to be implemented by user
struct state *nextState(struct state *current, struct state *next);
int stateCost(struct state *from, struct state *to);
int stateCompare(const void *v1, const void *v2);
int costCompare(const void *v1, const void *v2);
void statePrint(const char *prefix, const void *v);

// functions implemented in this file
struct state *dijkstra(struct state *start, struct state *end, int (*isEndState)(struct state*));
void stateFree(struct state *s);
#endif /* _DIJKSTRA_H_ */

#ifdef DIJKSTRA_IMPLEMENTATION
extern int debug;

struct state *dijkstra(struct state *start, struct state *end, int (*isEndState)(struct state*)) {
    struct dict *seen = dictCreate(1000000, stateCompare);
    queue q = queueCreate(0, costCompare);
    struct state *startState = start;
    struct state *endState = end;
    dictPut(seen, startState, startState);
    queuePush(q, startState);

    long states_handled = 0;
    while (queueLength(q) > 0) {
        struct state *current = (struct state*)queuePop(q);
        if (debug) statePrint("Current state:", current);
        states_handled++;
        if (isEndState(current)) {
            printf("END STATE REACHED!!!!!!!!!!!!!\n");
            if (current->cost < endState->cost) {
                endState->cost = current->cost;
                endState->prev = current->prev;
            }
            if (debug) printf("States handled: %ld\n", states_handled);
            if (debug) printf("States still queued: %d\n", queueLength(q));
            break;
        } else {
            struct state *next = NULL;
            while ((next = nextState(current, next)) != NULL) {
                if (debug) statePrint("Next state:", next);
                struct state *seennext = dictGet(seen, next);
                if (debug && (seennext != NULL)) statePrint("Seen Next state:", seennext);
                int next_cost = current->cost + stateCost(current, next);
                if (debug) printf("Next cost is %d\n", next_cost);
                if (seennext == NULL) {
                    if (debug) printf("state not seen yet\n");
                    next->cost = next_cost;
                    next->prev = current;
                    dictPut(seen, next, next);
                    queuePush(q, next);
                } else {
                    if (debug) printf("state already seen with cost %d\n", seennext->cost);
                    if (next_cost < seennext->cost) {
                        printf("found lower cost for (%d, %d, %c)\n", next->posx, next->posy, dirchars[next->dir]);
                        seennext->cost = next_cost;
                        seennext->prev = current;
                        next->cost = next_cost;
                        next->prev = current;
                        queuePush(q, next);
                    } else {
                        stateFree(next);
                    }
                }
            }
            //char buf[10];
            //if (debug) fgets(buf, 5, stdin);
            //return endState;
        }
    }
    printf("Queue lenght = %d\n", queueLength(q));

    return endState;
}

void stateFree(struct state *s) {
    if (s != NULL) free(s);
}
#endif
