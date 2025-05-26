#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

Token tokenTable[MAX_TOKENS];
int tokenCount = 0;

char *keywords[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if", "int",
    "long", "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while"
};

int isKeyword(char *str) {
    for (int i = 0; i < 32; i++)
        if (strcmp(str, keywords[i]) == 0)
            return 1;
    return 0;
}

int isOperator(char ch) {
    return strchr("+-*/=<>!&|%^", ch) != NULL;
}

int isPunctuation(char ch) {
    return strchr(";,(){}[]", ch) != NULL;
}

void addToken(int type, const char *lexeme, int line) {
    tokenTable[tokenCount].type = type;
    strncpy(tokenTable[tokenCount].lexeme, lexeme, MAX_LEXEME_LEN);
    tokenTable[tokenCount].line = line;
    tokenCount++;
}

void printTokenTable() {
    printf("\n%-15s %-20s %-10s\n", "TOKEN TYPE", "LEXEME", "LINE");
    printf("-----------------------------------------------------\n");
    for (int i = 0; i < tokenCount; i++) {
        // You need a helper to print type names as strings:
        const char *typeName;
        switch(tokenTable[i].type) {
            case TOKEN_KEYWORD: typeName = "KEYWORD"; break;
            case TOKEN_IDENTIFIER: typeName = "IDENTIFIER"; break;
            case TOKEN_NUMBER: typeName = "NUMBER"; break;
            case TOKEN_FLOAT: typeName = "FLOAT"; break;
            case TOKEN_STRING: typeName = "STRING"; break;
            case TOKEN_CHAR: typeName = "CHAR"; break;
            case TOKEN_OPERATOR: typeName = "OPERATOR"; break;
            case TOKEN_PUNCTUATION: typeName = "PUNCTUATION"; break;
            case TOKEN_COMMENT: typeName = "COMMENT"; break;
            case TOKEN_PREPROCESSOR: typeName = "PREPROCESSOR"; break;
            case TOKEN_UNKNOWN: typeName = "UNKNOWN"; break;
            default: typeName = "INVALID"; break;
        }

        printf("%-15s %-20s %-10d\n", typeName, tokenTable[i].lexeme, tokenTable[i].line);
    }
}

void exportTokenTable(const char *outFilename) {
    FILE *out = fopen(outFilename, "w");
    if (!out) {
        printf("Failed to write token table.\n");
        return;
    }
    fprintf(out, "%-15s %-20s %-10s\n", "TOKEN TYPE", "LEXEME", "LINE");
    fprintf(out, "-----------------------------------------------------\n");
    for (int i = 0; i < tokenCount; i++) {
        const char *typeName;
        switch(tokenTable[i].type) {
            case TOKEN_KEYWORD: typeName = "KEYWORD"; break;
            case TOKEN_IDENTIFIER: typeName = "IDENTIFIER"; break;
            case TOKEN_NUMBER: typeName = "NUMBER"; break;
            case TOKEN_FLOAT: typeName = "FLOAT"; break;
            case TOKEN_STRING: typeName = "STRING"; break;
            case TOKEN_CHAR: typeName = "CHAR"; break;
            case TOKEN_OPERATOR: typeName = "OPERATOR"; break;
            case TOKEN_PUNCTUATION: typeName = "PUNCTUATION"; break;
            case TOKEN_COMMENT: typeName = "COMMENT"; break;
            case TOKEN_PREPROCESSOR: typeName = "PREPROCESSOR"; break;
            case TOKEN_UNKNOWN: typeName = "UNKNOWN"; break;
            default: typeName = "INVALID"; break;
        }
        fprintf(out, "%-15s %-20s %-10d\n", typeName, tokenTable[i].lexeme, tokenTable[i].line);
    }
    fclose(out);
    printf("✅ Token table exported to %s\n", outFilename);
}

void runLexer(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("❌ Cannot open file.\n");
        return;
    }

    char ch, buffer[100];
    int i = 0, line = 1;

    while ((ch = fgetc(fp)) != EOF) {
        if (isspace(ch)) {
            if (ch == '\n') line++;
            continue;
        }

        if (ch == '#') {
            i = 0;
            buffer[i++] = ch;
            while ((ch = fgetc(fp)) != '\n' && ch != EOF)
                buffer[i++] = ch;
            buffer[i] = '\0';
            addToken(TOKEN_PREPROCESSOR, buffer, line++);
            continue;
        }

        if (isalpha(ch) || ch == '_') {
            i = 0;
            buffer[i++] = ch;
            while (isalnum(ch = fgetc(fp)) || ch == '_')
                buffer[i++] = ch;
            buffer[i] = '\0';
            ungetc(ch, fp);
            if (isKeyword(buffer))
                addToken(TOKEN_KEYWORD, buffer, line);
            else
                addToken(TOKEN_IDENTIFIER, buffer, line);
        }

        else if (isdigit(ch)) {
            i = 0;
            buffer[i++] = ch;
            int isFloat = 0;
            while (isdigit(ch = fgetc(fp)) || ch == '.') {
                if (ch == '.') isFloat = 1;
                buffer[i++] = ch;
            }
            buffer[i] = '\0';
            ungetc(ch, fp);
            addToken(isFloat ? TOKEN_FLOAT : TOKEN_NUMBER, buffer, line);
        }

        else if (ch == '"') {
            i = 0;
            buffer[i++] = ch;
            while ((ch = fgetc(fp)) != '"' && ch != EOF) {
                if (ch == '\\') buffer[i++] = ch;
                buffer[i++] = ch;
            }
            buffer[i++] = '"';
            buffer[i] = '\0';
            addToken(TOKEN_STRING, buffer, line);
        }

        else if (ch == '\'') {
            i = 0;
            buffer[i++] = ch;
            ch = fgetc(fp);
            if (ch == '\\') buffer[i++] = ch;
            buffer[i++] = fgetc(fp); // actual char or escape
            buffer[i++] = fgetc(fp); // closing '
            buffer[i] = '\0';
            addToken(TOKEN_CHAR, buffer, line);
        }

        else if (ch == '/') {
            char next = fgetc(fp);
            if (next == '/') {
                i = 0;
                buffer[i++] = '/';
                buffer[i++] = '/';
                while ((ch = fgetc(fp)) != '\n' && ch != EOF)
                    buffer[i++] = ch;
                buffer[i] = '\0';
                addToken(TOKEN_COMMENT, buffer, line++);
            } else if (next == '*') {
                i = 0;
                buffer[i++] = '/';
                buffer[i++] = '*';
                while ((ch = fgetc(fp)) != EOF) {
                    buffer[i++] = ch;
                    if (ch == '*' && (next = fgetc(fp)) == '/') {
                        buffer[i++] = '/';
                        break;
                    } else if (ch == '\n') line++;
                }
                buffer[i] = '\0';
                addToken(TOKEN_COMMENT, buffer, line);
            } else {
                buffer[0] = '/'; buffer[1] = '\0';
                ungetc(next, fp);
                addToken(TOKEN_OPERATOR, buffer, line);
            }
        }

        else if (isOperator(ch)) {
            i = 0;
            buffer[i++] = ch;
            char next = fgetc(fp);
            if ((ch == '=' && next == '=') || (ch == '!' && next == '=') ||
                (ch == '<' && next == '=') || (ch == '>' && next == '=') ||
                (ch == '&' && next == '&') || (ch == '|' && next == '|') ||
                (ch == '+' && next == '+') || (ch == '-' && next == '-')) {
                buffer[i++] = next;
            } else {
                ungetc(next, fp);
            }
            buffer[i] = '\0';
            addToken(TOKEN_OPERATOR, buffer, line);
        }

        else if (isPunctuation(ch)) {
            buffer[0] = ch; buffer[1] = '\0';
            addToken(TOKEN_PUNCTUATION, buffer, line);
        }

        else {
            buffer[0] = ch; buffer[1] = '\0';
            addToken(TOKEN_UNKNOWN, buffer, line);
        }
    }

    fclose(fp);
    printTokenTable();
    exportTokenTable("tokens.txt");
}

