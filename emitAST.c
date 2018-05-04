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
    fprintf( fp, ";global variables\n");
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
    emitASTstrings( fp, p->s1 );
    emitASTstrings( fp, p->s2 );
    emitASTstrings( fp, p->s3 );
    
    // move to the next node
    emitASTstrings( fp, p -> next );
}


void emit_id ( FILE * fp, ASTnode * p ) {
    
    fprintf(fp, "\n\t;Emitting an identifier...\n");
    
    // *** array functionality
    if ( p->s1 != NULL ) { // if s1 is not null, the ID is an array
    fprintf(fp, "\t;identifier is an array\n");
    switch ( p->s1->type ) {
        case NUMBER: 
            fprintf(fp, "\tMOV RBX, %d\t;load immediate into RBX\n", (p->s1->value)*8);
            break;
        
        case IDENTIFIER:
            emit_id(fp, p->s1);
            fprintf(fp, "\tMOV RBX, [RAX]\t;move into rbx after emit_id\n");
            fprintf(fp, "\tSHL RBX, 3\t;get the true array size, multiply by 8\n");
            break;
            
        case EXPR:
            emit_expr(fp, p->s1);
            fprintf(fp, "\tMOV RBX, [RSP + %d]\t;move from SP to RBX\n", (p->s1->symbol->offset)*8);
            fprintf(fp, "\tSHL RBX, 3\t;get the true array size, multiply by 8\n");
            break;
            
        case CALL: //call coming soon
            break;
        
        default: fprintf(fp, "\t;Error in array identifier\n");
        break;
    } // end switch for arrays
    
    } // end if s1 is not null
    
    // check if id is a global var
    if ( p->symbol->level == 0 ) {
        fprintf(fp, "\tMOV  RAX, %s\t;get identifier offset\n", p->name);
    } else {
        fprintf(fp, "\tMOV  RAX, %d\t;get identifier offset\n", (p->symbol->offset)*8);
        fprintf(fp, "\tADD  RAX, RSP\t;Add SP to have direct ref to mem\n");
    }
    
    // separate each identifier with a newline
    fprintf(fp, "\n");
    
}

void emit_expr( FILE * fp, ASTnode * p) {
    fprintf(fp, "\n\t;Emitting expression...\n");
    
    // left hand side
    switch( p->s1->type) {
        case NUMBER:
            fprintf(fp, "\tMOV RAX, %d\n", p->s1->value);
            break;
        
        case IDENTIFIER: 
            emit_id(fp, p->s1);
            fprintf(fp, "\tMOV RAX, [RAX]\n");
            break;
        
        case EXPR: 
            emit_expr(fp, p->s1); // a recursive call to emit expression
            fprintf(fp, "\tMOV RAX, [RSP + %d]\n", (p->s1->symbol->offset)*8);
            break;
            
        case CALL: //coming soon
            break;
        default: fprintf(fp, "\t;broken expr LHS!\n");
            break;    
    } // end first switch

    // move LHS value from rax to rsp with offset
    fprintf(fp, "\tMOV [RSP + %d], RAX\n", (p->symbol->offset)*8);

    // right hand side
    switch( p->s2->type ) {
        case NUMBER:
            fprintf(fp, "\tMOV RBX, %d\t;store immediate into rbx\n", p->s2->value);
            break;

        case IDENTIFIER:
            emit_id(fp, p->s2);
            fprintf(fp, "\tMOV RBX, [RAX]\t;move from rax to rbx\n");
            break;

        case EXPR:
            emit_expr( fp, p->s2 );
            fprintf(fp, "\tMOV RBX, [RSP + %d]\t;move value from rsp + offset into rbx\n", (p->s2->symbol->offset)*8);
            break;

        case CALL: //coming soon
            break;

        default: fprintf(fp, "\t;broken expr RHS!\n");

    } // end second switch
    
    // move the value from stack pointer into rax
    fprintf(fp, "\tMOV RAX, [RSP + %d]\n", (p->symbol->offset)*8);

   /**** After doing both LHS and RHS, evaluate any operators */
   fprintf(fp, "\t;Operator in expression\n");
   switch( p->operator) {
       case PLUS: fprintf(fp, "\tADD RAX, RBX\t;add operation\n");
          break;
       case MINUS: fprintf(fp, "\tSUB RAX, RBX\t;subtract operation\n");
          break;
       case TIMES: fprintf(fp, "\tIMUL RAX, RBX\t;multiplication operation\n");
          break;
       case DIVIDE: fprintf(fp, "\tXOR RDX, RDX\t;clear out rdx\n");
          fprintf(fp, "\tIDIV RBX\t;division operation\n");
          break;
       case EQUAL: fprintf(fp, "\tCMP RAX, RBX\t;equals operation\n");
          fprintf(fp, "\tSETE AL \t;set rax lower to equal??\n");
          fprintf(fp, "\tMOV RBX, 1\t;set rbx to 1 to filter the value in rax\n");
          fprintf(fp, "\tAND RAX, RBX\t;filter rax comparison\n");
          break;
       case NOTEQ: fprintf(fp, "\tCMP RAX, RBX\t;not equals operation compare\n");
          fprintf(fp, "\tSETNE AL \t;set rax lower to not equal\n");
          fprintf(fp, "\tMOV RBX, 1\t;rbx = 1 to filter rax\n");
          fprintf(fp, "\tAND RAX, RBX\t;filter rax comparison\n");
          break;
       case LESS: fprintf(fp, "\tCMP RAX, RBX ;less than operation compare\n");
          fprintf(fp, "\tSETL AL \t;set rax lower to lower\n");
          fprintf(fp, "\tMOV RBX, 1\t;set rbx to one for a filter\n");
          fprintf(fp, "\tAND RAX, RBX\t;filter RAX\n");
          break;
       case GREATER: fprintf(fp, "\tCMP RAX, RBX\t;greater than operation compare\n");
          fprintf(fp, "\tSETG AL \t;set rax lower to greater\n");
          fprintf(fp, "\tMOV RBX, 1\t;set rbx to one for a filter\n");
          fprintf(fp, "\tAND RAX, RBX\t;filter RAX\n");
          break;
       case LESSEQ: fprintf(fp, "\tCMP RAX, RBX\t;less than or equal to operation compare\n");
          fprintf(fp, "\tSETLE AL \t;set rax lower to less than or equal to\n");
          fprintf(fp, "\tMOV RBX, 1\t;set rbx to one for a filter\n");
          fprintf(fp, "\tAND RAX, RBX\t;filter RAX\n");
          break;

       case GREATEREQ: fprintf(fp, "\tCMP RAX, RBX\t;greater than or equal to operation compare\n");
          fprintf(fp, "\tSETGE AL \t;set rax lower to greater than or equal to\n");
          fprintf(fp, "\tMOV RBX, 1\t;set rbx to one for a filter\n");
          fprintf(fp, "\tAND RAX, RBX\t;filter RAX\n");
          break;

        default: fprintf(fp, "\t;unknown operator '%d'\n", p->operator);
    } // end operator switch
    
    // store the value from rax into the stack pointer + offset
    fprintf(fp, "\tMOV [RSP + %d], RAX\n", (p->symbol->offset)*8);
 
} // **end function emit expression

/* Emit NASM code following the abstract syntax tree */
void emitAST (FILE * fp, ASTnode * p)
{
    char * L1; //label 1
    char * L2; //label 2
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
                             
                        case EXPR: emit_expr(fp, p->s1); // emit the expression
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
                
        case EXPRSTMT : if(debug)printf("emitAST: ExprStmt\n");
                if( p->s1 != NULL ) {
                switch( p->s1->type ) {
                   
                    case NUMBER: fprintf(fp, "\tMOV RAX, %d\t;load immediate into rax\n", p->s1->value);
                        break;
                    case IDENTIFIER: emit_id(fp, p->s1);
                        fprintf(fp, "\tMOV RAX, [RAX]\t;move address of rax into rax\n");
                        break;
                    case EXPR: emit_expr(fp, p->s1);
                        fprintf(fp, "\tMOV RAX, [RSP + %d]\t;get value from rsp + offset and store in rax\n", (p->s1->symbol->offset)*8);
                        break;
                    default: fprintf(fp, "\t;invalid expression statement\n");
                } // end switch in expression statement
                fprintf(fp, "\tMOV [RSP + %d], RAX\t;move from rax into rsp + offset\n", (p->symbol->offset)*8);
                }
                break;
                
        case IDENTIFIER : if(debug)printf("Identifier\n");
                // no action
                break;
                    
        
        case EXPR: // EXPR statement already handled expressions
            break;        
        case NUMBER: // no action
            break;   
        case CALL: // no action
            break;
        case ARGLIST : // no action
            break;
            
        case ASSNSTMT: // call emitAST for s2 first
            fprintf(fp, "\n\t;Assignment statement\n");
            emitAST( fp, p->s2 );
            emit_id( fp, p->s1 );
            fprintf(fp, "\tMOV RBX, [RSP + %d]\t;move from RSP + offset into RBX\n", (p->s2->symbol->offset)*8);
            fprintf(fp, "\tMOV [RAX], RBX\t;move from rbx into rax\n");
            break;

        /* if-statement case */
        case SELECTSTMT:
            // create the temporary labels
            L1=CreateTempLbl();
            L2=CreateTempLbl();

            fprintf(fp, "\n%s:\t;Label for start of if statement\n", L1);

            switch(p->s1->type) {
               case NUMBER: fprintf(fp,"\tMOV RAX, %d;IF load immediate into rax\n", p->s1->value);
                break;
               
               case IDENTIFIER: emit_id(fp, p->s1);
                fprintf(fp, "\tMOV RAX, [RAX]\t;IF copy value into rax after emit_id\n");
                break;

               case CALL: fprintf(fp, "\t;IF call, coming soon...\n");
                break;   

              // most common case, expression
               case EXPR: emit_expr(fp, p->s1);
                fprintf(fp, "\tMOV RAX, [RSP + %d]\t;IF expr, move into rax from SP + offset\n", (p->s1->symbol->offset)*8);
                break;
               
               default: fprintf(fp,"\t;Error in SELECTSTMT (if)!\n");
            } // end switch for SELECTSTMT

            fprintf(fp, "\tCMP RAX, 0\t;IF comparison\n");
            fprintf(fp, "\tJE %s\t\t;IF condition, jump to else part\n", L2);

            emitAST(fp, p->s2); //emit statement 1 of the if statement

            fprintf(fp, "\n%s:\t;Label for start of else part\n", L2);

            emitAST(fp, p->s3); //emit statement 2 of the if statement

            break; // END case SELECTSTMT
            
        /* while-statement case */
        case ITERSTMT: // ITERSTMT is a while loop statement
            // create the temporary labels
            L1 = CreateTempLbl();
            L2 = CreateTempLbl();
            
            fprintf(fp, "\n%s:\t;Label - start of while statement\n", L1);
            
            switch ( p->s1->type ) {
                case NUMBER: fprintf(fp, "\tMOV RAX, %d\t;While stmt load immediate into rax\n", p->s1->value);
                  break;
                
                case IDENTIFIER: emit_id(fp, p->s1);
                  fprintf(fp, "\tMOV RAX, [RAX]\t;While stmt copy value into rax after emit_id\n");
                  break;
                  
                case CALL: fprintf(fp, "\t;While stmt call, coming soon...\n");
                  break;
                  
                case EXPR: emit_expr(fp, p->s1);
                  fprintf(fp, "\tMOV RAX, [RSP + %d]\t;While stmt expr, move into rax from SP + offset\n", (p->s1->symbol->offset)*8);
                  break;
                  
                default: fprintf(fp, "\tError in While statement!\n");
            } // end switch for ITERSTMT
            
            fprintf(fp, "\tCMP RAX, 0\t;WHILE comparison\n");
            fprintf(fp, "\tJE %s\t\t;WHILE condition, jump to end\n", L2);
            
            emitAST(fp, p->s2); //emit the body of the while loop (s2)
            
            fprintf(fp, "\tJMP %s\t\t;end of while, go back to check condition\n", L1);
            
            fprintf(fp, "\n%s:\t;Label - end of while statement\n", L2);
            
            break; // END case ITERSTMT
            

        default: printf("*** Unknown type (%d) in emitAST\n", p->type);
            fprintf(fp,"\t;*** Unknown type (%d) in emitAST\n", p->type);
            break;


       } /* end large switch */
       
       // move to the next node in the AST
       emitAST( fp, p -> next);

} /* end function emitAST */
