%{
#include <stdio.h>
%}

%%

begin       { printf("(1,%s)\n", yytext); }
if          { printf("(2,%s)\n", yytext); }
then        { printf("(3,%s)\n", yytext); }
while       { printf("(4,%s)\n", yytext); }
do          { printf("(5,%s)\n", yytext); }
end         { printf("(6,%s)\n", yytext); }

[a-zA-Z][a-zA-Z0-9]*   { printf("(10,%s)\n", yytext); }
[0-9]+                  { printf("(11,%s)\n", yytext); }

"+"         { printf("(13,%s)\n", yytext); }
"-"         { printf("(14,%s)\n", yytext); }
"*"         { printf("(15,%s)\n", yytext); }
"/"         { printf("(16,%s)\n", yytext); }
":"         { printf("(17,%s)\n", yytext); }
":="        { printf("(18,%s)\n", yytext); }
"<"         { printf("(20,%s)\n", yytext); }
"<>"        { printf("(21,%s)\n", yytext); }
"<="        { printf("(22,%s)\n", yytext); }
">"         { printf("(23,%s)\n", yytext); }
">="        { printf("(24,%s)\n", yytext); }
"="         { printf("(25,%s)\n", yytext); }
";"         { printf("(26,%s)\n", yytext); }
"("         { printf("(27,%s)\n", yytext); }
")"         { printf("(28,%s)\n", yytext); }
"#"         { printf("(0,%s)\n", yytext); }

[ \t\n]+    ; /* 忽略空白字符 */
.           { printf("错误：未知字符 %s\n", yytext); }

%%

int main(int argc, char** argv) {
    yylex();
    return 0;
}

int yywrap() {
    return 1;
}
