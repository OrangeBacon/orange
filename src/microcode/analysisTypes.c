#include "analysisTypes.h"

char* UserTypeNames[] = {
    [AST_TYPE_STATEMENT_ANY] = "any",
    [AST_TYPE_STATEMENT_ENUM] = "enum"
};

char *IdentifierTypeNames[] = {
    [TYPE_PARAMETER] = "parameter",
    [TYPE_VM_CONTROL_BIT] = "vm control bit",
    [TYPE_USER_TYPE] = "user type",
    [TYPE_BITGROUP] = "bitgroup"
};

void AnalysisStateInit(AnalysisState* state) {
    state->erroredParametersInitialized = false;
    state->parsedHeader = false;
    state->firstHeader = NULL;
    initTable(&state->identifiers, strHash, strCmp);
}