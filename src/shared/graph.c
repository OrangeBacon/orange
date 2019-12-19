#include "shared/graph.h"
#include "shared/platform.h"
#include <stdlib.h>

void InitGraph(Graph* graph) {
    ARRAY_ALLOC(Node, *graph, node);
    ARRAY_ALLOC(Edge, *graph, edge);
}

// is the provided node already in the graph?
static bool isInArray(Node* arr, unsigned int count, unsigned int value) {
    for(unsigned int i = 0; i < count; i++) {
        if(arr[i].value == value) {
            return true;
        }
    }
    return false;
}

Node* AddNode(Graph* graph, unsigned int node) {
    Node newNode = {.value = node, .removed = false};

    // add new node
    if(!isInArray(graph->nodes, graph->nodeCount, node)) {
        PUSH_ARRAY(void*, *graph, node, newNode);
        return &graph->nodes[graph->nodeCount - 1];
    }

    // search for already existing node and return it
    for(unsigned int i = 0; i < graph->nodeCount; i++) {
        if(graph->nodes[i].value == node) {
            return &graph->nodes[i];
        }
    }
    return NULL;
}

void AddEdge(Graph* graph, unsigned int start, unsigned int end) {
    PUSH_ARRAY(Edge, *graph, edge, ((Edge){
        .start = AddNode(graph, start), 
        .end = AddNode(graph, end)
    }));
}

NodeArray NodesNoInput(Graph* graph) {

    // initialise return value
    NodeArray ret;
    ARRAY_ALLOC(Node*, ret, node);
    
    // check each node
    for(unsigned int i = 0; i < graph->nodeCount; i++) {
        Node* node = &graph->nodes[i];
        if(node->removed) {
            continue;
        }

        bool incoming = false;

        // check each edge
        for(unsigned int j = 0; j < graph->edgeCount; j++) {
            Edge* e = &graph->edges[j];

            // if both ends still should be checked and ends on current node
            if(!e->start->removed && !e->end->removed && e->end->value == node->value) {
                incoming = true;
            }
        }
        if(!incoming) {
            PUSH_ARRAY(Node*, ret, node, node);
        }
    }
    return ret;
}

NodeArray TopologicalSort(Graph* graph) {
    NodeArray ret;
    ARRAY_ALLOC(Node*, ret, node);

    // toposort
    NodeArray s = NodesNoInput(graph);
    while(s.nodeCount > 0) {
        Node* node = s.nodes[0];
        PUSH_ARRAY(Node, ret, node, node);
        node->removed = true;
        s = NodesNoInput(graph);
    }

    // re-activate all nodes and check if any nodes were not picked
    bool retNull = false;
    for(unsigned int i = 0; i < graph->nodeCount; i++) {
        if(!graph->nodes[i].removed) {
            retNull = true;
        } else {
            graph->nodes[i].removed = false;
        }
    }

    ret.validArray = !retNull;
    return ret;
}
