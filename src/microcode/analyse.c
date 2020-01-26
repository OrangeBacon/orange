#include "microcode/analyse.h"

#include <math.h>
#include <string.h>
#include "shared/table.h"
#include "shared/memory.h"
#include "shared/graph.h"
#include "shared/log.h"
#include "emulator/compiletime/create.h"
#include "microcode/token.h"
#include "microcode/ast.h"
#include "microcode/parser.h"
#include "microcode/error.h"

// User defined types

typedef struct IdentifierEnum {
    Token* definition;

    // null terminated strings for each name in the enum
    DEFINE_ARRAY(Token*, member);

    // maximum length of each member name
    unsigned int identifierLength;

    // those names, but in a hash map
    Table membersTable;

    // number of bits this enum takes
    unsigned int bitWidth;
} IdentifierEnum;

// for error formatting
char* UserTypeNames[] = {
    [USER_TYPE_ANY] = "any",
    [USER_TYPE_ENUM] = "enum"
};


// Base types

// parameter, eg "phase: 4"
typedef struct IdentifierParameter {
    // name of the parameter
    Token* definition;

    unsigned int value;
} IdentifierParameter;

// bit defined by structure of VM, not found in code
typedef struct IdentifierControlBit {
    unsigned int value;
} IdentifierControlBit;

// user defined type
typedef struct IdentifierUserType {
    UserType type;
    union {
        IdentifierEnum enumType;
    } as;
} IdentifierUserType;

// group of control bits
typedef struct IdentifierBitGroup {
    Token* definition;

    // maximum length of all identifiers in group + null byte
    unsigned int lineLength;

    // list of null terminated strings of length lineLength
    char* substitutedIdentifiers;
} IdentifierBitGroup;

// all possible types
typedef enum IdentifierType {
    TYPE_PARAMETER,
    TYPE_VM_CONTROL_BIT,
    TYPE_USER_TYPE,
    TYPE_BITGROUP
} IdentifierType;

// for error formatting
char *IdentifierTypeNames[] = {
    [TYPE_PARAMETER] = "parameter",
    [TYPE_VM_CONTROL_BIT] = "vm control bit",
    [TYPE_USER_TYPE] = "user type",
    [TYPE_BITGROUP] = "bitgroup"
};

// so a name can be mapped to a type
typedef struct Identifier {
    IdentifierType type;
    union {
        IdentifierParameter parameter;
        IdentifierControlBit control;
        IdentifierUserType userType;
        IdentifierBitGroup bitgroup;
    } as;
} Identifier;


// why does c not define this in math.h?
static int max(int a, int b) {
    return a > b ? a : b;
}

// table mapping names to types, probably shouldnt be a global - but is.  Sorry.
Table identifiers;

Table erroredParameters;
bool erroredParametersInitialized = false;

// lookup a name as an identifier, check
//   if the name exists
//   if it represents a parameter
// if either check fails, emit error
//   unless error already emitted about that name
static Identifier* getParameter(Parser* parser, Token* errPoint, char* name,
    char* usage)
{
    if(!erroredParametersInitialized) {
        initTable(&erroredParameters, strHash, strCmp);
    }

    if(tableHas(&erroredParameters, name)) {
        return NULL;
    }

    Identifier* value;
    if(!tableGet(&identifiers, name, (void**)&value)) {
        Error* err = errNew(ERROR_SEMANTIC);
        errAddText(err, TextRed, "Parameter '%s' required to parse %s not "
            "found", name, usage);
        errAddSource(err, &errPoint->range);
        errEmit(err, parser);
        tableSet(&erroredParameters, name, (void*)1);
        return NULL;
    }

    if(value->type == TYPE_PARAMETER) {
        return value;
    }

    Error* err = errNew(ERROR_SEMANTIC);
    errAddText(err, TextRed, "To parse %s, '%s' is required as a parameter, "
        "but it is defined as a %s", usage, name,
        IdentifierTypeNames[value->type]);
    errAddSource(err, &errPoint->range);
    errEmit(err, parser);

    tableSet(&erroredParameters, name, (void*)1);
    return NULL;
}

// report symbol redefenition error at correct location
static void alreadyDefined(Parser* parser, char* name, Identifier* current,
    ASTStatement* s) {
    Token* errLoc;
    switch(s->type) {
        case AST_BLOCK_BITGROUP: errLoc = &s->as.bitGroup.name; break;
        case AST_BLOCK_HEADER: errLoc = &s->as.header.errorPoint; break;
        case AST_BLOCK_OPCODE: errLoc = &s->as.opcode.name; break;
        case AST_BLOCK_PARAMETER: errLoc = &s->as.parameter.name; break;
        case AST_BLOCK_TYPE: errLoc = &s->as.type.name; break;
    }

    Error* err = errNew(ERROR_SEMANTIC);
    errAddText(err, TextRed, "One or more prior definitions for '%s' found, "
        "currently declared as being of type %s", name,
        IdentifierTypeNames[current->type]);
    errAddSource(err, &errLoc->range);
    errEmit(err, parser);
}

// wrapper to make reporting type errors easier
static void wrongType(Parser* parser, Token* errLoc, IdentifierType expected,
    Identifier* val)
{
    Error* err = errNew(ERROR_SEMANTIC);
    errAddText(err, TextRed, "Expecting identifier '%s' to have type '%s', "
        "got type '%s'", errLoc->data.string, IdentifierTypeNames[expected],
        IdentifierTypeNames[val->type]);
    errAddSource(err, &errLoc->range);
    errEmit(err, parser);
}

// check if a type name has the required user defined type
// if not, report an error
static bool userTypeCheck(Parser* parser, UserType typeRequired,
    ASTParameter *typePair) {
    Identifier* ident;
    if(!tableGet(&identifiers, (char*)typePair->name.data.string,
        (void**)&ident))
    {
        Error* err = errNew(ERROR_SEMANTIC);
        errAddText(err, TextRed, "Identifier '%s' is not defined, %s type "
            "expected", typePair->name.data.string,
            UserTypeNames[typeRequired]);
        errAddSource(err, &typePair->name.range);
        errEmit(err, parser);
        return false;
    }

    if(ident->type != TYPE_USER_TYPE) {
        wrongType(parser, &typePair->name, TYPE_USER_TYPE, ident);
        return false;
    }

    if(typeRequired == USER_TYPE_ANY) {
        return true;
    }

    if(ident->as.userType.type != typeRequired) {
        Error* err = errNew(ERROR_SEMANTIC);
        errAddText(err, TextRed, "Expected identifier to have user type of "
            "'%s' however, it has type '%s'", UserTypeNames[typeRequired],
            UserTypeNames[ident->as.userType.type]);
        errAddSource(err, &typePair->value.range);
        errEmit(err, parser);
    }

    return true;
}

// check a parameter and add it to the identifiers map
static void analyseParameter(Parser* parser, ASTStatement* s) {
    CONTEXT(INFO, "Analysing parameter");

    char* key = (char*)s->as.parameter.name.data.string;

    Identifier* current;
    if(tableGet(&identifiers, key, (void**)&current)) {
        alreadyDefined(parser, key, current, s);
        return;
    }

    Identifier* value = ArenaAlloc(sizeof(Identifier));
    value->type = TYPE_PARAMETER;
    value->as.parameter.definition = &s->as.parameter.name;
    value->as.parameter.value = s->as.parameter.value.data.value;
    tableSet(&identifiers, key, (void*)value);
}

// analyse an array of microcode bits
// assumes that all the identifiers in the array exist and have the correct type
static NodeArray analyseLine(VMCoreGen* core, Parser* parser, BitArray* line,
    Token* opcodeName, unsigned int lineNumber) {
    CONTEXT(INFO, "Analysing line");

    Graph graph;
    InitGraph(&graph);

    // This works. Dont ask how, I cannot remember
    // creates dependancy graph for all the microcode bits, based on what they
    // change and what values they depend on.
    for(unsigned int i = 0; i < line->dataCount; i++) {
        Identifier* value;
        tableGet(&identifiers, (char*)line->datas[i].data.data.string,
            (void**)&value);
        unsigned int command = value->as.control.value;
        AddNode(&graph, command);
        for(unsigned int j = 0;
            j < core->commands[command].changesLength; j++) {
            unsigned int changed = core->commands[command].changes[j];
            for(unsigned int k = 0; k < line->dataCount; k++) {
                Identifier* value;
                tableGet(&identifiers, (char*)line->datas[k].data.data.string,
                    (void**)&value);
                unsigned int comm = value->as.control.value;
                for(unsigned int l = 0;
                    l < core->commands[comm].dependsLength; l++) {
                    unsigned int depended = core->commands[comm].depends[l];
                    if(changed == depended) {
                        AddEdge(&graph, command, comm);
                    }
                }
            }
        }
    }

    // get execution order for the microcode bits
    NodeArray nodes = TopologicalSort(&graph);
    if(!nodes.validArray) {
        Error* err = errNew(ERROR_SEMANTIC);
        errAddText(err, TextRed, "Unable to order microcode bits in line %u",
            lineNumber);
        errAddSource(err, &opcodeName->range);
        errEmit(err, parser);
    }

    // checking if any bus reads happen when the bus has not been written to

    // set all busses to not set
    for(unsigned int i = 0; i < core->componentCount; i++) {
        Component* component = &core->components[i];
        component->busStatus = false;
    }

    // loop through all commands in execution order
    for(unsigned int i = 0; i < nodes.nodeCount; i++) {
        Command* command = &core->commands[nodes.nodes[i]->value];

        // read from bus and check if possible
        for(unsigned int j = 0; j < command->readsLength; j++) {
            Component* bus = &core->components[command->reads[j]];
            if(!bus->busStatus) {
                Error* err = errNew(ERROR_SEMANTIC);
                errAddText(err, TextRed, "Command reads from bus before it was "
                    "written in line %u", lineNumber);
                errAddSource(err, &opcodeName->range);
                errEmit(err, parser);
            }
        }

        // write to bus, allow it to be read from
        // todo - do not allow multiple writes to a bus
        for(unsigned int j = 0; j < command->writesLength; j++) {
            Component* bus = &core->components[command->writes[j]];
            bus->busStatus = true;
        }
    }

    return nodes;
}

// check if all identifers in the array reperesent a control bit
static bool mcodeBitArrayCheck(Parser* parser, BitArray* arr, Table* paramNames) {
    CONTEXT(INFO, "Checking bit array");

    bool passed = true;

    for(unsigned int i = 0; i < arr->dataCount; i++) {
        Bit* bit = &arr->datas[i];

        Identifier* val;
        if(!tableGet(&identifiers, (char*)bit->data.data.string, (void**)&val)) {
            Error* err = errNew(ERROR_SEMANTIC);
            errAddText(err, TextRed, "Identifier was not defined");
            errAddSource(err, &bit->range);
            errEmit(err, parser);
            passed = false;
            continue;
        }

        if(val->type == TYPE_VM_CONTROL_BIT) {
            continue;
        } else if(val->type == TYPE_BITGROUP) {
            if(bit->paramCount != 1) {
                passed = false;
                Error* err = errNew(ERROR_SEMANTIC);
                errAddText(err, TextRed, "Only one parameter accepted by enum");
                errAddSource(err, &bit->data.range);
                errEmit(err, parser);
            }
            for(unsigned int j = 0; j < bit->paramCount; j++) {
                Token* param = &bit->params[j];
                if(!tableHas(paramNames, param)) {
                    passed = false;
                    Error* err = errNew(ERROR_SEMANTIC);
                    errAddText(err, TextRed,
                        "Could not resolve argument name \"%s\"",
                        param->data.string);
                    errAddSource(err, &param->range);
                    errEmit(err, parser);
                }
            }
        } else {
            wrongType(parser, &bit->data, TYPE_VM_CONTROL_BIT, val);
            passed = false;
        }
    }

    return passed;
}

// has a header statement been analysed yet?
static bool parsedHeader = false;

// the header statement AST, used for emitting duplicate header errors
static ASTStatement* firstHeader = NULL;

// check a header statement and put results into codegen
static void analyseHeader(Parser* parser, ASTStatement* s, VMCoreGen* core) {
    CONTEXT(INFO, "Analysing header");

    if(parsedHeader) {
        Error* err = errNew(ERROR_SEMANTIC);
        errAddText(err, TextRed, "Cannot have more than one header statement "
            "in a microcode");
        errAddSource(err, &s->as.header.errorPoint.range);
        errAddText(err, TextBlue, "Header first included here");
        errAddSource(err, &firstHeader->as.header.errorPoint.range);
        errEmit(err, parser);
        return;
    }
    parsedHeader = true;
    firstHeader = s;

    Identifier* phase = getParameter(parser, &s->as.header.errorPoint,
        "phase", "header");
    if(phase == NULL) return;
    unsigned int maxLines = (1 << phase->as.parameter.value);

    if(s->as.header.lineCount > maxLines) {
        Error* err = errNew(ERROR_SEMANTIC);
        errAddText(err, TextRed, "Number of lines in header (%u) is too high, "
            "the maximum is %u", s->as.header.lineCount, maxLines);
        errAddSource(err, &s->as.header.errorPoint.range);
        errEmit(err, parser);
    }

    for(unsigned int i = 0; i < s->as.header.lineCount; i++) {
        BitArray* line = &s->as.header.lines[i];

        Table noParams;
        if(!mcodeBitArrayCheck(parser, line, &noParams)) {
            continue;
        }

        NodeArray nodes = analyseLine(core, parser, line,
            &s->as.header.errorPoint, i);
        for(unsigned int j = 0; j < nodes.nodeCount; j++) {
            PUSH_ARRAY(unsigned int, *core, headBit, nodes.nodes[j]->value);
        }
    }
}

static NodeArray genlinePush(BitArray* bits, VMCoreGen* core, Parser* parser, ASTOpcode* opcode, unsigned int possibility, unsigned int lineNumber, unsigned int* tests) {
    BitArray subsLine;
    ARRAY_ALLOC(Bit, subsLine, data);
    for(unsigned int i = 0; i < bits->dataCount; i++) {
        Bit bit = bits->datas[i];
        Identifier* val;
        tableGet(&identifiers, (char*)bit.data.data.string, (void**)&val);
        if(val->type == TYPE_VM_CONTROL_BIT) {
            PUSH_ARRAY(Bit, subsLine, data, bit);
        } else {
            for(unsigned int j = 0; j < opcode->paramCount; j++) {
                ASTParameter* param = &opcode->params[j];
                Identifier* paramType;
                tableGet(&identifiers, (char*)param->name.data.string, (void**)&paramType);
                Bit newbit;
                newbit.data = createStrToken(&val->as.bitgroup.substitutedIdentifiers[val->as.bitgroup.lineLength*tests[possibility*opcode->paramCount+j]]);
                PUSH_ARRAY(Bit, subsLine, data, newbit);
            }
        }
    }

    return analyseLine(core, parser, &subsLine, &opcode->name, lineNumber);
}

static void analyseOpcode(Parser* parser, ASTStatement* s, VMCoreGen* core) {
    CONTEXT(INFO, "Analysing opcode statement");

    ASTOpcode *opcode = &s->as.opcode;

    if(!parsedHeader) return;
    Identifier* phase = getParameter(parser, &opcode->name,
        "phase", "opcode");
    if(phase == NULL) return;
    unsigned int maxLines = (1 << phase->as.parameter.value) -
        firstHeader->as.parameter.value.data.value;

    Identifier* opsize = getParameter(parser, &opcode->name,
        "opsize", "opcode");
    if(opsize == NULL) return;
    unsigned int maxHeaderBitLength = opsize->as.parameter.value;

    if(core->opcodes == NULL) {
        core->opcodeCount = 1 << maxHeaderBitLength;

        DEBUG("Allocating opcode array, size = %u", core->opcodeCount);
        core->opcodes = ArenaAlloc(sizeof(GenOpCode) * core->opcodeCount);

        for(unsigned int i = 0; i < core->opcodeCount; i++) {
            core->opcodes[i].isValid = false;
        }
    }

    // basic header checking
    Table paramNames;
    initTable(&paramNames, tokenHash, tokenCmp);

    unsigned int headerBitLength = 0;
    unsigned int opcodeID = opcode->id.data.value;
    unsigned int possibilities = 1;
    bool passed = true;

    headerBitLength += opcode->id.range.length - 2;
    for(unsigned int i = 0; i < opcode->paramCount; i++) {
        ASTParameter* pair = &opcode->params[i];
        passed &= userTypeCheck(parser, USER_TYPE_ANY, pair);

        bool checkLength = true;
        if(tableHas(&paramNames, &pair->value)) {
            Error* err = errNew(ERROR_SEMANTIC);
            errAddText(err, TextRed, "Parameter name \"%s\" is used multiple "
                "times", pair->value.data.string);
            errAddSource(err, &pair->value.range);
            errEmit(err, parser);
            checkLength = false;
        }
        tableSet(&paramNames, &pair->value, &pair->name);

        if(checkLength) {
            Identifier* ident;
            tableGet(&identifiers, (char*)pair->name.data.string, (void**)&ident);
            unsigned int width = ident->as.userType.as.enumType.bitWidth;
            headerBitLength += width;
            opcodeID <<= width;

            possibilities *= ident->as.userType.as.enumType.memberCount;
        }
    }

    if(!passed) {
        return;
    }

    if(headerBitLength > maxHeaderBitLength) {
        Error* err = errNew(ERROR_SEMANTIC);
        errAddText(err, TextRed, "Opcode header contains too many bits, found "
            "%u, expected %u", headerBitLength, maxHeaderBitLength);
        errAddSource(err, &opcode->id.range);
        errEmit(err, parser);
        return;
    }
    if(headerBitLength < maxHeaderBitLength) {
        Error* err = errNew(ERROR_SEMANTIC);
        errAddText(err, TextRed, "Opcode header does not contain enough bits, "
            "found %u, expected %u", headerBitLength, maxHeaderBitLength);
        errAddSource(err, &opcode->id.range);
        errEmit(err, parser);
        return;
    }

    if(opcode->lineCount > maxLines) {
        Error* err = errNew(ERROR_SEMANTIC);
        errAddText(err, TextRed, "Number of lines in opcode is too high",
            headerBitLength, maxHeaderBitLength);
        errAddSource(err, &opcode->name.range);
        errEmit(err, parser);
        return;
    }

    for(unsigned int i = 0; i < opcode->lineCount; i++) {
        Line* line = opcode->lines[i];
        GenOpCodeLine* genline = ArenaAlloc(sizeof(GenOpCodeLine));
        genline->hasCondition = line->hasCondition;

        // by default analyse low bits
        // are all of the bits valid
        if(!mcodeBitArrayCheck(parser, &line->bitsLow, &paramNames)) {
            passed = false;
            continue;
        }

        if(line->hasCondition) {
            // only check high bits if there is a condition,
            // otherwise they are identical
            if(!mcodeBitArrayCheck(parser, &line->bitsHigh, &paramNames)) {
                passed = false;
                continue;
            }
        }
    }

    if(!passed) {
        return;
    }

    // algorithm based off of fullfact from MATLAB's stats toolkit,
    // allows an int to be converted into a value to substitute
    unsigned int ncycles = possibilities;
    unsigned int* tests = ArenaAlloc(sizeof(unsigned int) * opcode->paramCount * possibilities);
    for(unsigned int i = 0; i < opcode->paramCount; i++) {
        ASTParameter* typeName;
        tableGet(&paramNames, &opcode->params[i].value, (void**)&typeName);
        Identifier* type;
        tableGet(&identifiers, (char*)typeName->name.data.string, (void**)&type);
        unsigned int level =
            type->as.userType.as.enumType.memberCount;
        unsigned int nreps = possibilities / ncycles;
        ncycles /= level;
        unsigned int count = 0;
        for(unsigned int cycle = 0; cycle < ncycles; cycle++) {
            for(unsigned int num = 0; num < level; num++) {
                for(unsigned int rep = 0; rep < nreps; rep++) {
                    tests[count*opcode->paramCount+i] = num;
                    count += 1;
                }
            }
        }
    }

    for(unsigned int i = 0; i < possibilities; i++) {
        GenOpCode* gencode = &core->opcodes[opcodeID+i];
        gencode->isValid = true;
        gencode->id = opcodeID+i;
        gencode->name = opcode->name.range.start;
        gencode->nameLen = opcode->name.range.length;
        ARRAY_ALLOC(GenOpCodeLine, *gencode, line);

        for(unsigned int j = 0; j < opcode->lineCount; j++) {
            Line* line = opcode->lines[j];
            GenOpCodeLine* genline = ArenaAlloc(sizeof(GenOpCodeLine));
            ARRAY_ALLOC(unsigned int, *genline, lowBit);
            genline->hasCondition = line->hasCondition;

            NodeArray low = genlinePush(&line->bitsLow, core, parser, opcode, i, j, tests);
            for(unsigned int k = 0; k < low.nodeCount; k++) {
                TRACE("Emitting %u at %u", low.nodes[k]->value, opcodeID+i);
                PUSH_ARRAY(unsigned int, *genline, lowBit, low.nodes[k]->value);
            }

            if(line->hasCondition) {
                ARRAY_ALLOC(unsigned int, *genline, highBit);
                NodeArray high = genlinePush(&line->bitsHigh, core, parser, opcode, i, j, tests);
                for(unsigned int k = 0; k < high.nodeCount; k++) {
                    PUSH_ARRAY(unsigned int, *genline, highBit, high.nodes[k]->value);
                }
            } else {
                genline->highBits = genline->lowBits;
                genline->highBitCount = genline->lowBitCount;
                genline->highBitCapacity = genline->lowBitCapacity;
            }

            PUSH_ARRAY(GenOpCodeLine, *gencode, line, genline);
        }
    }
}

static void analyseEnum(Parser* parser, ASTStatement* s) {
    CONTEXT(INFO, "Analysing enum statement");

    ASTType* typeStatement = &s->as.type;
    ASTTypeEnum* enumStatement = &typeStatement->as.enumType;

    Token* type = &s->as.type.name;
    Identifier* value;
    if(tableGet(&identifiers, (char*)type->data.string, (void**)&value)) {
        alreadyDefined(parser, (char*)type->data.string, value, s);
        return;
    }

    value = ArenaAlloc(sizeof(Identifier));
    value->type = TYPE_USER_TYPE;
    value->as.userType.type = USER_TYPE_ENUM;
    IdentifierEnum* enumIdent = &value->as.userType.as.enumType;
    enumIdent->definition = &s->as.type.name;
    tableSet(&identifiers, (char*)type->data.string, (void*)value);

    // check the correct number of members are present
    unsigned int size = enumStatement->width.data.value;
    enumIdent->bitWidth = size;
    unsigned int requiredMemberCount = size == 1 ? 2 : 1 << size;
    if(enumStatement->memberCount != requiredMemberCount) {
        if(enumStatement->memberCount < requiredMemberCount) {
            Error* err = errNew(ERROR_SEMANTIC);
            errAddText(err, TextRed, "Enum statement requires %u members, "
                "got %u", requiredMemberCount, enumStatement->memberCount);
            errAddSource(err, &typeStatement->name.range);
            errEmit(err, parser);
        } else {
            Error* err = errNew(ERROR_SEMANTIC);
            errAddText(err, TextRed, "Enum statement requires %u members, "
                "got %u", requiredMemberCount, enumStatement->memberCount);
            errAddSource(err,
                &enumStatement->members[requiredMemberCount].range);
            errEmit(err, parser);
        }
    }

    // as enums cannot be used directly, no issues if
    // enum values collide with other identifiers

    Table membersTable;
    initTable(&membersTable, tokenHash, tokenCmp);
    ARRAY_ALLOC(Token*, *enumIdent, member);
    enumIdent->identifierLength = 0;

    // check there are no duplicated names
    for(unsigned int i = 0; i < enumStatement->memberCount; i++) {
        Token* tok = &enumStatement->members[i];
        if(tableHas(&membersTable, tok)) {
            Token* original;
            tableGetKey(&membersTable, tok, (void**)&original);
            Error* err = errNew(ERROR_SEMANTIC);
            errAddText(err, TextRed, "Duplicated enum member");
            errAddSource(err, &tok->range);
            errAddText(err, TextBlue, "Originaly defined here");
            errAddSource(err, &original->range);
            errEmit(err, parser);
        } else {
            tableSet(&membersTable, tok, NULL);
            PUSH_ARRAY(Token*, *enumIdent, member, tok);
            enumIdent->identifierLength =
                max(enumIdent->identifierLength, tok->range.length);
        }
    }

    enumIdent->membersTable = membersTable;
}

static void analyseType(Parser* parser, ASTStatement* s) {
    CONTEXT(INFO, "Analysing type statement");

    switch(s->as.type.type) {
        case USER_TYPE_ENUM: analyseEnum(parser, s); break;
        case USER_TYPE_ANY:
            // Unreachable - should not be able to construct an any type
            // in the parser, only used for analysis
            break;
    }
}

static void analyseBitgroup(Parser* parser, ASTStatement* s) {
    CONTEXT(INFO, "Analysing type statement");

    Identifier* value;
    if(tableGet(&identifiers, (char*)s->as.bitGroup.name.data.string,
                (void**)&value)) {
        alreadyDefined(parser, (char*)s->as.bitGroup.name.data.string,
            value, s);
        return;
    }

    value = ArenaAlloc(sizeof(Identifier));
    value->type = TYPE_BITGROUP;
    value->as.bitgroup.definition = &s->as.type.name;
    tableSet(&identifiers, (char*)s->as.type.name.data.string, (void*)value);

    Table paramNames;
    initTable(&paramNames, tokenHash, tokenCmp);

    bool passed = true;

    // check that all parameters are valid enums and that the assigned names
    // are not duplicated
    for(unsigned int i = 0; i < s->as.bitGroup.paramCount; i++) {
        ASTParameter* pair = &s->as.bitGroup.params[i];
        passed &= userTypeCheck(parser, USER_TYPE_ENUM, pair);

        if(tableHas(&paramNames, &pair->value)) {
            Error* err = errNew(ERROR_SEMANTIC);
            errAddText(err, TextRed, "Parameter name '%s' collides with "
                "another parameter of the same name", pair->value.data.string);
            errAddSource(err, &pair->value.range);
            errEmit(err, parser);
        }
        tableSet(&paramNames, &pair->value, &pair->name);
    }
    if(!passed) {
        return;
    }


    // calculate the maximum length buffer required to store each possible
    // permutation of inupts
    unsigned int lineLength = 1; // starts at 1 due to null terminater
    unsigned int possibilities = 1;
    unsigned int substitutions = 0;
    for(unsigned int i = 0; i < s->as.bitGroup.segmentCount; i++) {
        ASTBitGroupIdentifier* seg = &s->as.bitGroup.segments[i];
        if(seg->type == AST_BIT_GROUP_IDENTIFIER_SUBST) {
            Token* typeName;
            if(!tableGet(&paramNames, &seg->identifier, (void**)&typeName)) {
                Error* err = errNew(ERROR_SEMANTIC);
                errAddText(err, TextRed, "Variable to substitute is not "
                    "defined");
                errAddSource(err, &seg->identifier.range);
                errEmit(err, parser);
                passed = false;
            } else {
                Identifier* type;
                tableGet(&identifiers, (char*)typeName->data.string,
                    (void**)&type);
                IdentifierEnum* enumType = &type->as.userType.as.enumType;
                lineLength += enumType->identifierLength;
                possibilities *= enumType->memberCount;
                substitutions++;
            }
        } else {
            lineLength += seg->identifier.range.length;
        }
    }
    if(!passed) {
        return;
    }

    // algorithm based off of fullfact from MATLAB's stats toolkit,
    // allows an int to be converted into a value to substitute
    unsigned int ncycles = possibilities;
    unsigned int* tests = ArenaAlloc(sizeof(unsigned int) * substitutions * possibilities);
    unsigned int substitution = 0;
    for(unsigned int i = 0; i < s->as.bitGroup.segmentCount; i++){
        ASTBitGroupIdentifier* seg = &s->as.bitGroup.segments[i];
        if(seg->type == AST_BIT_GROUP_IDENTIFIER_SUBST) {
            Token* typeName;
            tableGet(&paramNames, &seg->identifier, (void**)&typeName);
            Identifier* type;
            tableGet(&identifiers, (char*)typeName->data.string, (void**)&type);
            unsigned int level =
                type->as.userType.as.enumType.memberCount;
            unsigned int nreps = possibilities / ncycles;
            ncycles /= level;
            unsigned int count = 0;
            for(unsigned int cycle = 0; cycle < ncycles; cycle++) {
                for(unsigned int num = 0; num < level; num++) {
                    for(unsigned int rep = 0; rep < nreps; rep++) {
                        tests[count*substitutions+substitution] = num;
                        count += 1;
                    }
                }
            }
            substitution += 1;
        }
    }

    // list of null terminated strings
    char* substitutedList =
        ArenaAlloc(sizeof(char) * lineLength * possibilities);
    for(unsigned int i = 0; i < possibilities; i++) {
        char* currentIdent = &substitutedList[i*lineLength];
        currentIdent[0] = '\0';

        unsigned int count = 0;
        for(unsigned int j = 0; j < s->as.bitGroup.segmentCount; j++){
            ASTBitGroupIdentifier* seg = &s->as.bitGroup.segments[j];
            if(seg->type == AST_BIT_GROUP_IDENTIFIER_SUBST) {
                // type varified to be valid earlier in function
                Token* typeName;
                tableGet(&paramNames, &seg->identifier, (void**)&typeName);
                Identifier* type;
                tableGet(&identifiers, (char*)typeName->data.string,
                    (void**)&type);
                IdentifierEnum* enumIdent = &type->as.userType.as.enumType;

                // Played around in excel to work this out, cannot remember
                // how it works, but it does.  I don't think I knew when
                // I wrote it either.
                strcat(currentIdent, enumIdent->members[tests[i*substitutions+count]]->data.string);
                count += 1;
            } else {
                strncat(currentIdent, seg->identifier.data.string,
                    seg->identifier.range.length);
            }
        }

        // check that after the substitutions have completed, a valid
        // control bit was formed
        Identifier* val;
        if(!tableGet(&identifiers, currentIdent, (void**)&val)) {
            Error* err = errNew(ERROR_SEMANTIC);
            errAddText(err, TextRed, "Found undefined resultant identifier "
                "while substituting into bitgroup");
            errAddSource(err, &value->as.bitgroup.definition->range);
            errEmit(err, parser);
            passed = false;
        }
        if(val->type != TYPE_VM_CONTROL_BIT) {
            Error* err = errNew(ERROR_SEMANTIC);
            errAddText(err, TextRed, "Found resultant identifier to have type "
                "%s, expecting VM_CONTROL_BIT while substituting into bitgroup",
                IdentifierTypeNames[val->type]);
            errAddSource(err, &value->as.bitgroup.definition->range);
            errEmit(err, parser);
            passed = false;
        }

        if(!passed) {
            return;
        }
    }

    value->as.bitgroup.substitutedIdentifiers = substitutedList;
    value->as.bitgroup.lineLength = lineLength;
}

void Analyse(Parser* parser, VMCoreGen* core) {
    CONTEXT(INFO, "Running analysis");

    if(parser->hadError)return;

    initTable(&identifiers, strHash, strCmp);

    for(unsigned int i = 0; i < core->commandCount; i++) {
        char* key = (char*)core->commands[i].name;
        Identifier* value = ArenaAlloc(sizeof(Identifier));
        value->type = TYPE_VM_CONTROL_BIT;
        value->as.control.value = i;
        tableSet(&identifiers, key, (void*)value);
    }

    for(unsigned int i = 0; i < parser->ast->statementCount; i++) {
        ASTStatement* s = &parser->ast->statements[i];
        if(!s->isValid) continue;
        switch(s->type) {
            case AST_BLOCK_PARAMETER: analyseParameter(parser, s); break;
            case AST_BLOCK_HEADER: analyseHeader(parser, s, core); break;
            case AST_BLOCK_OPCODE: analyseOpcode(parser, s, core); break;
            case AST_BLOCK_TYPE: analyseType(parser, s); break;
            case AST_BLOCK_BITGROUP: analyseBitgroup(parser, s); break;
        }
    }

    INFO("Finished Analysis");
}
