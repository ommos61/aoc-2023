#ifndef _DIJKSTRA_H_
#define _DIJKSTRA_H_
//==================================================
// Implementation of the Dijkstra search algorithm
#include "dict.h"
#include "queue.h"
#include <stdio.h>
#include <limits.h>

typedef struct state *state;

state dijkstra(state start, state end, int (*isEndState)(state));
#endif /* _DIJKSTRA_H_ */

#ifdef DIJKSTRA_IMPLEMENTATION
extern int debug;

state dijkstra(state start, state end, int (*isEndState)(state)) {
    dict seen = dictCreate(0, stateCompare);
    queue q = queueCreate(0, costCompare);
    state startState = start;
    state endState = end;
    dictPut(seen, startState);
    queuePush(q, startState);

    long states_handled = 0;
    while (queueLength(q) > 0) {
        const state current = (const state)queuePop(q);
        if (debug) statePrint("Current state:", current);
        states_handled++;
        //if (stateCompare(current, endState) == 0) {
        if (isEndState(current)) {
            if (current->cost < endState->cost) {
                endState->cost = current->cost;
                endState->prev = current->prev;
            }
            if (debug) printf("States handled: %ld\n", states_handled);
            if (debug) printf("States still queued: %d\n", queueLength(q));
            break;
        } else {
            state next = NULL;
            while ((next = nextState(current, next)) != NULL) {
                state next1 = dictGet(seen, next);
                long next_cost = current->cost + 1;
                if (debug) statePrint("Next state:", next);
                if (next1 == NULL) {
                    next->cost = next_cost;
                    dictPut(seen, next);
                    queuePush(q, next);
                } else {
                    if (next_cost < next1->cost) {
                        next1->cost = next_cost;
                        next1->prev = current;
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

    return endState;
}
#endif
