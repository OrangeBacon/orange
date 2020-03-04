#ifndef ANALYSIS_TYPES_H
#define ANALYSIS_TYPES_H

#include "shared/memory.h"
#include "shared/table.h"
#include "microcode/ast.h"

typedef struct IdentifierEnum {
    Token* definition;

    // null terminated strings for each name in the enum
    ARRAY_DEFINE(Token*, member);

    // maximum length of each member name
    unsigned int identifierLength;

    // those names, but in a hash map
    Table membersTable;

    // number of bits this enum takes
    unsigned int bitWidth;
} IdentifierEnum;

// for error formatting
extern char* UserTypeNames[2];

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
    ASTTypeStatementType type;
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
extern char *IdentifierTypeNames[4];

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

typedef struct AnalysisState {
    // table mapping names to types
    Table identifiers;

    Table erroredParameters;
    bool erroredParametersInitialized;

    // has a header statement been analysed yet?
    bool parsedHeader;

// the header statement AST, used for emitting duplicate header errors
    ASTStatement* firstHeader;
} AnalysisState;

void AnalysisStateInit(AnalysisState* state);

#endif