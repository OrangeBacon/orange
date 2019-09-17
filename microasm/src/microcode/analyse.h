#ifndef ANALYSE_H
#define ANALYSE_H

struct Parser;
struct VMCoreGen;
struct AnalysisAst;

AnalysisAst* Analyse(struct Parser* parser, struct VMCoreGen* core);

#endif