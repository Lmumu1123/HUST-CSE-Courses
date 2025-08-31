#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKEN_LEN 100

// 词法分析器状态
typedef enum {
    START,      // 初始状态
    INID,       // 标识符状态
    INNUM,      // 数字状态
    INASSIGN,   // 赋值状态 (:=)
    DONE        // 结束状态
} StateType;

// 保存关键字及其对应的种别码
typedef struct {
    char* word;
    int code;
} KeywordCode;

// 关键字表
KeywordCode keywords[] = {
    {"begin", 1},
    {"if", 2},
    {"then", 3},
    {"while", 4},
    {"do", 5},
    {"end", 6}
};

int num_keywords = sizeof(keywords) / sizeof(KeywordCode);

// 获取下一个字符
char getNextChar(FILE *source) {
    return getc(source);
}

// 回退一个字符
void ungetNextChar(char c, FILE *source) {
    ungetc(c, source);
}

// 判断是否为关键字，如果是返回种别码，否则返回-1
int isKeyword(char *token) {
    for (int i = 0; i < num_keywords; i++) {
        if (strcmp(token, keywords[i].word) == 0) {
            return keywords[i].code;
        }
    }
    return -1;
}

// 获取下一个词法单元
void getNextToken(FILE *source, int *token_code, char *token_string) {
    int token_index = 0;
    StateType state = START;
    int save;
    char c;

    // 清空token_string
    token_string[0] = '\0';

    while (state != DONE) {
        c = getNextChar(source);
        save = 1;

        switch (state) {
            case START:
                if (isspace(c)) {
                    save = 0;
                } else if (isalpha(c)) {
                    state = INID;
                } else if (isdigit(c)) {
                    state = INNUM;
                } else if (c == ':') {
                    state = INASSIGN;
                } else {
                    state = DONE;
                    switch (c) {
                        case EOF:
                            save = 0;
                            *token_code = 0;  // #对应种别码0
                            strcpy(token_string, "#");
                            break;
                        case '+':
                            *token_code = 13;
                            break;
                        case '-':
                            *token_code = 14;
                            break;
                        case '*':
                            *token_code = 15;
                            break;
                        case '/':
                            *token_code = 16;
                            break;
                        case '=':
                            *token_code = 25;
                            break;
                        case ';':
                            *token_code = 26;
                            break;
                        case '(':
                            *token_code = 27;
                            break;
                        case ')':
                            *token_code = 28;
                            break;
                        case '<':
                            {
                                char next = getNextChar(source);
                                if (next == '=') {
                                    *token_code = 22;  // <=
                                    token_string[token_index++] = c;
                                    c = next;
                                } else if (next == '>') {
                                    *token_code = 21;  // <>
                                    token_string[token_index++] = c;
                                    c = next;
                                } else {
                                    ungetNextChar(next, source);
                                    *token_code = 20;  // <
                                }
                            }
                            break;
                        case '>':
                            {
                                char next = getNextChar(source);
                                if (next == '=') {
                                    *token_code = 24;  // >=
                                    token_string[token_index++] = c;
                                    c = next;
                                } else {
                                    ungetNextChar(next, source);
                                    *token_code = 23;  // >
                                }
                            }
                            break;
                        default:
                            *token_code = -1;  // 错误标记
                    }
                }
                break;

            case INID:
                if (!isalnum(c)) {
                    ungetNextChar(c, source);
                    save = 0;
                    state = DONE;
                    
                    // 判断是否为关键字
                    int keyword_code = isKeyword(token_string);
                    if (keyword_code != -1) {
                        *token_code = keyword_code;
                    } else {
                        *token_code = 10;  // ID种别码
                    }
                }
                break;

            case INNUM:
                if (!isdigit(c)) {
                    ungetNextChar(c, source);
                    save = 0;
                    state = DONE;
                    *token_code = 11;  // NUM种别码
                }
                break;
                
            case INASSIGN:
                state = DONE;
                if (c == '=') {
                    *token_code = 18;  // :=
                } else {
                    ungetNextChar(c, source);
                    save = 0;
                    *token_code = 17;  // :
                }
                break;
                
            case DONE:
                break;
                
            default:
                fprintf(stderr, "Scanner Bug: state= %d\n", state);
                state = DONE;
                *token_code = -1;
                break;
        }

        if (save && token_index < MAX_TOKEN_LEN) {
            token_string[token_index++] = c;
            token_string[token_index] = '\0';
        }
    }
}

int main() {
    FILE *source;
    int token_code;
    char token_string[MAX_TOKEN_LEN + 1];

    // 从标准输入读取源代码
    source = stdin;

    // 获取并打印所有词法单元
    while (1) {
        getNextToken(source, &token_code, token_string);
        
        if (token_code == 0 && strcmp(token_string, "#") == 0) {
            // 遇到EOF时结束
            break;
        }
        
        printf("(%d,%s)\n", token_code, token_string);
    }

    return 0;
}
