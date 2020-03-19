#ifndef ANALYSE_MICROCODE_H
#define ANALYSE_MICROCODE_H

#include "shared/graph.h"
#include "microcode/ast.h"
#include "microcode/parser.h"
#include "microcode/analysisTypes.h"
#include "emulator/compiletime/create.h"

NodeArray analyseLine(VMCoreGen* core, Parser* parser, ASTExpression* line,
    SourceRange* location, AnalysisState* state);

bool mcodeBitArrayCheck(Parser* parser, ASTExpression* arr, Table2* paramNames, AnalysisState* state);

NodeArray substituteAnalyseLine(ASTExpression* bits, VMCoreGen* core,
    Parser* parser, ASTStatementOpcode* opcode, unsigned int possibility,
    unsigned int lineNumber, AnalysisState* state);

#endif