#include "emulator/vmcore.h"
#include "emulator/register.h"
#include "shared/graph.h"

// allocate arrays within the virtual machine
void vmcoreInit(VMCore* core) {
    ARRAY_ALLOC(Bus, *core, bus);
    ARRAY_ALLOC(Register, *core, register);
    ARRAY_ALLOC(Command, *core, command);
    ARRAY_ALLOC(void*, *core, context);
    ARRAY_ALLOC(Dependancy, *core, depends);
    ARRAY_ALLOC(Dependancy, *core, changes);

    core->switchFile = fopen("../switch.c", "w");
    
}

void vmcoreFree(VMCore* core) {
    fputs("}", core->switchFile);
    fclose(core->switchFile);
}

// push a new bus into the emulator
unsigned int createBus(VMCore* core, const char* name) {
    Bus bus = {0};
    bus.isValid = false;
    PUSH_ARRAY(Bus, *core, bus, bus);

    fputs("Bus ", core->switchFile);
    fputs(name, core->switchFile);
    fputs("Bus;\n", core->switchFile);

    return core->busCount - 1;
}

// push a new register into the emulator
unsigned int createRegister(VMCore* core, const char* name) {
    Register reg;
    regInit(&reg);
    PUSH_ARRAY(Register, *core, register, reg);

    fputs("Register ", core->switchFile);
    fputs(name, core->switchFile);
    fputs("Register;\n", core->switchFile);

    return core->registerCount - 1;
}

// call a microcode command
void coreCall(VMCore* core, unsigned int method) {
    core->commands[method](core, core->contexts[method]);
}

// main loop func
void coreCallLine(VMCore* core, unsigned int count, ...) {
    
    // no busses will be asserted at start of clock cycle
    for(unsigned int i = 0; i < core->busCount; i++) {
        core->buss[i].isValid = false;
    }
    
    // increment phase
    core->phase++;

    va_list args;
    va_start(args, count);

    Graph graph;
    InitGraph(&graph);

    // convert va_list to normal list
    unsigned int* commands = ArenaAlloc(sizeof(unsigned int) * count);

    for(unsigned int i = 0; i < count; i++) {
        commands[i] = va_arg(args, unsigned int);
    }

    // I copied this from the c# implementation, I can no longer remember how
    // it works, but it seems to, although I think it is O(N^5) which is awful
    // TODO: pre calculate based on microcode file, look up result in table
    // during run time.
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

    // overall the stuff in here might use minimum 5 memory allocations
    // if you are lucky, per clock cycle, which really needs improving.
    // All of the allocations should be able to be removed.  They also
    // mean that each minute the vm runs, it uses 0.5gb of ram which 
    // obviously is really not great.

    va_end(args);
}