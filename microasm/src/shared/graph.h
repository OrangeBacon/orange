#include "shared/memory.h"

typedef struct Node {
    unsigned int value;
    bool removed;
} Node;

typedef struct Edge {
    Node* start;
    Node* end;
} Edge;

typedef struct Graph {
    DEFINE_ARRAY(Node, node);
    DEFINE_ARRAY(Edge, edge);
} Graph;

typedef struct NodeArray {
    DEFINE_ARRAY(Node*, node);
} NodeArray;

void InitGraph(Graph* graph);
Node* AddNode(Graph* graph, unsigned int node);
void AddEdge(Graph* graph, unsigned int start, unsigned int end);
NodeArray NodesNoInput(Graph* graph);
NodeArray TopologicalSort(Graph* graph);