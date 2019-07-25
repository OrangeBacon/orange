#include "shared/graph.h"

static bool isInArray(Node* arr, unsigned int count, Node value) {
    for(unsigned int i = 0; i < count; i++) {
        if(arr[i].value == value.value) {
            return true;
        }
    }
    return false;
}

void AddNode(Graph* graph, void* node) {
    Node newNode = {.value = node, .removed = false};
    if(!isInArray(graph->nodes, graph->nodeCount, newNode)) {
        PUSH_ARRAY(void*, *graph, node, newNode);
    }
}

void AddEdge(Graph* graph, void* start, void* end) {
    Node newStart = {.value = start};
    Node newEnd = {.value = end};
    PUSH_ARRAY(Edge, *graph, edge, ((Edge){.start = newStart, .end = newEnd}));
    AddNode(graph, start);
    AddNode(graph, end);
}

NodeArray NodesNoInput(Graph* graph) {
    NodeArray ret;
    ARRAY_ALLOC(void*, ret, node);
    for(unsigned int i = 0; i < graph->nodeCount; i++) {
        bool incoming = false;
        Node* node = &graph->nodes[i];
        if(node->removed) {
            continue;
        }
        for(unsigned int j = 0; j < graph->edgeCount; j++) {
            Edge* e = &graph->edges[j];
            if(e->end.value == node->value) {
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
    ARRAY_ALLOC(Node, ret, node);

    NodeArray s = NodesNoInput(graph);
    while(s.nodeCount > 0) {
        Node* node = s.nodes[0];
        PUSH_ARRAY(Node, ret, node, node);
        node->removed = true;
        s = NodesNoInput(graph);
    }

    for(unsigned int i = 0; i < graph->nodeCount; i++) {
        graph->nodes[i].removed = false;
    }

    if(graph->nodeCount > 0) {
        NodeArray arr = {0};
        ARRAY_ALLOC(Node, ret, node);
        return arr;
    }

    return ret;
}