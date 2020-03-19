#include "microcode/analyseMicrocode.h"

#include <string.h>
#include "microcode/error.h"

typedef enum {
    GRAPH_STATE_COMPONENT,
    GRAPH_STATE_COMMAND
} GraphState;

void printGraphState(void* g, graphPrintFn printFn) {
    if((GraphState)g == GRAPH_STATE_COMPONENT) {
        printFn(TextWhite, "Component");
    } else {
        printFn(TextWhite, "Command");
    }
}

// analyse an array of microcode bits
// assumes that all the identifiers in the array exist and have the correct type
NodeArray analyseLine(VMCoreGen* core, Parser* parser, ASTExpression* line,
    SourceRange* location, AnalysisState* state) {
    CONTEXT(INFO, "Analysing line");

    Graph graph;
    InitGraph(&graph, printGraphState);

    for(unsigned int i = 0; i < line->as.list.elementCount; i++) {
        // adds a command to the graph
        // the command gets a node in the graph
        // add edge command -> everything it changes
        // add edge everything it depends on -> command

        Identifier* bitIdent = TABLE2_GET(state->identifiers, line->as.list.elements[i]->as.variable.data.string);
        unsigned int commandID = bitIdent->as.control.value;
        Command* coreCommand = &core->commands[commandID];

        Node* commandNode = AddNode(&graph, commandID,
            coreCommand->name, (void*)GRAPH_STATE_COMMAND);

        // adds edge between dependancies and commandNode
        // id has commandCount added so the component id does not clash with
        // the command id
        for(unsigned int j = 0; j < coreCommand->dependsLength; j++) {
            unsigned int dependCompID = coreCommand->depends[j];
            Component* dependComp = &core->components[dependCompID];
            Node* dependNode = AddNode(&graph, dependCompID+core->commandCount,
                dependComp->printName, (void*)GRAPH_STATE_COMPONENT);
            AddEdge(&graph, dependNode, commandNode);
        }

        // same as above loop but adds edge from commandNode to everything
        // it changes
        for(unsigned int j = 0; j < coreCommand->changesLength; j++) {
            unsigned int changeCompID = coreCommand->changes[j];
            Component* changeComp = &core->components[changeCompID];
            Node* changeNode = AddNode(&graph, changeCompID+core->commandCount,
                changeComp->printName, (void*)GRAPH_STATE_COMPONENT);
            AddEdge(&graph, commandNode, changeNode);
        }
    }

    TRACE("Created command graph");

    // get execution order for the microcode bits
    NodeArray nodes = TopologicalSort(&graph);
    if(!nodes.validArray) {
        Error* err = errNew(ERROR_SEMANTIC);
        err->severity = ERROR_WARN;
        errAddText(err, TextYellow, "Unable to order microcode bits");
        errAddSource(err, location);
        errAddText(err, TextBlue, "Instruction graph (graphviz dot): ");
        errAddGraph(err, &graph);
        errAddText(err, TextBlue, "Substitutions: ");
        for(unsigned int i = 0; i < line->as.list.elementCount; i++) {
            if(line->as.list.elements[i]->type == AST_EXPRESSION_CALL) {
                errAddText(err, TextWhite, line->as.call.params[i]->as.variable.data.string);
            }
        }
        errEmit(err, parser);
        return nodes;
    }

    // filter graph results for only commands, not components
    NodeArray commands;
    commands.validArray = true;
    ARRAY_ALLOC(Node*, commands, node);
    for(unsigned int i = 0; i < nodes.nodeCount; i++) {
        Node* node = nodes.nodes[i];
        if((GraphState)node->data == GRAPH_STATE_COMMAND) {
            ARRAY_PUSH(commands, node, node);
        }
    }

    // checking if any bus reads happen when the bus has not been written to

    // set all busses to not set
    for(unsigned int i = 0; i < core->componentCount; i++) {
        Component* component = &core->components[i];
        component->busStatus = false;
    }

    // loop through all commands in execution order
    for(unsigned int i = 0; i < commands.nodeCount; i++) {

        Command* command = &core->commands[commands.nodes[i]->value];

        // read from bus and check if possible
        for(unsigned int j = 0; j < command->readsLength; j++) {
            Component* bus = &core->components[command->reads[j]];
            if(!bus->busStatus) {
                Error* err = errNew(ERROR_SEMANTIC);
                errAddText(err, TextRed, "Command reads from bus before it was "
                    "written");
                errAddSource(err, location);
                errAddText(err, TextBlue, "Command graph (graphviz dot):");
                errAddGraph(err, &graph);
                errEmit(err, parser);
                nodes.validArray = false;
            }
        }

        // write to bus, allow it to be read from
        // todo - do not allow multiple writes to a bus
        for(unsigned int j = 0; j < command->writesLength; j++) {
            Component* bus = &core->components[command->writes[j]];
            if(bus->busStatus) {
                Error* err = errNew(ERROR_SEMANTIC);
                errAddText(err, TextRed, "Command writes to bus twice");
                errAddSource(err, location);
                errAddText(err, TextBlue, "Command graph (graphviz dot):");
                errAddGraph(err, &graph);
                errEmit(err, parser);
            } else {
                bus->busStatus = true;
            }
        }
    }

    return commands;
}

// TODO- fix
void wrongType(Parser* parser, Token* errLoc, IdentifierType expected,
    Identifier* val);

// check if all identifers in the array reperesent a control bit
bool mcodeBitArrayCheck(Parser* parser, ASTExpression* arr, Table2* paramNames, AnalysisState* state) {
    CONTEXT(INFO, "Checking bit array");

    bool passed = true;

    for(unsigned int i = 0; i < arr->as.list.elementCount; i++) {
        ASTExpression* bit = arr->as.list.elements[i];

        // look up to check the identifier is defined
        Identifier* val = TABLE2_GET(state->identifiers, bit->as.variable.data.string);
        if(val == NULL) {
            Error* err = errNew(ERROR_SEMANTIC);
            errAddText(err, TextRed, "Identifier was not defined");
            errAddSource(err, &bit->as.variable.range);
            errEmit(err, parser);
            passed = false;
            continue;
        }

        if(val->type == TYPE_VM_CONTROL_BIT) {
            continue;
        } else if(val->type == TYPE_BITGROUP) {
            // check that there is only 1 parameter passed to a bitgroup
            if(bit->as.call.paramCount != 1) {
                passed = false;
                Error* err = errNew(ERROR_SEMANTIC);
                errAddText(err, TextRed, "Only one parameter accepted by enum");
                errAddSource(err, &bit->as.call.callee->as.variable.range);
                errEmit(err, parser);
            }
            // check all of the parameters of the bitgroup, regardless of how
            // many should have been passed
            for(unsigned int j = 0; j < bit->as.call.paramCount; j++) {
                ASTExpression* param = bit->as.call.params[j];
                if(!TABLE2_HAS(*paramNames, param)) {
                    passed = false;
                    Error* err = errNew(ERROR_SEMANTIC);
                    errAddText(err, TextRed,
                        "Could not resolve argument name \"%s\"",
                        param->as.call.callee->as.variable.data.string);
                    errAddSource(err, &param->as.variable.range);
                    errEmit(err, parser);
                }
            }
        } else {
            wrongType(parser, &bit->as.variable, TYPE_VM_CONTROL_BIT, val);
            passed = false;
        }
    }

    return passed;
}

NodeArray substituteAnalyseLine(ASTExpression* bits, VMCoreGen* core,
    Parser* parser, ASTStatementOpcode* opcode, unsigned int possibility,
    unsigned int lineNumber, AnalysisState* state)
{
    ASTExpression subsLine;
    ARRAY_ALLOC(ASTExpression*, subsLine.as.list, element);
    for(unsigned int i = 0; i < bits->as.list.elementCount; i++) {
        ASTExpression* bit = bits->as.list.elements[i];
        Identifier* val = TABLE2_GET(state->identifiers, bit->as.variable.data.string);
        if(val->type == TYPE_VM_CONTROL_BIT) {
            ARRAY_PUSH(subsLine.as.list, element, bit);
        } else {
            // assume bitarray checks have been done before, so
            // bit->paramCount is always 1
            // and val is a bitgroup

            // iterate through parameters in reverse order, eg regb, rega, ...
            for(int j = opcode->paramCount - 1; j >= 0; j--) {

                // get the type specified in the opcode's header
                Identifier* paramType = TABLE2_GET(state->identifiers,
                    opcode->params[j].name.data.string);

                // possibility is a number outof the member counts of all
                // parameters multiplied together.  Therefore, dividing
                // possibility by the member count gives the number of
                // possibilities of all remaining members.  Modulo used to
                // get the state of the current parameter (remainder from
                // division) and subtract used to ensure possibility is a
                // multiple of the member count.
                // This means currentNumber is an index into the substituted
                // identifiers from the specified bitgroup (stored in val)
                unsigned int currentNumber = possibility % paramType->as.userType.as.enumType.memberCount;
                possibility -= currentNumber;
                possibility /= paramType->as.userType.as.enumType.memberCount;

                if(strcmp(bit->as.call.params[0]->as.variable.data.string, opcode->params[j].value.data.string) == 0) {
                    ASTExpression newBit;
                    newBit.as.call.callee->as.variable = createStrToken((char*)&val->as.bitgroup.substitutedIdentifiers[currentNumber*val->as.bitgroup.lineLength]);
                    ARRAY_PUSH(subsLine.as.list, element, &newBit);
                }
            }
        }
    }

    return analyseLine(core, parser, &subsLine, &opcode->expressions[lineNumber]->as.list.elements[0]->as.variable.range, state);
}
