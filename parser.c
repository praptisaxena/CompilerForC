#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"

int currentTokenIndex = 0;

// Get the current token
Token* getCurrentToken() {
    if (currentTokenIndex < tokenCount)
        return &tokenTable[currentTokenIndex];
    return NULL;
}

void syntaxError(const char *message, Token *tok) {
    if (tok) {
        fprintf(stderr, "Syntax error: %s but found '%s' at line %d\n",
                message, tok->lexeme, tok->line);
    } else {
        fprintf(stderr, "Syntax error: %s at end of input\n", message);
    }
    exit(1);
}

// Advance to next token and return it
Token* getNextToken() {
    if (currentTokenIndex < tokenCount)
        return &tokenTable[currentTokenIndex++];
    return NULL;
}

// Match token lexeme, consume token if matched, else error
void match(const char *expectedLexeme) {
    Token *tok = getCurrentToken();
    if (!tok || strcmp(tok->lexeme, expectedLexeme) != 0) {
        syntaxError(expectedLexeme, tok);
    }
    currentTokenIndex++;
}

// Create AST node helper
ASTNode* createNode(ASTNodeType type) {
    ASTNode *node = (ASTNode*) malloc(sizeof(ASTNode));
    node->type = type;
    node->name[0] = '\0';
    node->body = NULL;
    node->elseBody = NULL;
    node->condition = NULL;
    node->next = NULL;
    return node;
}

// Forward declarations
ASTNode* parseBlock();
ASTNode* parseStatement();
ASTNode* parseExpression();

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

// Parse block: { statement* }
ASTNode* parseBlock() {
    match("{");
    ASTNode *blockNode = createNode(AST_BLOCK);
    ASTNode *lastStmt = NULL;

    while (1) {
        Token *tok = getCurrentToken();
        if (!tok) syntaxError("unexpected EOF in block", NULL);
        if (strcmp(tok->lexeme, "}") == 0) {
            match("}");
            break;
        }

        ASTNode *stmt = parseStatement();
        if (!stmt) continue;

        if (!blockNode->body)
            blockNode->body = stmt;
        else
            lastStmt->next = stmt;

        lastStmt = stmt;
    }

    return blockNode;
}

// Parse a statement
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

        Token *next = getCurrentToken();
        if (next && strcmp(next->lexeme, "=") == 0) {
            currentTokenIndex++; 
            decl->body = parseExpression();
        }

        match(";");
        return decl;
    }

    // If statement
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

    // While loop
    if (strcmp(tok->lexeme, "while") == 0) {
        currentTokenIndex++;
        ASTNode *whileNode = createNode(AST_WHILE);
        match("(");
        whileNode->condition = parseExpression();
        match(")");
        whileNode->body = parseBlock();
        return whileNode;
    }

    // Return statement
    if (strcmp(tok->lexeme, "return") == 0) {
        currentTokenIndex++;
        ASTNode *retNode = createNode(AST_EXPRESSION);
        strcpy(retNode->name, "return");
        retNode->body = parseExpression();
        match(";");
        return retNode;
    }

    // Block
    if (strcmp(tok->lexeme, "{") == 0) {
        return parseBlock();
    }

    // Assignment or expression
    if (tok->type == TOKEN_IDENTIFIER) {
        ASTNode *assignNode = createNode(AST_EXPRESSION);
        strcpy(assignNode->name, "=");

        ASTNode *lhs = createNode(AST_EXPRESSION);
        strcpy(lhs->name, tok->lexeme);
        currentTokenIndex++;  // consume identifier

        match("=");
        ASTNode *rhs = parseExpression();
        match(";");

        assignNode->condition = lhs;
        assignNode->body = rhs;
        return assignNode;
    }

    syntaxError("unknown statement", tok);
    return NULL;
}

// Parse expression (basic binary expressions)
ASTNode* parseExpression() {
    Token *tok = getCurrentToken();
    if (!tok) syntaxError("unexpected EOF in expression", NULL);

    ASTNode *left = createNode(AST_EXPRESSION);
    strcpy(left->name, tok->lexeme);
    currentTokenIndex++;

    Token *op = getCurrentToken();
    if (op && (
        strcmp(op->lexeme, "+") == 0 || strcmp(op->lexeme, "-") == 0 ||
        strcmp(op->lexeme, "*") == 0 || strcmp(op->lexeme, "/") == 0 ||
        strcmp(op->lexeme, ">") == 0 || strcmp(op->lexeme, "<") == 0 ||
        strcmp(op->lexeme, "==") == 0 || strcmp(op->lexeme, "!=") == 0 ||
        strcmp(op->lexeme, ">=") == 0 || strcmp(op->lexeme, "<=") == 0)) {

        ASTNode *binExpr = createNode(AST_EXPRESSION);
        strcpy(binExpr->name, op->lexeme);
        currentTokenIndex++;

        ASTNode *right = createNode(AST_EXPRESSION);
        Token *rightTok = getCurrentToken();
        if (!rightTok) syntaxError("expected right operand", NULL);

        strcpy(right->name, rightTok->lexeme);
        currentTokenIndex++;

        binExpr->condition = left;
        binExpr->body = right;
        return binExpr;
    }

    return left;
}

// Print AST
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
