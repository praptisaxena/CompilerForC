#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"

int currentTokenIndex = 0;

Token* getCurrentToken() {
    if (currentTokenIndex < tokenCount)
        return &tokenTable[currentTokenIndex];
    return NULL;
}

Token* getNextToken() {
    if (currentTokenIndex < tokenCount)
        return &tokenTable[currentTokenIndex++];
    return NULL;
}

void match(const char *expected) {
    Token *tok = getCurrentToken();
    if (!tok || strcmp(tok->lexeme, expected) != 0) {
        syntaxError(expected, tok);
    }
    currentTokenIndex++;
}

void syntaxError(const char *message, Token *tok) {
    if (tok) {
        fprintf(stderr, "Syntax error: %s but found '%s' at line %d\n", message, tok->lexeme, tok->line);
    } else {
        fprintf(stderr, "Syntax error: %s at end of input\n", message);
    }
    exit(1);
}

ASTNode* createNode(ASTNodeType type) {
    ASTNode *node = (ASTNode*) malloc(sizeof(ASTNode));
    node->type = type;
    node->name[0] = '\0';
    node->body = node->condition = node->elseBody = node->next = NULL;
    return node;
}

ASTNode* parseBlock();
ASTNode* parseStatement();
ASTNode* parseExpression();
ASTNode* parseProgram();

ASTNode* parseProgram() {
    ASTNode *head = NULL, *tail = NULL;

    while (currentTokenIndex < tokenCount) {
        Token *tok = getCurrentToken();

        ASTNode *node = NULL;

        if (tok->type == TOKEN_PREPROCESSOR) {
            node = createNode(AST_PREPROCESSOR);
            strcpy(node->name, tok->lexeme);
            currentTokenIndex++;
        } else if (strcmp(tok->lexeme, "int") == 0) {
            node = parseFunction();  
        } else {
            syntaxError("expected preprocessor directive or function", tok);
        }

        if (node) {
            if (!head) head = node;
            else tail->next = node;
            tail = node;
        }
    }

    return head;
}

ASTNode* parseFunction() {
    match("int");
    Token *name = getNextToken();

    if (!name || name->type != TOKEN_IDENTIFIER) {
        syntaxError("expected function name", name);
    }

    ASTNode *funcNode = createNode(AST_FUNCTION);
    strcpy(funcNode->name, name->lexeme);
    match("(");
    match(")");
    funcNode->body = parseBlock();
    return funcNode;
}

ASTNode* parseBlock() {
    match("{");
    ASTNode *blockNode = createNode(AST_BLOCK);
    ASTNode *last = NULL;

    while (1) {
        Token *tok = getCurrentToken();
        if (!tok) syntaxError("unexpected EOF in block", NULL);
        if (strcmp(tok->lexeme, "}") == 0) {
            match("}");
            break;
        }

        ASTNode *stmt = parseStatement();
        if (!stmt) continue;

        if (!blockNode->body) blockNode->body = stmt;
        else last->next = stmt;
        last = stmt;
    }

    return blockNode;
}

ASTNode* parseStatement() {
    Token *tok = getCurrentToken();
    if (!tok) return NULL;

    if (tok->type == TOKEN_KEYWORD &&
        (strcmp(tok->lexeme, "int") == 0 || strcmp(tok->lexeme, "float") == 0)) {

        currentTokenIndex++;
        Token *id = getNextToken();
        if (!id || id->type != TOKEN_IDENTIFIER) {
            syntaxError("expected identifier", id);
        }

        ASTNode *decl = createNode(AST_EXPRESSION);
        strcpy(decl->name, "declare");

        ASTNode *var = createNode(AST_EXPRESSION);
        strcpy(var->name, id->lexeme);
        decl->condition = var;

        tok = getCurrentToken();
        if (tok && strcmp(tok->lexeme, "=") == 0) {
            currentTokenIndex++;
            decl->body = parseExpression();
        }

        match(";");
        return decl;
    }

    if (strcmp(tok->lexeme, "if") == 0) {
        currentTokenIndex++;
        ASTNode *ifNode = createNode(AST_IF);
        match("(");
        ifNode->condition = parseExpression();
        match(")");
        ifNode->body = parseBlock();

        tok = getCurrentToken();
        if (tok && strcmp(tok->lexeme, "else") == 0) {
            currentTokenIndex++;
            ifNode->elseBody = parseBlock();
        }
        return ifNode;
    }

    if (strcmp(tok->lexeme, "while") == 0) {
        currentTokenIndex++;
        ASTNode *whileNode = createNode(AST_WHILE);
        match("(");
        whileNode->condition = parseExpression();
        match(")");
        whileNode->body = parseBlock();
        return whileNode;
    }

    if (strcmp(tok->lexeme, "return") == 0) {
        currentTokenIndex++;
        ASTNode *retNode = createNode(AST_EXPRESSION);
        strcpy(retNode->name, "return");
        retNode->body = parseExpression();
        match(";");
        return retNode;
    }

    if (strcmp(tok->lexeme, "{") == 0) {
        return parseBlock();
    }

    if (tok->type == TOKEN_IDENTIFIER) {
        Token *id = tok;
        currentTokenIndex++;

        if (strcmp(getCurrentToken()->lexeme, "=") == 0) {
            currentTokenIndex++;

            ASTNode *assign = createNode(AST_EXPRESSION);
            strcpy(assign->name, "=");

            ASTNode *lhs = createNode(AST_EXPRESSION);
            strcpy(lhs->name, id->lexeme);
            assign->condition = lhs;

            assign->body = parseExpression();
            match(";");
            return assign;
        } else {
            syntaxError("expected '=' after identifier", getCurrentToken());
        }
    }

    syntaxError("unknown statement", tok);
    return NULL;
}

ASTNode* parseExpression() {
    Token *tok = getCurrentToken();
    if (!tok) syntaxError("unexpected EOF in expression", NULL);

    ASTNode *left = NULL;

    if (strcmp(tok->lexeme, "(") == 0) {
        match("(");
        left = parseExpression();
        match(")");
    } else {
        left = createNode(AST_EXPRESSION);
        strcpy(left->name, tok->lexeme);
        currentTokenIndex++;
    }

    tok = getCurrentToken();
    if (tok && (
        strcmp(tok->lexeme, "+") == 0 || strcmp(tok->lexeme, "-") == 0 ||
        strcmp(tok->lexeme, "*") == 0 || strcmp(tok->lexeme, "/") == 0 ||
        strcmp(tok->lexeme, "==") == 0 || strcmp(tok->lexeme, "!=") == 0 ||
        strcmp(tok->lexeme, ">=") == 0 || strcmp(tok->lexeme, "<=") == 0 ||
        strcmp(tok->lexeme, "<") == 0 || strcmp(tok->lexeme, ">") == 0)) {

        ASTNode *opNode = createNode(AST_EXPRESSION);
        strcpy(opNode->name, tok->lexeme);
        currentTokenIndex++;

        ASTNode *right = parseExpression();

        opNode->condition = left;
        opNode->body = right;
        return opNode;
    }

    return left;
}

void printAST(ASTNode *node, int indent) {
    if (!node) return;

    for (int i = 0; i < indent; i++) printf("  ");

    switch (node->type) {
        case AST_FUNCTION:
            printf("Function: %s\n", node->name);
            break;
        case AST_BLOCK:
            printf("Block\n");
            break;
        case AST_IF:
            printf("If\n");
            break;
        case AST_WHILE:
            printf("While\n");
            break;
        case AST_EXPRESSION:
            printf("Expr: %s\n", node->name);
            break;
        case AST_PREPROCESSOR:
            printf("Preprocessor: %s\n", node->name);
            break;
        default:
            printf("Unknown\n");
    }

    if (node->condition) printAST(node->condition, indent + 1);
    if (node->body) printAST(node->body, indent + 1);
    if (node->elseBody) {
        for (int i = 0; i < indent; i++) printf("  ");
        printf("Else\n");
        printAST(node->elseBody, indent + 1);
    }

    printAST(node->next, indent);
}
