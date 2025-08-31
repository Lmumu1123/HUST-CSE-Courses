#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

// Tokens
typedef enum {
    BEGIN = 1, IF, THEN, WHILE, DO, END, ID = 10, NUM, PLUS = 13, MINUS, TIMES, DIVIDE, COLON = 17, ASSIGN, LT = 20, NEQ, LEQ, GT = 23, GEQ, EQ, SEMICOLON = 26, LPAREN, RPAREN, HASH = 0
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[100];
} Token;

const char* keywords[] = { "begin", "if", "then", "while", "do", "end" };
const int keywordTokens[] = { BEGIN, IF, THEN, WHILE, DO, END };

Token getNextToken(const char** str) {
    Token token;
    token.lexeme[0] = '\0';
    while (isspace(**str)) (*str)++;
    if (isalpha(**str)) {
        int length = 0;
        while (isalnum(**str)) {
            token.lexeme[length++] = **str;
            (*str)++;
        }
        token.lexeme[length] = '\0';
        for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
            if (strcmp(token.lexeme, keywords[i]) == 0) {
                token.type = (TokenType)keywordTokens[i];
                return token;
            }
        }
        token.type = ID;
    }
    else if (isdigit(**str)) {
        int length = 0;
        while (isdigit(**str)) {
            token.lexeme[length++] = **str;
            (*str)++;
        }
        token.lexeme[length] = '\0';
        token.type = NUM;
    }
    else {
        switch (**str) {
        case '+': token.type = PLUS; token.lexeme[0] = '+'; token.lexeme[1] = '\0'; (*str)++; break;
        case '-': token.type = MINUS; token.lexeme[0] = '-'; token.lexeme[1] = '\0'; (*str)++; break;
        case '*': token.type = TIMES; token.lexeme[0] = '*'; token.lexeme[1] = '\0'; (*str)++; break;
        case '/': token.type = DIVIDE; token.lexeme[0] = '/'; token.lexeme[1] = '\0'; (*str)++; break;
        case ':':
            (*str)++;
            if (**str == '=') {
                token.type = ASSIGN;
                token.lexeme[0] = ':';
                token.lexeme[1] = '=';
                token.lexeme[2] = '\0';
                (*str)++;
            }
            else {
                token.type = COLON;
                token.lexeme[0] = ':';
                token.lexeme[1] = '\0';
            }
            break;
        case '<':
            (*str)++;
            if (**str == '=') {
                token.type = LEQ;
                token.lexeme[0] = '<';
                token.lexeme[1] = '=';
                token.lexeme[2] = '\0';
                (*str)++;
            }
            else if (**str == '>') {
                token.type = NEQ;
                token.lexeme[0] = '<';
                token.lexeme[1] = '>';
                token.lexeme[2] = '\0';
                (*str)++;
            }
            else {
                token.type = LT;
                token.lexeme[0] = '<';
                token.lexeme[1] = '\0';
            }
            break;
        case '>':
            (*str)++;
            if (**str == '=') {
                token.type = GEQ;
                token.lexeme[0] = '>';
                token.lexeme[1] = '=';
                token.lexeme[2] = '\0';
                (*str)++;
            }
            else {
                token.type = GT;
                token.lexeme[0] = '>';
                token.lexeme[1] = '\0';
            }
            break;
        case '=': token.type = EQ; token.lexeme[0] = '='; token.lexeme[1] = '\0'; (*str)++; break;
        case ';': token.type = SEMICOLON; token.lexeme[0] = ';'; token.lexeme[1] = '\0'; (*str)++; break;
        case '(': token.type = LPAREN; token.lexeme[0] = '('; token.lexeme[1] = '\0'; (*str)++; break;
        case ')': token.type = RPAREN; token.lexeme[0] = ')'; token.lexeme[1] = '\0'; (*str)++; break;
        case '#': token.type = HASH; token.lexeme[0] = '#'; token.lexeme[1] = '\0'; (*str)++; break;
        default: token.type = HASH; token.lexeme[0] = '\0'; break;
        }
    }
    return token;
}

void printToken(Token token) {
    printf("(%d,%s)\n", token.type, token.lexeme);
}

int main() {
    char* code = (char*)malloc(500 * sizeof(char));
    if (code == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    int cnt = 0;
    char c;
    while ((c = getchar()) != EOF) {
        code[cnt++] = c;
    }
    code[cnt] = '\0'; // Null-terminate the string

    const char* code_ptr = code; // Use a pointer to traverse the code
    Token token;
    while (*code_ptr != '\0') {
        token = getNextToken(&code_ptr);
        if (token.type != HASH) {
            printToken(token);
        }
    }

    free(code); // Free allocated memory
    return 0;
}