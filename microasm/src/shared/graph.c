#include "shared/graph.h"
#include "shared/platform.h"
#include <stdlib.h>

void InitGraph(Graph* graph) {
    ARRAY_ALLOC(Node, *graph, node);
    ARRAY_ALLOC(Edge, *graph, edge);
}

static bool isInArray(Node* arr, unsigned int count, Node value) {
    for(unsigned int i = 0; i < count; i++) {
        if(arr[i].value == value.value) {
            return true;
        }
    }
    return false;
}

Node* AddNode(Graph* graph, unsigned int node) {
    Node newNode = {.value = node, .removed = false};
    if(!isInArray(graph->nodes, graph->nodeCount, newNode)) {
        PUSH_ARRAY(void*, *graph, node, newNode);
        return &graph->nodes[graph->nodeCount - 1];
    }
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
    NodeArray ret;
    ARRAY_ALLOC(Node*, ret, node);
    
    for(unsigned int i = 0; i < graph->nodeCount; i++) {
        bool incoming = false;
        Node* node = &graph->nodes[i];
        if(node->removed) {
            continue;
        }
        for(unsigned int j = 0; j < graph->edgeCount; j++) {
            Edge* e = &graph->edges[j];
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

    NodeArray s = NodesNoInput(graph);
    while(s.nodeCount > 0) {
        Node* node = s.nodes[0];
        PUSH_ARRAY(Node, ret, node, node);
        node->removed = true;
        s = NodesNoInput(graph);
    }

    bool retNull = false;
    for(unsigned int i = 0; i < graph->nodeCount; i++) {
        if(!graph->nodes[i].removed) {
            retNull = true;
        } else {
            graph->nodes[i].removed = false;
        }
    }

    if(retNull) {
        cErrPrintf(TextRed, "Graph Cycle\n");
        exit(1);
    }

    return ret;
}