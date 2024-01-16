
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
int debug = 1; // 0 = no debug output, 1 = some extra debug output
#define MAX_EDGES 5000
struct component {
    char *comp1;
    int index1;
    char *comp2;
    int index2;
} edges[MAX_EDGES];
int edge_count = 0;

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
            // store the connection information for each component
            char *p = strchr(line, ':');
            assert((p != NULL) && "line should contain ':' character");
            int len = p - line;
            char *comp = malloc(len + 1);
            strncpy(comp, line, len);
            comp[len] = '\000';

            p++; // skip colon
            while (isspace(*p)) p++; // skip spaces
            char buf[20];
            while (*p) {
                memset(buf, 0, array_count(buf)); 
                int buf_index = 0;
                while (isalpha(*p)) buf[buf_index++] = *p++;
                buf[buf_index] = '\000';

                edges[edge_count].comp1 = comp;
                edges[edge_count].comp2 = malloc(strlen(buf) + 1);
                strcpy(edges[edge_count].comp2, buf);

                edge_count++;
                assert((edge_count < MAX_EDGES) && "Edge storage overflow");
                while (isspace(*p)) p++;
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

void print_edge(int e) {
    printf("  %s -> %s\n", edges[e].comp1, edges[e].comp2);
}

int are_distinct(int e1, int e2, int e3) {
    return (e1 != e2) && (e1 != e3) && (e2 != e3);
}

#define MAX_COMPONENTS 5000
struct comp {
    char *name;
    int connections;
} components[MAX_COMPONENTS];
int component_count = 0;
int *visited = NULL;

void visit_all(int c, int e1, int e2, int e3) {
    visited[c] = 1; // true
    for (int e = 0; e < edge_count; e++) {
        if ((e == e1) || (e == e2) || (e == e3)) continue;
        int target = -1;
        if (c == edges[e].index1) {
            target = edges[e].index2;
        } else if (c == edges[e].index2) {
            target = edges[e].index1;
        } else {
        }
        if ((target != -1) && !visited[target]) {
            visit_all(target, e1, e2, e3);
        }
    }
}

int is_graph_connected(int e1, int e2, int e3) {
    if (visited == NULL) visited = malloc(component_count * sizeof(int));
    memset(visited, 0, component_count *sizeof(int));
    visit_all(edges[e1].index1, e1, e2, e3);

    int unvisited_count = 0;
    for (int c = 0; c < component_count; c++) {
        if (!visited[c]) unvisited_count++;
    }
    int unconnected = (unvisited_count > 1) && (unvisited_count < component_count - 1);
    if (unconnected) {
        printf(" unvisited is %d\n", unvisited_count);
        printf("Answer is %d\n", unvisited_count * (component_count - unvisited_count));
    }
    return unconnected;
}

int find_component(char *name) {
    int comp_index = -1;

    for (int c = 0; c < component_count; c++) {
        if (strcmp(components[c].name, name) == 0) {
            comp_index = c;
            break;
        }
    }

    return comp_index;
}

void build_components(void) {
    for (int e = 0; e < edge_count; e++) {
        int index = find_component(edges[e].comp1);
        if (index == -1) {
            assert((component_count < MAX_COMPONENTS) && "Component list overflow");
            index = component_count++;
            components[index].name = edges[e].comp1;
            components[index].connections = 0;
        }
        edges[e].index1 = index;
        components[index].connections++;
        index = find_component(edges[e].comp2);
        if (index == -1) {
            assert((component_count < MAX_COMPONENTS) && "Component list overflow");
            index = component_count++;
            components[index].name = edges[e].comp2;
            components[index].connections = 0;
        }
        edges[e].index2 = index;
        components[index].connections++;
    }
    int max_connections = 0;
    for (int c = 0; c < component_count; c++) {
        max_connections = MAX(max_connections, components[c].connections);
    }
    printf(" max connections is %d\n", max_connections);
}

struct path_el {
    int comp;
    struct path_el *next;
};
struct path_el *new_node(int comp, struct path_el *next) {
    struct path_el *n = malloc(sizeof(struct path_el));
    n->comp = comp;
    n->next = next;
    return n;
}
void del_node(struct path_el *node) {
    free(node);
}
int deleted[] = { -1, -1, -1 };
struct path_el *get_neighbors(int comp) {
    struct path_el *ns = NULL;
    for (int e = 0; e < edge_count; e++) {
        if ((e == deleted[0]) || (e == deleted[1]) || (e == deleted[2])) continue;
        if (edges[e].index1 == comp) {
            ns = new_node(edges[e].index2, ns);
        } else if (edges[e].index2 == comp) {
            ns = new_node(edges[e].index1, ns);;
        }
    }
    return ns;
}
int prev[MAX_COMPONENTS];
int seen[MAX_COMPONENTS];
struct path_el *get_path(int start, int end) {
    struct path_el *path = NULL;
    for (int c = 0; c < component_count; c++) {
        prev[c] = -1;
        seen[c] = 0; // false
    }

    struct path_el *nodes = new_node(start, NULL);
    while (nodes != NULL) {
        struct path_el *new_nodes = NULL;
        while (nodes != NULL) {
            struct path_el *neighbors = get_neighbors(nodes->comp);
            while (neighbors != NULL) {
                struct path_el *el = neighbors;
                if (! seen[el->comp]) {
                    seen[el->comp] = 1; // true
                    prev[el->comp] = nodes->comp;
                    new_nodes = new_node(el->comp, new_nodes);
                }

                neighbors = neighbors->next;
                del_node(el);
            }
            struct path_el *temp = nodes;
            nodes = nodes->next;
            del_node(temp);
        }

        nodes = new_nodes;
    }

    if (prev[end] != -1) {
        int cur = end;
        path = new_node(end, path);
        while (cur != start) {
            cur = prev[cur];
            path = new_node(cur, path);
        }
    }

    return path;
}


int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    printf("There are %d edges.\n", edge_count);
    build_components();
    printf("There are %d components.\n", component_count);

    // TODO: implement algorithm
#if 0
    int found = 0; // false
    for (int e1 = 0; !found && e1 < edge_count; e1++) {
        for (int e2 = e1 + 1; !found && e2 < edge_count; e2++) {
            printf("edge1 = %4d, edge2 = %d     \r", e1, e2); fflush(stdout);
            for (int e3 = e2 + 1; !found && e3 < edge_count; e3++) {
                if (! are_distinct(e1, e2, e3))
                    continue;

                if (is_graph_connected(e1, e2, e3)) {
                    found = 1; // true
                    printf("Found solution with edges:\n");
                    print_edge(e1);
                    print_edge(e2);
                    print_edge(e3);
                }
            }
        }
    }
    if (!found) printf("No solution found!\n");
#else
    int *edge_counts = malloc(edge_count * sizeof(int));
    for (int e = 0; e < edge_count; e++) edge_counts[e] = 0;
    for (int i = 0; i < 500; i++) {
        int x1 = rand() % component_count;
        int x2 = rand() % component_count;
        if (x1 != x2) {
            struct path_el *path = get_path(x1, x2);
            struct path_el *p = path;
            assert(p != NULL);
            while (p->next != NULL) {
                //int edge = find_edge_index(x1, x2);
                for (int e = 0; e < edge_count; e++) {
                    if (((p->comp == edges[e].index1) && (p->next->comp == edges[e].index2)) ||
                        ((p->comp == edges[e].index2) && (p->next->comp == edges[e].index1))) {
                        edge_counts[e]++;
                        break;
                    }
                }
                p = p->next;
            }
        }
    }
    int top1 = 0, top2 = 0, top3 = 0;
    for (int e = 0; e < edge_count; e++) {
        if (edge_counts[e] > top1) {
            top3 = top2; top2 = top1; top1 = edge_counts[e];
        } else if (edge_counts[e] > top2) {
            top3 = top2; top2 = edge_counts[e];
        } else if (edge_counts[e] > top3) {
            top3 = edge_counts[e];
        }
    }
    int delete_index = 0;
    for (int e = 0; e < edge_count; e++) {
        if (edge_counts[e] >= top3) {
            deleted[delete_index++] = e;
            printf("%d: %s -> %s (%d)\n", e, edges[e].comp1, edges[e].comp2, edge_counts[e]);
        }
    }
    assert(delete_index == 3);
    struct path_el *none = get_path(edges[deleted[0]].index1, edges[deleted[0]].index2);
    assert(none == NULL);
    int seen_count = 0;
    for (int c = 0; c < component_count; c++) {
        if (seen[c] == 1) seen_count++;
    }
    printf("seen_count is %d\n", seen_count);
    printf("Answer is %d\n", seen_count * (component_count - seen_count));
#endif


    return EXIT_SUCCESS;
}

