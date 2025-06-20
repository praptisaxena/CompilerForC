#ifndef AST_H
#define AST_H
#include "lexer.h"

#define MAX_NAME_LEN 100

extern int currentTokenIndex; 

typedef enum {
    AST_FUNCTION,
    AST_BLOCK,
    AST_IF,
    AST_WHILE,
    AST_FOR,
    AST_PREPROCESSOR,
    AST_EXPRESSION,
    AST_STATEMENT,

} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;

    char name[MAX_NAME_LEN];

    struct ASTNode *body;
    struct ASTNode *elseBody;

    struct ASTNode *condition;
    struct ASTNode *next;

} ASTNode;

ASTNode* createNode(ASTNodeType type);

ASTNode* parseFunction();
ASTNode* parseProgram();
void printAST(ASTNode *node, int indent);

#endif
