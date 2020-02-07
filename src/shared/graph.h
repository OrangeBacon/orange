#ifndef GRAPH_H
#define GRAPH_H

#include "shared/memory.h"

typedef struct Node {
    // unique id, can be any number user provided
    // ids do not have to be sequential
    unsigned int value;

    // name of the node, only used in printing the graph
    // does not affect node comparisons.
    const char* name;

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
// if it finds a node with the same id, it will return that one, rather than
// creating a new node.  In that case, the returned node's name will be
// the one it already had, not what was passed to this function.
Node* AddNode(Graph* graph, unsigned int node, const char* name);

// link two nodes
// creates the nodes if they do not exist.
void AddEdge(Graph* graph, unsigned int startVal, const char* startName, unsigned int endVal, const char* endName);

// return the array of active nodes with no edges directed inwards
NodeArray NodesNoInput(Graph* graph);

// sort the nodes
NodeArray TopologicalSort(Graph* graph);

void printGraph(Graph* graph);

#endif
