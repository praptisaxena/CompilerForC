// main.c
#include <stdio.h>
#include "lexer.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <source-file.c>\n", argv[0]);
        return 1;
    }

    runLexer(argv[1]);
    return 0;
}
