#ifndef LEXER_H
#define LEXER_H

#define MAX_TOKENS 1000
#define MAX_LEXEME_LEN 100

// Token type enum or defines
enum TokenType {
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_CHAR,
    TOKEN_OPERATOR,
    TOKEN_PUNCTUATION,
    TOKEN_COMMENT,
    TOKEN_PREPROCESSOR,
    TOKEN_UNKNOWN
};

typedef struct {
    int type;                      
    char lexeme[MAX_LEXEME_LEN];
    int line;
} Token;

extern Token tokenTable[MAX_TOKENS];
extern int tokenCount;

void runLexer(const char *filename);
void printTokenTable(void);
void exportTokenTable(const char *outFilename);
void syntaxError(const char *message, Token *tok);


#endif


