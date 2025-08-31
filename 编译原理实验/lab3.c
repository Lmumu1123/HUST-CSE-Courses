#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// 定义四元式结构
typedef struct {
    char op[10];    // 操作符
    char arg1[10];  // 操作数1
    char arg2[10];  // 操作数2
    char result[10];// 结果
} Quadruple;

Quadruple quadruples[100]; // 存储四元式
int quad_count = 0;        // 四元式计数器
int temp_count = 0;        // 临时变量计数器
int label_count = 0;       // 标签计数器
char* current_token;       // 当前 token
char* input;               // 输入字符串
int pos = 0;               // 输入字符串位置

// 函数原型声明
char* parse_factor();
char* parse_term();
char* parse_expression();
char* parse_condition();
void parse_assignment();
void parse_block();
void parse_if();
void parse_while();
void parse_statement();
void parse_program();

// 生成新的临时变量
char* new_temp() {
    char* temp = (char*)malloc(10);
    sprintf(temp, "t%d", ++temp_count);
    return temp;
}

// 生成新的标签
char* new_label() {
    char* label = (char*)malloc(10);
    sprintf(label, "L%d", ++label_count);
    return label;
}

// 添加四元式
void add_quadruple(const char* op, const char* arg1, const char* arg2, const char* result) {
    strcpy(quadruples[quad_count].op, op);
    strcpy(quadruples[quad_count].arg1, arg1);
    strcpy(quadruples[quad_count].arg2, arg2);
    strcpy(quadruples[quad_count].result, result);
    quad_count++;
}

// 打印四元式
void print_quadruples() {
    for (int i = 0; i < quad_count; i++) {
        if (strcmp(quadruples[i].op, "label") == 0) {
            printf("%s:\n", quadruples[i].result);
        } else if (strcmp(quadruples[i].op, "ifFalse") == 0) {
            printf("ifFalse %s goto %s\n", quadruples[i].arg1, quadruples[i].result);
        } else if (strcmp(quadruples[i].op, "goto") == 0) {
            printf("goto %s\n", quadruples[i].result);
        } else if (strcmp(quadruples[i].op, ":=") == 0) {
            printf("%s = %s\n", quadruples[i].result, quadruples[i].arg1);
        } else {
            printf("%s = %s %s %s\n", quadruples[i].result, quadruples[i].arg1, quadruples[i].op, quadruples[i].arg2);
        }
    }
}

// 词法分析器：获取下一个 token
char* get_next_token() {
    while (isspace(input[pos]) || input[pos] == '\n') pos++; // 跳过空白字符和换行符
    if (input[pos] == '\0') return NULL;  // 输入结束
    char* token = (char*)malloc(10);
    int i = 0;

    // 读取多字符符号（如 :=, <=, >=, ==, !=）
    if (input[pos] == ':' && input[pos + 1] == '=') {
        token[i++] = input[pos++];
        token[i++] = input[pos++];
        token[i] = '\0';
        return token;
    }
    if (input[pos] == '<' && input[pos + 1] == '=') {
        token[i++] = input[pos++];
        token[i++] = input[pos++];
        token[i] = '\0';
        return token;
    }
    if (input[pos] == '>' && input[pos + 1] == '=') {
        token[i++] = input[pos++];
        token[i++] = input[pos++];
        token[i] = '\0';
        return token;
    }
    if (input[pos] == '=' && input[pos + 1] == '=') {
        token[i++] = input[pos++];
        token[i++] = input[pos++];
        token[i] = '\0';
        return token;
    }
    if (input[pos] == '!' && input[pos + 1] == '=') {
        token[i++] = input[pos++];
        token[i++] = input[pos++];
        token[i] = '\0';
        return token;
    }

    // 读取标识符或关键字
    if (isalpha(input[pos])) {
        while (isalnum(input[pos])) {
            token[i++] = input[pos++];
        }
        token[i] = '\0';
        return token;
    }

    // 读取数字
    if (isdigit(input[pos])) {
        while (isdigit(input[pos])) {
            token[i++] = input[pos++];
        }
        token[i] = '\0';
        return token;
    }

    // 读取单字符符号
    token[i++] = input[pos++];
    token[i] = '\0';
    return token;
}

// 匹配 token
void match(const char* expected) {
    if (current_token == NULL || strcmp(current_token, expected) != 0) {
        printf("Error: Expected '%s', but got '%s'\n", expected, current_token ? current_token : "NULL");
        exit(1);
    }
    current_token = get_next_token();
}

// 解析因子
char* parse_factor() {
    char* result;
    if (strcmp(current_token, "(") == 0) {
        current_token = get_next_token();
        result = parse_expression();
        match(")");
    } else if (isalpha(current_token[0]) || isdigit(current_token[0])) {
        result = strdup(current_token);
        current_token = get_next_token();
    } else {
        printf("Error: Invalid factor\n");
        exit(1);
    }
    return result;
}

// 解析项
char* parse_term() {
    char* result = parse_factor();
    while (current_token && (strcmp(current_token, "*") == 0 || strcmp(current_token, "/") == 0)) {
        char* op = strdup(current_token);
        current_token = get_next_token();
        char* arg2 = parse_factor();
        char* temp = new_temp();
        add_quadruple(op, result, arg2, temp);
        free(result);
        result = temp;
    }
    return result;
}

// 解析表达式
char* parse_expression() {
    char* result = parse_term();
    while (current_token && (strcmp(current_token, "+") == 0 || strcmp(current_token, "-") == 0)) {
        char* op = strdup(current_token);
        current_token = get_next_token();
        char* arg2 = parse_term();
        char* temp = new_temp();
        add_quadruple(op, result, arg2, temp);
        free(result);
        result = temp;
    }
    return result;
}

// 解析条件
char* parse_condition() {
    char* left = parse_expression();
    char* op = strdup(current_token);
    current_token = get_next_token();
    char* right = parse_expression();
    char* temp = new_temp();
    add_quadruple(op, left, right, temp);
    return temp;
}

// 解析赋值语句
void parse_assignment() {
    char* id = strdup(current_token);
    current_token = get_next_token();
    match(":=");
    char* expr = parse_expression();
    add_quadruple(":=", expr, "", id);
    match(";");
}

// 解析语句块
void parse_block() {
    match("{");
    while (current_token && strcmp(current_token, "}") != 0) {
        parse_statement();
    }
    match("}");
}

// 解析条件语句
void parse_if() {
    match("if");
    char* cond = parse_condition();
    char* label_end = new_label();
    add_quadruple("ifFalse", cond, "", label_end);
    parse_block();
    add_quadruple("label", "", "", label_end);
}

// 解析循环语句
void parse_while() {
    match("while");
    char* label_start = new_label();
    char* label_end = new_label();
    add_quadruple("label", "", "", label_start);
    char* cond = parse_condition();
    add_quadruple("ifFalse", cond, "", label_end);
    parse_block();
    add_quadruple("goto", "", "", label_start);
    add_quadruple("label", "", "", label_end);
}

// 解析语句
void parse_statement() {
    if (strcmp(current_token, "if") == 0) {
        parse_if();
    } else if (strcmp(current_token, "while") == 0) {
        parse_while();
    } else {
        parse_assignment();
    }
}

// 解析程序
void parse_program() {
    match("main");
    match("(");
    match(")");
    parse_block();
}

int main() {
    // 读取输入
    char buffer[1024];
    size_t len = 0;
    while (fgets(buffer + len, sizeof(buffer) - len, stdin)) {
        len += strlen(buffer + len);
    }
    buffer[len] = '\0';
    input = buffer;

    // 初始化
    quad_count = 0;
    temp_count = 0;
    label_count = 0;
    pos = 0;
    current_token = get_next_token();

    // 解析并生成四元式
    parse_program();
    print_quadruples();

    return 0;
}