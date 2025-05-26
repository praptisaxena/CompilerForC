#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "codegen.h"

int tempCount = 0;
int labelCount = 0;
int codeIndex = 0;
Quadruple code[MAX_CODE];

char* newTemp() {
    char* temp = (char*)malloc(10);
    sprintf(temp, "t%d", tempCount++);
    return temp;
}

char* newLabel() {
    char* label = (char*)malloc(10);
    sprintf(label, "L%d", labelCount++);
    return label;
}

void emit(const char* result, const char* arg1, const char* op, const char* arg2) {
    if (codeIndex >= MAX_CODE) {
        fprintf(stderr, "Error: Code array overflow\n");
        exit(1);
    }
    
    strncpy(code[codeIndex].result, result, MAX_LEN-1);
    strncpy(code[codeIndex].arg1, arg1, MAX_LEN-1);
    if (op) {
        strncpy(code[codeIndex].op, op, MAX_LEN-1);
    } else {
        code[codeIndex].op[0] = '\0';
    }
    if (arg2) {
        strncpy(code[codeIndex].arg2, arg2, MAX_LEN-1);
    } else {
        code[codeIndex].arg2[0] = '\0';
    }
    codeIndex++;
}

int is_number(char* str) {
    if (!str) return 0;
    for (int i = 0; str[i]; i++) if (!isdigit(str[i])) return 0;
    return 1;
}

int eval_const(int a, int b, char* op) {
    if (strcmp(op, "+") == 0) return a + b;
    if (strcmp(op, "-") == 0) return a - b;
    if (strcmp(op, "*") == 0) return a * b;
    if (strcmp(op, "/") == 0 && b != 0) return a / b;
    if (strcmp(op, "==") == 0) return a == b;
    if (strcmp(op, "!=") == 0) return a != b;
    if (strcmp(op, "<") == 0) return a < b;
    if (strcmp(op, ">") == 0) return a > b;
    if (strcmp(op, "<=") == 0) return a <= b;
    if (strcmp(op, ">=") == 0) return a >= b;
    return 0;
}

void printIntermediateCode(const char* phase) {
    printf("\n=== %s Intermediate Code ===\n", phase);
    printf("%-5s %-10s %-10s %-5s %-10s\n", "Line", "Result", "Arg1", "Op", "Arg2");
    printf("----------------------------------------\n");
    for (int i = 0; i < codeIndex; i++) {
        printf("%-5d %-10s %-10s %-5s %-10s\n", 
               i, 
               code[i].result, 
               code[i].arg1, 
               code[i].op, 
               code[i].arg2);
    }
    printf("========================================\n");
}

void optimize() {
    printf("\nPerforming optimization...\n");
    for (int i = 0; i < codeIndex; i++) {
        if (is_number(code[i].arg1)) {
            if (strlen(code[i].op)) { // Binary operation
                if (is_number(code[i].arg2)) {
                    int val = eval_const(atoi(code[i].arg1), atoi(code[i].arg2), code[i].op);
                    sprintf(code[i].arg1, "%d", val);
                    strcpy(code[i].op, "=");
                    code[i].arg2[0] = '\0';
                    printf("Optimized line %d: Constant folding applied\n", i);
                }
            }
        }
    }
}

void generateFinalCode() {
    printf("\n=== Final Assembly Code ===\n");
    printf("PUSH BP\n");
    printf("MOV BP, SP\n");
    
    for (int i = 0; i < codeIndex; i++) {
        if (strcmp(code[i].op, "if") == 0) {
            printf("LOAD %s\n", code[i].arg1);
            printf("JNZ %s\n", code[i].arg2);
        } 
        else if (strcmp(code[i].op, "goto") == 0) {
            printf("JMP %s\n", code[i].arg2);
        }
        else if (strcmp(code[i].op, "=") == 0 && strlen(code[i].arg2) == 0) {
            if (is_number(code[i].arg1)) {
                printf("MOV %s, %s\n", code[i].result, code[i].arg1);
            } else {
                printf("LOAD %s\n", code[i].arg1);
                printf("STORE %s\n", code[i].result);
            }
        } 
        else if (strcmp(code[i].result, "RET") == 0) {
            printf("LOAD %s\n", code[i].arg1);
            printf("RET\n");
        }
        else {
            printf("LOAD %s\n", code[i].arg1);
            if (strlen(code[i].op) > 0) {
                printf("%s %s\n", 
                    strcmp(code[i].op, "+") == 0 ? "ADD" :
                    strcmp(code[i].op, "-") == 0 ? "SUB" :
                    strcmp(code[i].op, "*") == 0 ? "MUL" : "DIV",
                    code[i].arg2);
            }
            printf("STORE %s\n", code[i].result);
        }
    }
    
    printf("MOV SP, BP\n");
    printf("POP BP\n");
    printf("================================\n");
}

char* generateCode(ASTNode* node) {
    if (!node) return NULL;

    char* temp1, *temp2, *temp3, *label1, *label2;

    switch (node->type) {
        case AST_FUNCTION:
            emit(node->name, "", "FUNC", "");
            generateCode(node->body);
            break;

        case AST_BLOCK:
            generateCode(node->body);
            break;

        case AST_IF:
            label1 = newLabel();
            label2 = newLabel();
            temp1 = generateCode(node->condition);
            emit("", temp1, "if", label1);
            generateCode(node->body);
            emit("", "", "goto", label2);
            emit(label1, "", "LABEL", "");
            if (node->elseBody) {
                generateCode(node->elseBody);
            }
            emit(label2, "", "LABEL", "");
            break;

        case AST_WHILE:
            label1 = newLabel();
            label2 = newLabel();
            emit(label1, "", "LABEL", "");
            temp1 = generateCode(node->condition);
            emit("", temp1, "if", label2);
            generateCode(node->body);
            emit("", "", "goto", label1);
            emit(label2, "", "LABEL", "");
            break;

        case AST_EXPRESSION:
            if (strcmp(node->name, "=") == 0) {
                temp1 = generateCode(node->body);
                emit(node->condition->name, temp1, "=", "");
            } 
            else if (strcmp(node->name, "declare") == 0) {
                if (node->body) {
                    temp1 = generateCode(node->body);
                    emit(node->condition->name, temp1, "=", "");
                }
            }
            else if (strcmp(node->name, "return") == 0) {
                temp1 = generateCode(node->body);
                emit("RET", temp1, "RET", "");
            }
            else if (node->condition && node->body) {
                // Binary operation
                temp1 = generateCode(node->condition);
                temp2 = generateCode(node->body);
                temp3 = newTemp();
                emit(temp3, temp1, node->name, temp2);
                return temp3;
            }
            else {
                // Variable or constant
                return node->name;
            }
            break;

        default:
            if (node->next) generateCode(node->next);
            break;
    }

    if (node->next) generateCode(node->next);
    return NULL;
}
void printCode(const char* label) {
    printf("\n--- %s ---\n", label);
    for (int i = 0; i < codeIndex; i++) {
        if (strlen(code[i].op)) {
            if (strlen(code[i].arg2)) {
                printf("%s = %s %s %s\n", code[i].result, code[i].arg1, code[i].op, code[i].arg2);
            } else {
                printf("%s = %s %s\n", code[i].result, code[i].arg1, code[i].op);
            }
        } else {
            printf("%s = %s\n", code[i].result, code[i].arg1);
        }
    }
    printf("----------------------\n");
}

