
#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"

#define MAX_CODE 1000
#define MAX_LEN 100

typedef struct {
    char result[MAX_LEN];
    char arg1[MAX_LEN];
    char op[MAX_LEN];
    char arg2[MAX_LEN];
} Quadruple;

extern Quadruple code[MAX_CODE];
extern int codeIndex;
extern int tempCount;
extern int labelCount;

void emit(const char* result, const char* arg1, const char* op, const char* arg2);
char* newTemp();
char* newLabel();
char* generateCode(ASTNode* node);
void optimize();
void generateFinalCode();
void printIntermediateCode(const char* phase);
int is_number(char* str);
int eval_const(int a, int b, char* op);

#endif
