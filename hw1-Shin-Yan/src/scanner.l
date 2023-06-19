%{
#include <stdint.h>

#define LIST                concatenateString(yytext)
#define TOKEN(t)            { LIST; if (opt_tok) printf("<%s>\n", #t); }
#define TOKEN_CHAR(t)       { LIST; if (opt_tok) printf("<%c>\n", (t)); }
#define TOKEN_STRING(t, s)  { LIST; if (opt_tok) printf("<%s: %s>\n", #t, (s)); }
#define MAX_LINE_LENG       512

uint32_t line_num = 1;
static char buffer[MAX_LINE_LENG];

static uint32_t opt_src = 1;
static uint32_t opt_tok = 1;
static char *buffer_ptr = buffer;

static void concatenateString(const char *yytext_ptr);

%}
    /* Definition Section */
digit           [0-9]
letter          [a-zA-Z]
integer         0|[1-9]{digit}*
floating        {integer}\.(0|{digit}*[1-9])
octal           0[0-7]+
real            ([1-9]{digit}*)|([1-9]{digit}*\.(0|{digit}*[1-9]))|(0\.{digit}*[1-9])
scientific      {real}[eE][-+]?{integer}
ascii           [^\"\n]

%s C_COMMENT 
%s CPP_COMMENT
%s PSEUDO_COMMENT
%s STRING

%%
    /* Rules Section */

    /* Comment Rules */
<INITIAL>"/*"    {LIST, BEGIN C_COMMENT;}
<C_COMMENT>"*/"  {LIST, BEGIN INITIAL;}
<C_COMMENT>.     {LIST;}
<INITIAL>"//"    {LIST, BEGIN CPP_COMMENT;}
<CPP_COMMENT>\n  {BEGIN INITIAL; {
                    if (opt_src) {
                        printf("%d: %s\n", line_num, buffer);
                    }
                    ++line_num;
                    buffer[0] = '\0';
                    buffer_ptr = buffer;
                    }
                }
<CPP_COMMENT>.   {LIST;}
    /* Pseudo Comments*/
<INITIAL>"//&"    {LIST; BEGIN PSEUDO_COMMENT; }
<PSEUDO_COMMENT>"S+"({ascii}|"\"")*  {LIST; opt_src = 1;}
<PSEUDO_COMMENT>"S-"({ascii}|"\"")*  {LIST; opt_src = 0;}
<PSEUDO_COMMENT>"T+"({ascii}|"\"")*  {LIST; opt_tok = 1;}
<PSEUDO_COMMENT>"T-"({ascii}|"\"")*  {LIST; opt_tok = 0;}
<PSEUDO_COMMENT>\n  {BEGIN INITIAL; {
                        if (opt_src) {
                            printf("%d: %s\n", line_num, buffer);
                        }
                        ++line_num;
                        buffer[0] = '\0';
                        buffer_ptr = buffer;
                        }
                    }

    /* Delimiters */
<INITIAL>","     { TOKEN_CHAR(','); }
<INITIAL>";"     { TOKEN_CHAR(';'); }
<INITIAL>":"     { TOKEN_CHAR(':'); }
<INITIAL>"("     { TOKEN_CHAR('('); }
<INITIAL>")"     { TOKEN_CHAR(')'); }
<INITIAL>"["     { TOKEN_CHAR('['); }
<INITIAL>"]"     { TOKEN_CHAR(']'); }

    /* Operator */
<INITIAL>"+"     { TOKEN_CHAR('+'); }
<INITIAL>"-"     { TOKEN_CHAR('-'); }
<INITIAL>"*"     { TOKEN_CHAR('*'); }
<INITIAL>"/"     { TOKEN_CHAR('/'); }
<INITIAL>mod     { TOKEN(mod); }
<INITIAL>":="    { TOKEN(:=); }
<INITIAL>"<"     { TOKEN_CHAR('<'); }
<INITIAL>"<="    { TOKEN(<=); }
<INITIAL>"<>"    { TOKEN(<>); }
<INITIAL>">="    { TOKEN(>=); }
<INITIAL>">"     { TOKEN_CHAR('>'); }
<INITIAL>"="     { TOKEN_CHAR('='); }
<INITIAL>and     { TOKEN(and); }
<INITIAL>or      { TOKEN(or); }
<INITIAL>not     { TOKEN(not); }

    /* Discarded Tokens*/
<INITIAL>[ \t]   { LIST; }

    /* Reserved Words */
<INITIAL>var     { TOKEN(KWvar); }
<INITIAL>def     { TOKEN(KWdef); }
<INITIAL>array   { TOKEN(KWarray); }
<INITIAL>of      { TOKEN(KWof); }
<INITIAL>boolean { TOKEN(KWboolean); }
<INITIAL>integer { TOKEN(KWinteger); }
<INITIAL>real    { TOKEN(KWreal); }
<INITIAL>string  { TOKEN(KWstring); }
<INITIAL>true    { TOKEN(KWtrue); }
<INITIAL>false   { TOKEN(KWfalse); }
<INITIAL>while   { TOKEN(KWwhile); }
<INITIAL>do      { TOKEN(KWdo); }
<INITIAL>if      { TOKEN(KWif); }
<INITIAL>then    { TOKEN(KWthen); }
<INITIAL>else    { TOKEN(KWelse); }
<INITIAL>for     { TOKEN(KWfor); }
<INITIAL>to      { TOKEN(KWto); }
<INITIAL>begin   { TOKEN(KWbegin); }
<INITIAL>end     { TOKEN(KWend); }
<INITIAL>print   { TOKEN(KWprint); }
<INITIAL>read    { TOKEN(KWread); }
<INITIAL>return  { TOKEN(KWreturn); }

    /* Identifier */
<INITIAL>{letter}({letter}|{digit})* { TOKEN_STRING(id, yytext); }

    /* Scientific Notations*/
<INITIAL>{scientific} { TOKEN_STRING(scientific, yytext); }

    /* Constants */
<INITIAL>{integer}  { TOKEN_STRING(integer, yytext); }
<INITIAL>{octal}    { TOKEN_STRING(oct_integer, yytext); }
<INITIAL>{floating} { TOKEN_STRING(float, yytext); }
<INITIAL>"\""       { LIST; BEGIN STRING; if(opt_tok) printf("<string: ");}
<STRING>"\"\""      { LIST; if(opt_tok) printf("\"");}
<STRING>{ascii}     { LIST; if(opt_tok) printf("%s", yytext);}
<STRING>"\""        { LIST; BEGIN INITIAL; if(opt_tok) printf(">\n");}

\n {
    if (opt_src) {
        printf("%d: %s\n", line_num, buffer);
    }
    ++line_num;
    buffer[0] = '\0';
    buffer_ptr = buffer;
}

    /* Catch the character which is not accepted by rules above */
. {
    printf("Error at line %d: bad character \"%s\"\n", line_num, yytext);
    exit(-1);
}

%%
/* Code Section */
static void concatenateString(const char *yytext_ptr) {
    while (*yytext_ptr) {
        *buffer_ptr = *yytext_ptr;
        ++buffer_ptr;
        ++yytext_ptr;
    }
    *buffer_ptr = '\0';
}

int main(int argc, const char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./scanner [filename]\n");
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        fprintf(stderr, "Open file error\n");
        exit(-1);
    }

    yylex();

    return 0;
}
