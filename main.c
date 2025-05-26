#include <stdio.h>
#include "lexer.h"
#include "parser.h"
#include "codegen.h"

void analyzeAST(ASTNode *node);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <sourcefile>\n", argv[0]);
        return 1;
    }

    runLexer(argv[1]);  // Tokenize source file

    currentTokenIndex = 0;
    ASTNode *ast = parseProgram();

    printf("\n=== Parsed AST ===\n");
    printAST(ast, 0);

    analyzeAST(ast);         // Run semantic checks
    printf("Semantic analysis successful.\n");

    // Generate TAC from AST
    generateCode(ast);
    printIntermediateCode("Initial");

    // Optimize TAC
    optimize();
    printIntermediateCode("Optimized");

    // Generate final assembly
    generateFinalCode();

    return 0;
}
