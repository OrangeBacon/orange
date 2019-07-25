#include "emulator/vmcore.h"
#include "emulator/register.h"
#include "shared/graph.h"

void vmcoreInit(VMCore* core) {
    ARRAY_ALLOC(Bus, *core, bus);
    ARRAY_ALLOC(Register, *core, register);
    ARRAY_ALLOC(Command, *core, command);
    ARRAY_ALLOC(void*, *core, context);
    ARRAY_ALLOC(Dependancy, *core, depends);
    ARRAY_ALLOC(Dependancy, *core, changes);
}

unsigned int createBus(VMCore* core) {
    Bus bus = {0};
    PUSH_ARRAY(Bus, *core, bus, bus);

    return core->busCount - 1;
}

unsigned int createRegister(VMCore* core) {
    Register reg;
    regInit(&reg);
    PUSH_ARRAY(Register, *core, register, reg);
    return core->registerCount - 1;
}

void coreCall(VMCore* core, unsigned int method) {
    core->commands[method](core, core->contexts[method]);
}

void coreCallLine(VMCore* core, unsigned int count, ...) {
    va_list args;
    va_start(args, count);

    Graph graph;
    InitGraph(&graph);

    unsigned int* commands = ArenaAlloc(sizeof(unsigned int) * count);

    for(unsigned int i = 0; i < count; i++) {
        commands[i] = va_arg(args, unsigned int);
    }

    for(unsigned int i = 0; i < count; i++) {
        unsigned int command = commands[i];
        AddNode(&graph, command);
        for(unsigned int j = 0; j < core->changess[command].depCount; j++) {
            void* changed = core->changess[command].deps[j];
            for(unsigned int k = 0; k < count; k++) {
                unsigned int comm = commands[k];
                for(unsigned int l = 0; l < core->dependss[comm].depCount; l++) {
                    void* depended = core->dependss[comm].deps[l];
                    if(changed == depended) {
                        AddEdge(&graph, command, comm);
                    }
                }
            }
        }
    }

    NodeArray nodes = TopologicalSort(&graph);
    
    for(unsigned int i = 0; i < nodes.nodeCount; i++) {
        coreCall(core, nodes.nodes[i]->value);
    }

    va_end(args);
}