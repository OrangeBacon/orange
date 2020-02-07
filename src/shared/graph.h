#ifndef GRAPH_H
#define GRAPH_H

#include "shared/memory.h"

typedef struct Node {
    // unique id, can be any number user provided
    // ids do not have to be sequential
    unsigned int value;

    // is the node still active, used in toposport
    bool removed;
} Node;

// link between two nodes
typedef struct Edge {
    Node* start;
    Node* end;
} Edge;

// directional, cyclic graph
typedef struct Graph {
    DEFINE_ARRAY(Node, node);
    DEFINE_ARRAY(Edge, edge);
} Graph;

// temporary array result
typedef struct NodeArray {
    DEFINE_ARRAY(Node*, node);

    // did the operation producing this result succeed
    bool validArray;
} NodeArray;

// create a new graph
void InitGraph(Graph* graph);

// add and return a new node
Node* AddNode(Graph* graph, unsigned int node);

// link two nodes
void AddEdge(Graph* graph, unsigned int start, unsigned int end);

// return the array of active nodes with no edges directed inwards
NodeArray NodesNoInput(Graph* graph);

// sort the nodes
NodeArray TopologicalSort(Graph* graph);

void printGraph(Graph* graph);

#endif
