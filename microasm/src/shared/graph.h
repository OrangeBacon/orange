#include "shared/memory.h"

typedef struct Node {
    void* value;
    bool removed;
} Node;

typedef struct Edge {
    Node start;
    Node end;
} Edge;

typedef struct Graph {
    DEFINE_ARRAY(Node, node);
    DEFINE_ARRAY(Edge, edge);
} Graph;

typedef struct NodeArray {
    DEFINE_ARRAY(Node*, node);
} NodeArray;

void AddNode(Graph* graph, void* node);
void AddEdge(Graph* graph, void* start, void* end);
NodeArray NodesNoInput(Graph* graph);
NodeArray TopologicalSort(Graph* graph);