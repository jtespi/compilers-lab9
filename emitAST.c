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

void emitASTmaster ( FILE * fp, ASTnode *p ) {
    /* Emit the header every time */
    fprintf( fp, "%%include \"io64.inc\"");
    fprintf( fp, "\n");
    emitASTglobals( fp, p );
    fprintf( fp, "\nsection .data\t ;section for global strings\n");
    emitASTstrings( fp, p );
    fprintf( fp, "\nsection .text\n");
    fprintf( fp, "\tglobal main\n");
    
    /* After the header is set up, 
        emit code for any and all functions */
    emitAST( fp, p );
    
    fprintf( fp, "\n  ; end program\n");
    
}

void emitASTglobals( FILE * fp, ASTnode * p ) {
  
    if ( p == NULL ) return;
    
    switch (p -> type) {
           /* Variable declarations */
           case VARDEC :  //printf("Variable declaration: ");
                     if ( p-> operator == INTDEC ) {
                         //printf("INT");
                         fprintf(fp, " common\t %s\t", p->name);
                     }
                     else if ( p-> operator == VOIDDEC )
                         //printf("VOID");
                         ;
                     else 
                         fprintf(fp, "*unknown type in vardec*");
                     //printf(" %s",p -> name);
                     if ( p-> value > 0) {
                        //printf("[%d]",p -> value);
                         fprintf(fp, "%d", (p->value)*8 );
                     }
                     else {
                         fprintf(fp, "%d", 8);
                     }
                     fprintf(fp, "\n");
                     break;
    } // end switch
    
    // move to the next node
    emitASTglobals( fp, p->next );                     
} // end emitASTglobals


void emitASTstrings ( FILE * fp, ASTnode * p ) {
    if ( p == NULL ) return;
    if ( (p -> type == WRITESTMT) && (p->strlabel != NULL) ) {
            /* Print the global strings */
            fprintf(fp, "%s: \t", p->strlabel); // print the label
            fprintf(fp, "db %s, 0", p->name); // print the string literal
            fprintf(fp, "    ;global string\n"); // print a comment
    } // end if
    
    // alternate abstract syntax tree navigation
    else if ( p->s1 != NULL) emitASTstrings( fp, p->s1 );
    else if ( p->s2 != NULL) emitASTstrings( fp, p->s2 );
    else if ( p->s3 != NULL) emitASTstrings( fp, p->s3 );
    
    // move to the next node
    emitASTstrings( fp, p -> next );
}


void emit_id ( FILE * fp, ASTnode * p ) {
    // check if id is a global var
    if ( p->symbol->level == 0 ) {
        fprintf(fp, "\tMOV  RAX, %s\t;get identifier offset\n", p->name);
    } else {
        fprintf(fp, "\tMOV  RAX, %d\t;get identifier offset\n", (p->symbol->offset)*8);
        fprintf(fp, "\tADD  RAX, RSP\t;Add SP to have direct ref to mem\n");
    }
    
}

void emit_expr( FILE * fp, ASTnode * p) {
    // left hand side
    switch( p->s1->type) {
        case NUMBER:
            fprintf(fp, "\tMOV RAX, %d\n", p->s1->value);
            break;
        
        case IDENTIFIER: emit_id(fp, p->s1);
            fprintf(fp, "\tMOV RAX, [RAX]\n");
            break;
        
        case EXPR: emit_expr(fp, p->s1);
            fprintf(fp, "\tMOV RAX, [RSP+%d]\n", (p->s1->symbol->offset)*8);
            break;
            
        case CALL: //coming soon
            break;
        default: fprintf(fp, "\t;broken expr LHS\n");
        
    }
    
    // more coming soon....
}

/*  Print out the abstract syntax tree */
void emitAST (FILE * fp, ASTnode * p)
{
    if (p == NULL ) return;
    /* **Start of large switch structure for emitting code for the different node types** */
    switch (p -> type) {
        case VARDEC:
            //no action for vardec
            break;
            
        case FUNCTDEC: // first print the function's name
               fprintf(fp, "%s: \n", p->name);
               if ( strcmp( p->name, "main") == 0 ) // if the current function is main
                   fprintf(fp, "\tMOV RBP, RSP\t;move rsp into rbp for main ONLY!\n");
               // ***
               // set up each function
               fprintf(fp,"\tMOV R8, RSP\t;copy stack pointer to R8\n");
               fprintf(fp,"\tADD R8, -%d\t;adjust stack pointer for activation record\n", (p->value)*8);
               fprintf(fp,"\tMOV [R8], RBP\t;store the old BP\n");
               fprintf(fp,"\tMOV [R8+8], RSP\t;store old SP\n");
               
               fprintf(fp,"\tMOV RSP, R8\t;set the new stack pointer\n\n");
               // not needed: s1 is params emitAST(fp,p->s1);
               emitAST(fp,p->s2);
               
               //function teardown
               fprintf(fp,"\tMOV RBP, [RSP]\t;restore the old BP from memory\n");
               fprintf(fp,"\tMOV RSP, [RSP+8]\t;restore the old SP\n");
               if ( strcmp( p->name, "main") == 0 ) {
                   fprintf(fp,"\tMOV RSP, RBP\t;SP and BP need to be the same value, for main ONLY!\n");
               }
               fprintf(fp,"\tRET\n");
               
               break;
               
        case COMPSTMT:
               emitAST(fp,p->s1);
               emitAST(fp,p->s2);
               break;
               
        case READSTMT : if(debug)printf("READ statement\n");
                /* emit the identifier (variable), which moves it to RAX */
                emit_id( fp, p->s1 );
                fprintf(fp, "\tGET_DEC 8, [RAX]\t; READ in an integer");
                fprintf( fp, "\n");
                break;
                    
        case WRITESTMT : if(debug)printf("WRITE statement\n");
                /* print the string */
                if ( p -> s1 == NULL) {
                    fprintf( fp, "\tPRINT_STRING\t %s\t;print a string\n", p->strlabel);
                    fprintf( fp, "\tNEWLINE\n");
                }
                else {
                    
                    switch( p->s1->type) {
                        case NUMBER: fprintf(fp, "\tMOV rsi, %d\t;move the value of the number into rsi\n", p->s1->value);
                             break;
                             
                        case IDENTIFIER: emit_id(fp, p->s1); //emit the identifier
                             // the identifier is now stored in RAX
                             // move RAX to RSI
                             fprintf(fp, "\tMOV RSI, [RAX]\t;load immediate val from rax to rsi\n");
                             break;
                             
                        case EXPR: //emit_expr(fp, p->s1); // emit the expression
                            fprintf(fp,"\tMOV RSI, [RSP+%d]\t;rsp plus offset holds the value of the expr\n", (p->s1->symbol->offset)*8);
                            break;
                            
                        case CALL: //emit a function call, coming soon
                            break;
                            
                        default: fprintf(fp, "\t;ERROR: Unknown value in write statement!\n");
                    } // end switch
                    fprintf(fp, "\tPRINT_DEC 8 , rsi\t;standard write a value\n");
                    fprintf(fp, "\tNEWLINE\n");
                    //emitAST( fp, p -> s1);
                } // end else: s1 is not a string
                fprintf( fp, "\n");
                    
                break;
                    
        case IDENTIFIER : if(debug)printf("Identifier\n");
                // no action
                break;
                    
        
        /* print the expression */
        case EXPR : 
                    /* print the operator */
                    switch ( p -> operator ) {
                        case LESSEQ : if(debug)printf("operator <=");
                            break;
                        case LESS : if(debug)printf("operator <");
                            break;
                        case GREATER : if(debug)printf("operator >");
                            break;
                        case GREATEREQ : if(debug)printf("operator >=");
                            break;
                        case EQUAL : if(debug)printf("operator =");
                            break;
                        case NOTEQ : if(debug)printf("operator !=");
                            break;
                        case PLUS : if(debug)printf("operator +");
                            break;
                        case MINUS : if(debug)printf("operator -");
                            break;
                        case TIMES : if(debug)printf("operator *");
                            break;
                        case DIVIDE : if(debug)printf("operator /");
                            break;
                            
                        default : printf("<unknown operator>");
                    } /* end switch for operator */
                    printf("\n");
                    
                    /* emit the term to the left of the operator */
                    emitAST( fp, p -> s1 );
                    
                    /* emit the term to the right of the operator */
                    emitAST( fp, p -> s2 );
        
                    break; // break case expr
        
        case NUMBER: // no action
            break;
            
        case CALL: // no action
            break;
            
        case ARGLIST : // no action
            break;
            
        case ASSNSTMT: //no action
            break;
        
        default: printf("*** Unknown type '%d' in emitAST\n", p->type);
            break;


       } /* end large switch */
       
       // move to the next node in the AST
       emitAST( fp, p -> next);

} /* end function emitAST */
