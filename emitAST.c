/* CS 370 Compilers
 * Lab 9 - Create NASM from AST
 * Jacob Espinoza
 * 2018 May 04
 */

#include "AST.h"
#include "symtable.h"
#include <stdio.h>
#include <ctype.h>
#include <malloc.h>

extern int debug; // debug flag from yacc file

/* ****
void emitASTmaster ( FILE * fp, ASTnode *p ) {
    /* Emit the header */ /*
    fprintf( fp, "%%include \"io64.inc\"");
    fprintf( fp, "\n");
    emitASTglobals( fp, p );
    fprintf( fp, "\nsection .data\t ;section for global strings\n");
    emitASTstrings( fp, p );
    fprintf( fp, "\nsection .text\n");
    fprintf( fp, "\tglobal main\n");
    
    /* After the header is set up, 
        emit code for any and all functions */ /*
    emitAST( fp, p ); /*
    
    fprintf( fp, "\n  ; end program");
    
} 
*/

void emitASTmaster ( ASTnode *p ) {
    /* Emit the header */
    printf( "%%include \"io64.inc\"");
    printf( "\n");
    emitASTglobals( p );
    printf( "\nsection .data\t ;section for global strings\n");
    emitASTstrings( p );
    printf( "\nsection .text\n");
    printf( "\tglobal main\n");
    
    /* After the header is set up, 
        emit code for any and all functions */
    //emitAST( fp, p);
    
    printf( "\n  ; end program");
    
}

void emitASTglobals( ASTnode * p ) {
  
    if ( p == NULL ) return;
    
    switch (p -> type) {
           /* Variable declarations */
           case VARDEC :  //printf("Variable declaration: ");
                     if ( p-> operator == INTDEC ) {
                         //printf("INT");
                         printf(" common\t %s\t", p->name);
                     }
                     else if ( p-> operator == VOIDDEC )
                         //printf("VOID");
                         ;
                     else 
                         printf("*unknown*");
                     //printf(" %s",p -> name);
                     if ( p-> value > 0) {
                        //printf("[%d]",p -> value);
                         printf("%d", (p->value)*8 );
                     }
                     else {
                         printf("%d", 8);
                     }
                     printf("\n");
                     break;
    } // end switch
    
    // move to the next node
    emitASTglobals( p->next );                     
} // end emitASTglobals



void emitASTstrings ( ASTnode * p ) {
    if ( p == NULL ) return;
    if ( (p -> type == WRITESTMT) && (p->strlabel != NULL) ) {
            /* Print the global strings */
            printf("%s: \t", p->strlabel); // print the label
            printf("db %s, 0", p->name); // print the string literal
            printf("    ;global string\n"); // print a comment
    } // end if
    // alternate abstract syntax tree navigation
    else if ( p->s1 != NULL) emitASTstrings( p->s1 );
    else if ( p->s2 != NULL) emitASTstrings( p->s2 );
    else if ( p->s3 != NULL) emitASTstrings( p->s3 );
    
    // move to the next node
    emitASTstrings( p -> next );
}


void emit_id ( ASTnode * p ) {
    // check if id is a global var
    if ( p->symbol->level == 0 ) {
        printf("MOV  RAX, %s\t;get identifier offset\n", p->name);
    } else {
        printf("MOV  RAX, %d\t;get identifier offset\n", p->symbol->offset);
        printf("ADD  RAX, RSP\t;Add SP to have direct ref to mem\n");
    }
    
}

void emit_read_num ( ASTnode * p ) {
    //printf("MOV RAX, 16\t;get Identifier offset");
	//printf("ADD RAX, RSP\t;Add the SP to have direct reference to memory ");
    emit_id( p );
	printf("GET_DEC 8, [RAX]\t; READ in an integer");
}



/*  Print out the abstract syntax tree */
void emitAST (FILE * fp, ASTnode * p)
{
   if (p == NULL ) return;
   else
     { 
         /* **Start of large switch structure for emitting code for the different node types** */
       switch (p -> type) {
           case READSTMT : if(debug)printf("READ statement\n");
                    /* print the identifier (variable) */
                    emitAST( fp, p -> s1 );
                    printf("\n");
                    
                    break;
                    
           case WRITESTMT : if(debug)printf("WRITE statement\n");
                    /* print the expression */
                    emitAST( fp, p -> s1 );
                    printf("\n");
                    
                    break;
                    
            case IDENTIFIER : if(debug)printf("Identifier ");
                    emit_id( p );
                    break;
                    
        
        /* print the expression */
        case EXPR : 
                    /* print the operator */
                    switch ( p -> operator ) {
                        case LESSEQ : printf("<=");
                            break;
                        case LESS : printf("<");
                            break;
                        case GREATER : printf(">");
                            break;
                        case GREATEREQ : printf(">=");
                            break;
                        case EQUAL : printf("=");
                            break;
                        case NOTEQ : printf("!=");
                            break;
                        case PLUS : printf("+");
                            break;
                        case MINUS : printf("-");
                            break;
                        case TIMES : printf("*");
                            break;
                        case DIVIDE : printf("/");
                            break;
                            
                        default : printf("<unknown op>");
                    } /* end switch for operator */
                    printf("\n");
                    
                    /* print the term to the left of the operator */
                    emitAST( fp, p -> s1 );
                    
                    /* print the term to the right of the operator */
                    emitAST( fp, p -> s2 );
        
                    break;
                     
        default: printf("*** Unknown type in emitAST\n");


       } /* end switch */
       
       emitAST( fp, p -> next);
     }

} /* end function emitAST */
