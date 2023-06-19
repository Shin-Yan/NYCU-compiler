%{
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern int32_t line_num;  /* declared in scanner.l */
extern char buffer[];     /* declared in scanner.l */
extern FILE *yyin;        /* declared by lex */
extern char *yytext;      /* declared by lex */

extern int yylex(void); 
static void yyerror(const char *msg);
%}
%left AND OR NOT
%left '<' '>' '=' LE NE GE
%left '-'
%left '+'
%left '/' '%'
%left '*'
%token AND OR NOT LE NE GE ASSIGN
%token KWvar KWarray KWof KWboolean KWinteger KWreal KWstring KWtrue KWfalse KWdef KWreturn KWbegin KWend KWwhile KWdo KWif KWthen KWelse KWfor KWto KWprint KWread
%token ID DEC OCT FLOAT SCI STR 

%%
    /* Program */
ProgramName: ID ';' decl_list func_list compound KWend

    /* function declararions */
args: | arg_decl;
arg_decl: ids ':' scalar_type;
func_decl: ID '(' args ')' ':' scalar_type ';' | ID '(' args ')' ';';
func_def: ID '(' args ')' ':' scalar_type compound KWend | ID '(' args ')' compound KWend;
func_list: | func_list func_decl | func_list func_def;

    /* variable declarations */
ids: ID | ids ',' ID;
const_decl: KWvar ids ':' constant ';';
scalar_decl: KWvar ids ':' scalar_type ';';
array_decl: KWvar ids ':' KWarray integer_const KWof array_type';';
decl_list: | decl_list const_decl | decl_list scalar_decl | decl_list array_decl;

    /* scalar and array types */
scalar_type: KWinteger | KWboolean | KWreal | KWstring;
array_type: KWarray integer_const KWof array_type | scalar_type;

    /* constant */
constant: integer_const | FLOAT | SCI | STR | KWtrue | KWfalse;
integer_const: DEC | OCT;

    /* statements */
compound: KWbegin decl_list statements KWend;
statements: | statements statement;
statement: compound | return | simple | condition | while | for | procedure;

return: KWreturn expr ';';
assignment: var_ref ASSIGN expr ';';
print: KWprint expr ';';
read: KWread expr ';';
condition: KWif expr KWthen compound KWend KWif | KWif expr KWthen compound KWelse compound KWend KWif;
simple: print | read | assignment;
while: KWwhile expr KWdo compound KWend KWdo;
for: KWfor ID ASSIGN integer_const KWto integer_const KWdo compound KWend KWdo;
procedure: function_invoke ';';

    /* expression */
var_ref: ID | array_ref;
array_ref: array_ref '[' expr ']' | ID '[' expr ']';

function_invoke: ID '(' expr_list ')';

expr: '-' expr %prec '*'
    | expr '*' expr
    | expr '/' expr
    | expr '%' expr
    | expr '+' expr
    | expr '-' expr
    | expr '<' expr
    | expr '>' expr
    | expr '=' expr
    | expr GE expr
    | expr LE expr
    | expr NE expr
    | expr AND expr
    | expr OR expr
    | NOT expr
    | '(' expr ')'
    | constant | var_ref | function_invoke;

expr_list: | non_empty_expr_list;
non_empty_expr_list: expr | non_empty_expr_list ',' expr;

%%

void yyerror(const char *msg) {
    fprintf(stderr,
            "\n"
            "|-----------------------------------------------------------------"
            "---------\n"
            "| Error found in Line #%d: %s\n"
            "|\n"
            "| Unmatched token: %s\n"
            "|-----------------------------------------------------------------"
            "---------\n",
            line_num, buffer, yytext);
    exit(-1);
}

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./parser <filename>\n");
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        perror("fopen() failed:");
    }

    yyparse();

    printf("\n"
           "|--------------------------------|\n"
           "|  There is no syntactic error!  |\n"
           "|--------------------------------|\n");
    return 0;
}
