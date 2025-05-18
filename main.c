#include <stdio.h>
#include "lexer.h"
#include "parser.h"

//main.c
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <sourcefile>\n", argv[0]);
        return 1;
    }

    runLexer(argv[1]);  // Tokenize source file

    currentTokenIndex = 0;
    ASTNode *ast = parseFunction();

    printf("\n=== Parsed AST ===\n");
    printAST(ast, 0);

    return 0;
}
