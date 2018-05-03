#ifndef AST_H
#define AST_H
#include <stdio.h>
#include <malloc.h>
#include <ctype.h>

enum ASTtype {
   VARDEC, /*value = 0*/
   INTTYPE,
   VOIDTYPE,
   IDENTIFIER,
   FUNCTDEC,
   PARAMS,
   PARAM,
   COMPSTMT,
   LOCALDEC,
   STMTLIST,
   EXPRSTMT,
   SELECTSTMT, /*AKA if-statement, value = 10*/
   ITERSTMT, /*AKA while-statement, value = 11*/
   ASSNSTMT,
   RETURNSTMT,
   READSTMT,
   WRITESTMT,
   EXPR,
   NUMBER,
   TERM,
   ARGLIST,
   CALL  /*value = 20*/
};
enum OPERATORS {
   PLUS, /*value = 0*/
   MINUS,
   TIMES,
   DIVIDE,
   LESSEQ,
   GREATEREQ,
   GREATER,
   LESS,
   EQUAL,
   NOTEQ, /*value = 9*/

   INTDEC,
   VOIDDEC
};

typedef struct ASTnodetype
{
     enum ASTtype type;
     enum OPERATORS operator;
     enum OPERATORS isType; /* either INTDEC or VOIDDEC */
     char * name;
     int value;
     struct ASTnodetype *next; /* next is the connector for other statements */
     struct ASTnodetype *s1,*s2, *s3 ; /* used for holding IF and WHILE components */
     struct SymbTab * symbol;
     char * strlabel;
} ASTnode;

ASTnode *ASTCreateNode(enum ASTtype mytype);
void ASTAttachNext(ASTnode *p,ASTnode *q);
void ASTprint(int level, ASTnode *p);

//-------- Functions & data contained in emitAST.c ---------------------------

FILE * fp; //file pointer object
char * L1; //label 1
char * L2; //label 2

char * CreateTemp2();
char * CreateTempLbl();

void emit_id ( FILE * fp, ASTnode *p );
void emit_expr( FILE * fp, ASTnode *p);

void emitASTmaster ( FILE * fp, ASTnode *p );
void emitAST( FILE * fp, ASTnode * p );
void emitASTstrings ( FILE * fp, ASTnode * p );
void emitASTglobals( FILE * fp, ASTnode * p );

ASTnode * program;

#endif
