#ifndef AST_H
#define AST_H
#include <stdio.h>
#include <malloc.h>
#include <ctype.h>

enum ASTtype {
   VARDEC,
   INTTYPE, /**/
   VOIDTYPE, /**/
   IDENTIFIER,
   FUNCTDEC,
   PARAMS, /**/
   PARAM, /**/
   COMPSTMT,
   LOCALDEC,
   STMTLIST,
   EXPRSTMT,
   SELECTSTMT,
   ITERSTMT,
   ASSNSTMT,
   RETURNSTMT,
   READSTMT,
   WRITESTMT,
   EXPR,
   NUMBER,
   TERM,
   ARGLIST,
   CALL
};
enum OPERATORS {
   PLUS,
   MINUS,
   TIMES,
   DIVIDE,
   LESSEQ,
   GREATEREQ,
   GREATER,
   LESS,
   EQUAL,
   NOTEQ,

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

char * CreateTemp2();

FILE * fp;

ASTnode *ASTCreateNode(enum ASTtype mytype);
void ASTAttachNext(ASTnode *p,ASTnode *q);
void ASTprint(int level, ASTnode *p);

//----------------------------------------------------------

void emitASTmaster ( FILE * fp, ASTnode *p );
void emitAST( FILE * fp, ASTnode * p );
void emitASTstrings ( FILE * fp, ASTnode * p );
void emitASTglobals( FILE * fp, ASTnode * p );

ASTnode * program;

#endif
