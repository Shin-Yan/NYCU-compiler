%{
#include "visitor/AstNodeVisitor.hpp"
#include "AST/AstDumper.hpp"
#include "AST/ast.hpp"
#include "AST/program.hpp"
#include "AST/decl.hpp"
#include "AST/variable.hpp"
#include "AST/ConstantValue.hpp"
#include "AST/function.hpp"
#include "AST/CompoundStatement.hpp"
#include "AST/print.hpp"
#include "AST/expression.hpp"
#include "AST/BinaryOperator.hpp"
#include "AST/UnaryOperator.hpp"
#include "AST/FunctionInvocation.hpp"
#include "AST/VariableReference.hpp"
#include "AST/assignment.hpp"
#include "AST/read.hpp"
#include "AST/if.hpp"
#include "AST/while.hpp"
#include "AST/for.hpp"
#include "AST/return.hpp"

#include <cassert>
#include <errno.h>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>

#define YYLTYPE yyltype
// the maximum string size
#define MAX_CSTR_LEN 32

typedef struct YYLTYPE {
    uint32_t first_line;
    uint32_t first_column;
    uint32_t last_line;
    uint32_t last_column;
} yyltype;

extern int32_t line_num;  /* declared in scanner.l */
extern char buffer[];     /* declared in scanner.l */
extern FILE *yyin;        /* declared by lex */
extern char *yytext;      /* declared by lex */

static AstNode *root;
static AstNodeVisitor *visitor = new AstDumper();

extern "C" int yylex(void);
static void yyerror(const char *msg);
extern int yylex_destroy(void);
%}

%code requires {
    class AstNode;
    struct Unary_Op;
    struct Binary_Op;
    struct Constant_Value;
    struct Name;
    #include<vector>
}

    /* For yylval */
%union {
    /* basic semantic value */
    char *identifier;
    char *const_str;
    float const_real;
    int const_int;

    AstNode *node;

    std::vector<AstNode*> *node_list;
    std::vector<Name*> *name_list;
    
    struct Constant_Value *const_val;
};

%type <identifier> ProgramName ID FunctionName
%type <node_list> DeclarationList FunctionList FormalArgList ArrRefList ExpressionList StatementList
%type <node_list> Declarations Functions FormalArgs ArrRefs Expressions Statements

%type <name_list> IdList

%type <node> FunctionDeclaration FunctionDefinition Statement CompoundStatement
%type <node> Function FormalArg Declaration Simple Condition While For Return 
%type <node> VariableReference ElseOrNot FunctionCall FunctionInvocation Expression

%type <const_str> ReturnType Type ScalarType ArrType ArrDecl
%type <const_str> VAR STRING_LITERAL TRUE FALSE  INTEGER REAL STRING BOOLEAN ARRAY
%type <const_val> LiteralConstant StringAndBoolean IntegerAndReal
%type <const_int> NegOrNot INT_LITERAL
%type <const_real> REAL_LITERAL 

    /* Follow the order in scanner.l */

    /* Delimiter */
%token COMMA SEMICOLON COLON
%token L_PARENTHESIS R_PARENTHESIS
%token L_BRACKET R_BRACKET

    /* Operator */
%token ASSIGN
%left OR
%left AND
%right NOT
%left LESS LESS_OR_EQUAL EQUAL GREATER GREATER_OR_EQUAL NOT_EQUAL
%left PLUS MINUS
%left MULTIPLY DIVIDE MOD
%right UNARY_MINUS

    /* Keyword */
%token ARRAY BOOLEAN INTEGER REAL STRING
%token END BEGIN_ /* Use BEGIN_ since BEGIN is a keyword in lex */
%token DO ELSE FOR IF THEN WHILE
%token DEF OF TO RETURN VAR
%token FALSE TRUE
%token PRINT READ

    /* Identifier */
%token ID

    /* Literal */
%token INT_LITERAL
%token REAL_LITERAL
%token STRING_LITERAL

%%

Program:
    ProgramName SEMICOLON
    /* ProgramBody */
    DeclarationList FunctionList CompoundStatement
    /* End of ProgramBody */
    END {

        // printf("Biggest compound statement with decl len %d and state len %d\n", dynamic_cast<CompoundStatementNode*>$5->getDeclListLen(), dynamic_cast<CompoundStatementNode*>$5->getStateListLen());
        // if($5 == NULL)
            // printf("Compound node pointer is NULL!\n");
        root = new ProgramNode(@1.first_line, @1.first_column, 
                                $1, "void", $3, $4, $5);

        free($1);
    }
;

ProgramName:
    ID
;

DeclarationList:
    Epsilon
    {
        $$ = NULL;
    }
    |
    Declarations
    {
        $$ = $1;
    }
;

Declarations:
    Declaration
    {
        std::vector<AstNode*> *decls = new std::vector<AstNode*>;
        if($1 != NULL)
            decls->push_back($1);
        $$ = decls;
    }
    |
    Declarations Declaration
    {
        $$ = $1;
        if($2 != NULL)
            $$->push_back($2);
    }
;

FunctionList:
    Epsilon
    {
        $$ = NULL;
    }
    |
    Functions
    {
        $$ = $1;
    }
;

Functions:
    Function
    {
        std::vector<AstNode*> *f_list = new std::vector<AstNode*>;
        f_list->push_back($1);
        $$ = f_list;
    }
    |
    Functions Function
    {
        $$ = $1;
        $$->push_back($2);
    }
;

Function:
    FunctionDeclaration
    {
        $$ = $1;
    }
    |
    FunctionDefinition
    {
        $$ = $1;
    }
;

FunctionDeclaration:
    FunctionName L_PARENTHESIS FormalArgList R_PARENTHESIS ReturnType SEMICOLON
    {
        $$ = new FunctionNode(@1.first_line, @1.first_column, $1, $5, $3, NULL);
    }
;

FunctionDefinition:
    FunctionName L_PARENTHESIS FormalArgList R_PARENTHESIS ReturnType
    CompoundStatement
    END
    {
        $$ = new FunctionNode(@1.first_line, @1.first_column, $1, $5, $3, $6);
    }
;

FunctionName:
    ID
;

FormalArgList:
    Epsilon
    {
        $$=NULL;
    }
    |
    FormalArgs
    {
        $$=$1;
    }
;

FormalArgs:
    FormalArg
    {
        std::vector<AstNode*> *decls = new std::vector<AstNode*>;
        decls->push_back($1);
        $$ = decls;
    }
    |
    FormalArgs SEMICOLON FormalArg
    {
        $$ = $1;
        $$->push_back($3);
    }
;

FormalArg:
    IdList COLON Type
    {
        // printf("Formal arg type: %s\n",$3);
        std::vector<AstNode*> *var_list = new std::vector<AstNode*>;
        for(unsigned int i = 0 ; i < $1->size() ; i++){
            VariableNode *v = new VariableNode($1->at(i)->line, $1->at(i)->col, $1->at(i)->id, $3, NULL);
            var_list->push_back(v);
        }
        $$ = new DeclNode(@1.first_line, @1.first_column, var_list);
    }
;

IdList:
    ID
    {
        $$=new std::vector<Name*>;
        Name *n=new Name;
        n->id=$1;
        n->line=@1.first_line;
        n->col=@1.first_column;
        $$->push_back(n);
    }
    |
    IdList COMMA ID
    {
        $$ = $1;
        Name *n = new Name;
        n->id = $3;
        n->line = @3.first_line;
        n->col = @3.first_column;
        $$->push_back(n);
    }
;

ReturnType:
    COLON ScalarType
    {
        $$ = $2;
    }
    |
    Epsilon
    {
        char s[MAX_CSTR_LEN+1];
        strcpy(s, "void");
        $$ = s;
    }
;

    /*
       Data Types and Declarations
                                   */

Declaration:
    VAR IdList COLON Type SEMICOLON
    {
        std::vector<AstNode*> *var_list = new std::vector<AstNode*>;
        for(unsigned int i = 0 ; i < $2->size(); i++){
            VariableNode *v = new VariableNode($2->at(i)->line, $2->at(i)->col, $2->at(i)->id, $4, NULL);
            var_list->push_back(v);
        }
        $$ = new DeclNode(@1.first_line, @1.first_column, var_list);
    }
    |
    VAR IdList COLON LiteralConstant SEMICOLON
    {
        std::vector<AstNode*> *var_list = new std::vector<AstNode*>;
        char s[MAX_CSTR_LEN+1];
        for(unsigned int i = 0 ; i < $2->size(); i++){
            ConstantValueNode *c = new ConstantValueNode($4->line, $4->col, *($4));
            if($4->int_type){
                strcpy(s, "integer");
            }
            else if($4->real_type){
                strcpy(s, "real"); 
            }
            else if($4->str_type){
                strcpy(s, "string");
            }
            else if($4->bool_type){
                strcpy(s, "boolean");
            }
            // printf("type is %s\n",s);
            VariableNode *v = new VariableNode($2->at(i)->line, $2->at(i)->col, $2->at(i)->id, s, c);
            var_list->push_back(v);
        }
        $$ = new DeclNode(@1.first_line, @1.first_column, var_list);
    }
;

Type:
    ScalarType
    {
        // printf("The scalar type: %s\n",$1);
        $$ = $1;
    }
    |
    ArrType
    {
        // printf("The array type: %s\n",$1);
        $$ = $1;
    }
;

ScalarType:
    INTEGER
    {
        char s[MAX_CSTR_LEN+1];
        strcpy(s, "integer"); 
        $$ = s;
    }
    |
    REAL
    {
        char s[MAX_CSTR_LEN+1];
        strcpy(s, "real"); 
        $$ = s;
    }
    |
    STRING
    {
        char s[MAX_CSTR_LEN+1];
        strcpy(s, "string");   
        $$ = s;
    }
    |
    BOOLEAN
    {
        char s[MAX_CSTR_LEN+1];
        strcpy(s, "boolean");   
        $$ = s;
    }
;

ArrType:
    ArrDecl ScalarType
    {
        char s1[MAX_CSTR_LEN+1];
        char s2[MAX_CSTR_LEN+1];
        char s3[MAX_CSTR_LEN+1];
        strcpy(s1, $2);
        strcpy(s3, $1);

        strcpy(s2," ");
        strcat(s1, s2);
        strcat(s1, s3);
        $$ = s1;
    }
;

ArrDecl:
    ARRAY INT_LITERAL OF
    {
        // printf("In ArrDecl, INT_LITERAL = %d", $2);
        char s1[MAX_CSTR_LEN+1];
        char s2[MAX_CSTR_LEN+1];
        strcpy(s1,"[");
        int val = $2;
        sprintf(s2, "%d", val);
        strcat(s1, s2);
        strcpy(s2, "]");
        strcat(s1,s2);
        $$ = s1;
    }
    |
    ArrDecl ARRAY INT_LITERAL OF
    {
        char s1[MAX_CSTR_LEN+1];
        char s2[MAX_CSTR_LEN+1];
        strcpy(s1, $1);
        strcpy(s2,"[");
        strcat(s1,s2);
        int val = $3;
        sprintf(s2, "%d", val);
        strcat(s1, s2);
        strcpy(s2,"]");
        strcat(s1,s2);
        $$ = s1;
    }
;

LiteralConstant:
    NegOrNot INT_LITERAL
    {
        $$ = new Constant_Value;
        $$->int_value = $1 * $2;
        $$->int_type = true;
        if($1==1){
            $$->col= @2.first_column;
            $$->line=@2.first_line;
        }
        else{
            $$->col= @1.first_column;
            $$->line=@1.first_line;
        }
    }
    |
    NegOrNot REAL_LITERAL
    {
        $$ = new Constant_Value;
        $$->real_value = $1 * $2;
        $$->real_type = true;
        if($1==1){
            $$->col= @2.first_column;
            $$->line=@2.first_line;
        }
        else{
            $$->col= @1.first_column;
            $$->line=@1.first_line;
        }
    }
    |
    StringAndBoolean
    {
        // printf("Literal constant!\n");
        $$=$1;
        $$->col= @1.first_column;
        $$->line=@1.first_line;
    }
;

NegOrNot:
    Epsilon
    {
        $$ = 1;
    }
    |
    MINUS %prec UNARY_MINUS
    {
        $$ = -1;
    }
;

StringAndBoolean:
    STRING_LITERAL
    {
        // printf("find a new string constant: %s\n", $1);
        $$ = new Constant_Value;
        $$->str_value = $1;
        $$->str_type = true;
    }
    |
    TRUE
    {
        // printf("StringAndBoolean!\n");
        $$ = new Constant_Value;
        $$->str_value = "true";
        $$->bool_type = true;
    }
    |
    FALSE
    {
        $$ = new Constant_Value;
        $$->str_value = "false";
        $$->bool_type = true;
    }
;

IntegerAndReal:
    INT_LITERAL
    {
        // printf("find a new int constant:%d!\n",$1);
        $$ = new Constant_Value;
        $$->int_value = $1;
        $$->int_type = true;
    }
    |
    REAL_LITERAL
    {
        // printf("find a new real constant:%lf!\n",$1);
        $$ = new Constant_Value;
        $$->real_value = $1;
        $$->real_type = true;
    }
;

    /*
       Statements
                  */

Statement:
    CompoundStatement
    {
        $$ = $1;
    }
    |
    Simple
    {
        $$ = $1;
    }
    |
    Condition
    {
        $$ = $1;
    }
    |
    While
    {
        $$ = $1;
    }
    |
    For
    {
        $$ = $1;
    }
    |
    Return
    {
        $$ = $1;
    }
    |
    FunctionCall
    {
        $$ = $1;
    }
;

CompoundStatement:
    BEGIN_
    DeclarationList
    StatementList
    END
    {
        // printf("There is a compound statemnt!\n");
        // printf("Length of the statement is %d\n", $3->size());
        $$ = new CompoundStatementNode(@1.first_line, @1.first_column, $2, $3);
        // $$ = cmsn;
        // $$ = NULL;
    }
;

Simple:
    VariableReference ASSIGN Expression SEMICOLON
    {
        // printf("find an assignment!\n");
        $$ = new AssignmentNode(@2.first_line, @2.first_column, $1, $3);
    }
    |
    PRINT Expression SEMICOLON
    {
        // printf("There is a print!\n");
        $$ = new PrintNode(@1.first_line, @1.first_column, $2);
    }
    |
    READ VariableReference SEMICOLON
    {
        $$ = new ReadNode(@1.first_line, @1.first_column, $2);
    }
;

VariableReference:
    ID ArrRefList
    {
        // printf("find a var ref: %s!\n",$1);
        $$ = new VariableReferenceNode(@1.first_line, @1.first_column, $1, $2);
    }
;

ArrRefList:
    Epsilon
    {
        $$ = NULL;
    }
    |
    ArrRefs
    {
        $$ = $1;
    }
;

ArrRefs:
    L_BRACKET Expression R_BRACKET
    {
        std::vector<AstNode*> *v = new std::vector<AstNode*>;
        v->push_back($2);
        $$ = v;
    }
    |
    ArrRefs L_BRACKET Expression R_BRACKET
    {
        $$ = $1;
        $$->push_back($3);
    }
;

Condition:
    IF Expression THEN
    CompoundStatement
    ElseOrNot
    END IF
    {
        // printf("There is a condition!\n");
        if($4 == NULL)
            $$ = new IfNode(@1.first_line, @1.first_column, $2, $4, NULL);
        else
            $$ = new IfNode(@1.first_line, @1.first_column, $2, $4, $5);
        // $$ = NULL;
    }
;

ElseOrNot:
    ELSE
    CompoundStatement
    {
        $$ = $2;
    }
    |
    Epsilon
    {
        $$ = NULL;
    }
;

While:
    WHILE Expression DO
    CompoundStatement
    END DO
    {
        $$ = new WhileNode(@1.first_line, @1.first_column, $2, $4);
    }
;

For:
    FOR ID ASSIGN INT_LITERAL TO INT_LITERAL DO
    CompoundStatement
    END DO
    {
        // Declaration
        std::vector<AstNode*> *var_list = new std::vector<AstNode*>;
        VariableNode *v = new VariableNode(@2.first_line, @2.first_column, $2, "integer", NULL);
        var_list->push_back(v);
        DeclNode *d = new DeclNode(@2.first_line, @2.first_column, var_list);

        // Assignment
        VariableReferenceNode *vr = new VariableReferenceNode(@2.first_line, @2.first_column, $2, NULL);
        Constant_Value *cv1 = new Constant_Value;
        cv1->int_value = $4;
        cv1->int_type = true;
        ConstantValueNode *c1 = new ConstantValueNode(@4.first_line, @4.first_column, *(cv1));
        AssignmentNode *a = new AssignmentNode(@3.first_line, @3.first_column, vr, c1);

        // condition
        Constant_Value *cv2 = new Constant_Value;
        cv2->int_value = $6;
        cv2->int_type = true;
        ConstantValueNode *c2 = new ConstantValueNode(@6.first_line, @6.first_column, *(cv2));

        // for
        $$ = new ForNode(@1.first_line, @1.first_column, d, a, c2, $8);
    }
;

Return:
    RETURN Expression SEMICOLON
    {
        $$ = new ReturnNode(@1.first_line, @1.first_column, $2);
    }
;

FunctionCall:
    FunctionInvocation SEMICOLON
    {
        $$ = $1;
    }
;

FunctionInvocation:
    ID L_PARENTHESIS ExpressionList R_PARENTHESIS
    {
        $$ = new FunctionInvocationNode(@1.first_line, @1.first_column, $1, $3);
    }
;

ExpressionList:
    Epsilon
    {
        $$ = NULL;
    }
    |
    Expressions
    {
        $$ = $1;
    }
;

Expressions:
    Expression
    {
        std::vector<AstNode*> *v = new std::vector<AstNode*>;
        v->push_back($1);
        $$=v;
    }
    |
    Expressions COMMA Expression
    {
        $$ = $1;
        $$->push_back($3);
    }
;

StatementList:
    Epsilon
    {
        $$ = NULL;
    }
    |
    Statements
    {
        // printf("There is a statement list!\n");
        $$ = $1;
    }
;

Statements:
    Statement
    {
        std::vector<AstNode*> *v = new std::vector<AstNode*>;
        if($1 != NULL)
            v->push_back($1);
        $$ = v;
        // $$ = NULL;
    }
    |
    Statements Statement
    {
        $$ = $1;
        if($2 != NULL)
            $$->push_back($2);
        // $$ = NULL;
    }
;

Expression:
    L_PARENTHESIS Expression R_PARENTHESIS
    {
        $$ = $2;
    }
    |
    MINUS Expression %prec UNARY_MINUS
    {
        Unary_Op u;
        u.neg = true;

        $$ = new UnaryOperatorNode(@1.first_line, @1.first_column, u ,$2);
    }
    |
    Expression MULTIPLY Expression
    {
        Binary_Op b;
        b.mul = true;

        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, b, $1, $3);
    }
    |
    Expression DIVIDE Expression
    {
        Binary_Op b;
        b.div = true;

        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, b, $1, $3);
    }
    |
    Expression MOD Expression
    {
        Binary_Op b;
        b.mod = true;

        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, b, $1, $3);
    }
    |
    Expression PLUS Expression
    {
        Binary_Op b;
        b.add = true;
        // printf("find bin oper: +!\n");
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, b, $1, $3);
    }
    |
    Expression MINUS Expression
    {
        Binary_Op b;
        b.sub = true;

        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, b, $1, $3);
    }
    |
    Expression LESS Expression
    {
        Binary_Op b;
        b.lt = true;
        // printf("find bin oper: <!\n");
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, b, $1, $3);
    }
    |
    Expression LESS_OR_EQUAL Expression
    {
        Binary_Op b;
        b.le = true;

        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, b, $1, $3);
    }
    |
    Expression GREATER Expression
    {
        Binary_Op b;
        b.gt = true;

        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, b, $1, $3);
    }
    |
    Expression GREATER_OR_EQUAL Expression
    {
        Binary_Op b;
        b.ge = true;

        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, b, $1, $3);
    }
    |
    Expression EQUAL Expression
    {
        Binary_Op b;
        b.eq = true;
        // printf("There is an equal operator!\n");
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, b, $1, $3);
        // $$ = NULL;
    }
    |
    Expression NOT_EQUAL Expression
    {
        Binary_Op b;
        b.ne = true;

        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, b, $1, $3);
    }
    |
    NOT Expression
    {
        Unary_Op u;
        u.NOT = true;

        $$ = new UnaryOperatorNode(@1.first_line, @1.first_column, u, $2);
    }
    |
    Expression AND Expression
    {
        Binary_Op b;
        b.AND = true;

        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, b, $1, $3);
    }
    |
    Expression OR Expression
    {
        Binary_Op b;
        b.OR = true;

        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, b, $1, $3);
    }
    |
    IntegerAndReal
    {
        $$ = new ConstantValueNode(@1.first_line, @1.first_column, *($1));
    }
    |
    StringAndBoolean
    {
        // printf("There is a string or boolean: %s\n", $1->str_value);
        $$ = new ConstantValueNode(@1.first_line, @1.first_column, *($1));
    }
    |
    VariableReference
    {
        // printf("There is a variable reference!\n");
        $$ = $1;
    }
    |
    FunctionInvocation
    {
        $$ = $1;
    }
;

    /*
       misc
            */
Epsilon:
;

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
    if (argc < 2) {
        fprintf(stderr, "Usage: ./parser <filename> [--dump-ast]\n");
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        perror("fopen() failed:");
    }

    yyparse();

    if (argc >= 3 && strcmp(argv[2], "--dump-ast") == 0) {
        root->accept(*visitor);
    }

    printf("\n"
           "|--------------------------------|\n"
           "|  There is no syntactic error!  |\n"
           "|--------------------------------|\n");

    delete root;
    fclose(yyin);
    yylex_destroy();
    return 0;
}
