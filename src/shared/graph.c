#include "shared/graph.h"
#include "shared/platform.h"
#include <stdlib.h>

void InitGraph(Graph* graph, NodeDataPrintFn print) {
    ARRAY_ALLOC(Node, *graph, node);
    ARRAY_ALLOC(Edge, *graph, edge);
    graph->nodeDataPrint = print;
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

Node* AddNode(Graph* graph, unsigned int id, const char* name, void* data) {
    Node newNode = {
        .value = id,
        .removed = false,
        .name = name,
        .data = data
    };

    // add new node
    if(!isInArray(graph->nodes, graph->nodeCount, id)) {
        PUSH_ARRAY(void*, *graph, node, newNode);
        return &graph->nodes[graph->nodeCount - 1];
    }

    // search for already existing node and return it
    for(unsigned int i = 0; i < graph->nodeCount; i++) {
        if(graph->nodes[i].value == id) {
            return &graph->nodes[i];
        }
    }
    return NULL;
}

void AddEdge(Graph* graph, Node* start, Node* end) {
    for(unsigned int i = 0; i < graph->edgeCount; i++) {
        if(graph->edges[i].start == start &&
           graph->edges[i].end == end) {
            return;
        }
    }

    Edge e = {
        .start = start,
        .end = end,
    };
    PUSH_ARRAY(Edge, *graph, edge, e);
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

void printGraph(Graph* graph) {
    cOutPrintf(TextWhite, "digraph g {\n");
    for(unsigned int i = 0; i < graph->nodeCount; i++) {
        Node* node = &graph->nodes[i];
        cOutPrintf(TextWhite, "\t\"%s (", node->name);
        graph->nodeDataPrint(node->data);
        cOutPrintf(TextWhite, ")\";\n");
    }

    for(unsigned int i = 0; i < graph->edgeCount; i++) {
        Edge* edge = &graph->edges[i];
        cOutPrintf(TextWhite, "\t\"%s (", edge->start->name);
        graph->nodeDataPrint(edge->start->data);
        cOutPrintf(TextWhite, ")\" -> \"%s (", edge->end->name);
        graph->nodeDataPrint(edge->end->data);
        cOutPrintf(TextWhite, ")\";\n");
    }
    cOutPrintf(TextWhite, "}\n");
}
