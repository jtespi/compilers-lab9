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

// emits the global variables
void emitASTglobals( FILE * fp, ASTnode * p ) {
  
    if ( p == NULL ) return;
    
    switch (p -> type) {
           /* Variable declarations */
           case VARDEC :
                     if ( p-> operator == INTDEC ) {
                       
                         fprintf(fp, " common\t %s\t", p->name);
                     }
                     else if ( p-> operator == VOIDDEC )
                         //printf("VOID");
                         ;
                     else 
                         fprintf(fp, "*unknown type in vardec*");
                  
                     if ( p-> value > 0) {
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


// emit all strings in the program and store in the data section.
// all strings are considered global in scope.
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
} // end emitASTstrings


// emit_id preconsition is that the node passed is an IDENTIFIER
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
            
        case CALL: emit_funct(fp, p->s1);
            fprintf(fp, "\tCALL %s\t;Call the function \'%s\'\n", p->s1->name, p->s1->name);
            fprintf(fp, "\tMOV RBX, RAX\t;after function call return, move the resuling value from rax\n");
            fprintf(fp, "\tSHL RBX, 3\t;get the true array size, multiply by 8\n");
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

    if ( p->s1 != NULL ) 
        fprintf(fp, "\tADD RAX, RBX\t;add array component into rax\n");
    
    // separate each identifier with a newline
    fprintf(fp, "\n");
    
} // end emit_id
// the id's value is now stored in RAX (postcondition)


// emit_expr precondition: the node passed is of type EXPR
void emit_expr( FILE * fp, ASTnode * p) {
    fprintf(fp, "\n\t;Emitting expression...\n");
    
    // emit the left hand side
    switch( p->s1->type) {
        case NUMBER:
            fprintf(fp, "\tMOV RAX, %d\t;load immediate into rax\n", p->s1->value);
            break;
        
        case IDENTIFIER: 
            emit_id(fp, p->s1);
            fprintf(fp, "\tMOV RAX, [RAX]\t;move address of rax into rax\n");
            break;
        
        case EXPR: 
            emit_expr(fp, p->s1); // a recursive call to emit expression
            fprintf(fp, "\tMOV RAX, [RSP + %d]\t;move value from rsp + offset into rax\n", (p->s1->symbol->offset)*8);
            break;
            
        case CALL: emit_funct(fp, p->s1);
            fprintf(fp, "\tCALL %s\t;Call the function \'%s\'\n", p->s1->name, p->s1->name);
            break;
        default: fprintf(fp, "\t;broken expr LHS!\n");
            break;    
    } // end first switch

    // move LHS value from rax to rsp with offset
    fprintf(fp, "\tMOV [RSP + %d], RAX\n", (p->symbol->offset)*8);

    // emit the right hand side
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

        case CALL: emit_funct(fp, p->s2);
            fprintf(fp, "\tCALL %s\t;Call the function \'%s\'\n", p->s1->name, p->s1->name);
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
          fprintf(fp, "\tSETE AL \t;set rax lower to equal\n");
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

// emit_function precondition: node passed is a CALL node
void emit_funct( FILE * fp, ASTnode * p) {
    if(debug)printf("In emit_funct\n");
    
    //emit the args first
    fprintf(fp, "\n\t;Emitting function call...\n");
    if ( p == NULL) return;
    
    // s1 of ARGLIST should point to an expression
    // emit the arguments
    emit_args(fp, p->s1);
    
    // p->symbol->fparams is the formal parameters
    // p->symbol->mySize is the function size
    // emit code for copying arguments to parameters
    emit_arg2param(fp, p->s1, p->symbol->fparams, p->symbol->mySize);
    
    // emit_arg2param will move to the next ARGLIST
} // ** end function emit_function

// emit_args precondition: node passed is an ARGLIST
void emit_args( FILE * fp, ASTnode * p ) {
    
    if(debug) printf("In emit_args\n");
    
    if ( p == NULL ) return;
    
    fprintf(fp, "\t; emitting an argument\n");
    
        switch( p->s1->type) {
            case NUMBER: fprintf(fp, "\tMOV RAX, %d\t;load immediate into rax\n", p->s1->value);
            break;

            case IDENTIFIER: emit_id(fp, p->s1);
            fprintf(fp, "\tMOV RAX, [RAX]\t;move address of rax into rax\n");
            break;

            case EXPR: emit_expr(fp, p->s1);
            fprintf(fp, "\tMOV RAX, [RSP + %d]\t;store value from rsp+offset into rax\n", (p->s1->symbol->offset)*8);
            break;
            
            case CALL: emit_funct(fp, p->s1);
            fprintf(fp, "\tCALL %s\t;Call the function \'%s\'\n", p->s1->name, p->s1->name);                    
            break;
            
            default: fprintf(fp, "\t;Invalid argument\n");
        }
        
        // move from rax to rsp + offset
        fprintf(fp, "\tMOV [RSP + %d], RAX\t;move rax into rsp + offset\n", (p->symbol->offset)*8);
        
        //call emit_args again
        emit_args(fp, p->next);
} // ** end function emit_args

// 4 values are needed to be passed to emit_arg2param
void emit_arg2param( FILE * fp, ASTnode * a, ASTnode * p, int fSize) {
    if(debug)printf("In emit_arg2param\n");
    
    // get the current stack pointer and move it into rbx
    fprintf(fp, "\tMOV RBX, RSP\t;copy the current SP into RBX\n");
    fprintf(fp, "\tSUB RBX, %d\t;subtract the function size from the SP\n", ((fSize+1)*8));
    
    while ( (p != NULL) && (a != NULL) ) {
        
        fprintf(fp, "\tMOV RAX, [RSP + %d]\t;move the argument RSP + offset into RAX\n", (a->symbol->offset)*8);
        fprintf(fp, "\tMOV [RBX + %d], RAX\t;move from rax into the param loc\n", (p->symbol->offset)*8);
        
        // move to the next node
        // a is ARGLIST
        // p is fparams
        a = a->next;
        p = p->next;
        
    } // end while
    
} // end emit_arg2param

// emit_return precondition is that the node passed in is a RETURNSTMT
void emit_return( FILE * fp, ASTnode * p ) {
    if(debug)fprintf(fp, "\t;in emit_return\n");
    if ( (p != NULL) && (p->s1 != NULL) ) {
        //switch on expression
        if(debug)printf("emit_return: about to enter switch\n");
        if(debug)fprintf(fp, "\t;emit_return: about to enter switch\n");
        switch( p->s1->type ) {
            case NUMBER: fprintf(fp, "\tMOV RAX, %d\t;load immediate into rax\n", p->s1->value);
            break;

            case IDENTIFIER: emit_id(fp, p->s1);
            fprintf(fp, "\tMOV RAX, [RAX]\t;move address of rax into rax\n");
            break;

            case EXPR: emit_expr(fp, p->s1);
            fprintf(fp, "\tMOV RAX, [RSP + %d]\t;store value from rsp+offset into rax\n", (p->s1->symbol->offset)*8);
            break;
            
            // the return is a recursive call to another function
            case CALL: emit_funct(fp, p->s1);
            fprintf(fp, "\tCALL %s\t;Call the function \'%s\'\n", p->s1->name, p->s1->name);                    
            break;

            default: fprintf(fp, "\t;invalid expression in return (type %d)!\n", p->s1->type);
            } // end switch
    } // end if not null
    /* after the switch is finished, the function return value will be stored in RAX*/
    
    //function teardown
    fprintf(fp, "\n\t;Function teardown...\n");
    fprintf(fp,"\tMOV RBP, [RSP]\t;restore the old BP from memory\n");
    fprintf(fp,"\tMOV RSP, [RSP+8]\t;restore the old SP\n");
    
    /* ** if the function is main, then it is the responsibility of the function which called emit_return to set the SP and BP to be the same */
    /* The emitting of RET is also done by the calling function */
    
} // ** end function emit_return

/* Emit NASM code following the abstract syntax tree */
/*  This is the main emitter code */
void emitAST (FILE * fp, ASTnode * p)
{
    char * L1; //label 1
    char * L2; //label 2
    char * L3; //label 3
    if (p == NULL ) return;
    /* **Start of large switch structure for emitting code for the different node types** */
    switch (p -> type) {
        case VARDEC:
            //no action for vardec
            break;
            
        case FUNCTDEC: // first print the function's name
               fprintf(fp, "%s: \t\t;Start of a function \n", p->name);
               if ( strcmp( p->name, "main") == 0 ) // if the current function is main
                   fprintf(fp, "\tMOV RBP, RSP\t;move rsp into rbp for main ONLY!\n");
               // ***
               // set up each function
               fprintf(fp,"\tMOV R8, RSP\t;copy stack pointer to R8\n");
               fprintf(fp,"\tADD R8, -%d\t;adjust stack pointer for activation record\n", (p->value)*8);
               fprintf(fp,"\tMOV [R8], RBP\t;store the old BP\n");
               fprintf(fp,"\tMOV [R8+8], RSP\t;store old SP\n");
               
               fprintf(fp,"\tMOV RSP, R8\t;set the new stack pointer\n\n");
    
               emitAST(fp,p->s2); // s2 is compound stmt               
               
               //function teardown
              fprintf(fp, "\n\t;Function teardown...\n");
              fprintf(fp,"\tMOV RBP, [RSP]\t;restore the old BP from memory\n");
              fprintf(fp,"\tMOV RSP, [RSP+8]\t;restore the old SP\n");
               if ( strcmp( p->name, "main") == 0 ) {
                   fprintf(fp,"\tMOV RSP, RBP\t;SP and BP need to be the same value, for main ONLY!\n");
               }
               fprintf(fp,"\tRET\n\n");
               
               break;
               
        case COMPSTMT: //call emitAST for s1 and s2
               emitAST(fp,p->s1);
               emitAST(fp,p->s2);
               break;
               
        case READSTMT : if(debug)printf("emitAST: Read statement\n");
                /* emit the identifier (variable), which moves it to RAX */
                emit_id( fp, p->s1 );
                fprintf(fp, "\tGET_DEC 8, [RAX]\t; READ in an integer");
                fprintf( fp, "\n");
                break;
                    
        case WRITESTMT : if(debug)printf("emitAST: Write statement\n");
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
                            
                        case CALL: emit_funct(fp, p->s1); //emit a function call
                            fprintf(fp, "\t;About to call the function, each param should be already set up in the new activation record\n");
                            fprintf(fp, "\tCALL %s\t\t;Call to function %s\n", p->s1->name, p->s1->name);
                            fprintf(fp, "\tMOV RSI, RAX\t;after function call return, move the resuling value from rax into rsi to be printed\n");
                            break;
                            
                        default: fprintf(fp, "\t;ERROR: Unknown value in write statement!\n");
                    } // end switch
                    fprintf(fp, "\tPRINT_DEC 8 , rsi\t;standard write a value\n");
                    fprintf(fp, "\tNEWLINE\n");
                 
                } // end else: s1 is not a string
                fprintf( fp, "\n");
                
                break;

                
        // RETURNSTMT s1 holds an expression
        case RETURNSTMT: if(debug)printf("emitAST: ReturnStmt\n");
             fprintf(fp, "\t;In return statement\n");
            // send emit_return a RETURNSTMT node
              emit_return(fp, p);
              fprintf(fp, "\tRET\n");
              break; //end case for return statement
                
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

                     case CALL: emit_funct(fp, p->s1); // a call node is sent to emit_funct
                       fprintf(fp, "\t;About to call the function, each param should be set up in the new activation record\n");
            
                       fprintf(fp, "\tCALL %s\t\t;Call to function %s\n", p->name, p->name);
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
        case ARGLIST : // no action
            break;
            
        case ASSNSTMT: if(debug) printf("emitAST: AssnStmt\n"); 
            // call emitAST for s2 first
            fprintf(fp, "\n\t;Assignment statement\n");
            emitAST( fp, p->s2 );
            emit_id( fp, p->s1 );
            fprintf(fp, "\tMOV RBX, [RSP + %d]\t;move from RSP + offset into RBX\n", (p->s2->symbol->offset)*8);
            fprintf(fp, "\tMOV [RAX], RBX\t;move from rbx into rax\n");
            break;

        /* if-statement case */
        case SELECTSTMT: if(debug) printf("emitAST: SelectStmt (IF)\n");
            // create the temporary labels
            L1=CreateTempLbl();
            L2=CreateTempLbl();
            L3=CreateTempLbl();

            fprintf(fp, "\n%s:\t;Label for start of if statement\n", L1);

            switch(p->s1->type) {
               case NUMBER: fprintf(fp,"\tMOV RAX, %d;IF load immediate into rax\n", p->s1->value);
                break;
               
               case IDENTIFIER: emit_id(fp, p->s1);
                fprintf(fp, "\tMOV RAX, [RAX]\t;IF copy value into rax after emit_id\n");
                break;

               case CALL: emit_funct(fp, p->s1); // a call node is sent to emit_funct
                 fprintf(fp, "\t;About to call the function, each param should be set up in the new activation record\n");
            
                 fprintf(fp, "\tCALL %s\t\t;Call to function %s\n", p->name, p->name);
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
            fprintf(fp, "\tJMP %s\t\t;jump over else (if present)\n", L3);

            fprintf(fp, "\n%s:\t;Label for start of else part\n", L2);

            emitAST(fp, p->s3); //emit statement 2 of the if statement
            
            fprintf(fp, "\n%s:\t;Label for after else part\n", L3);

            break; // END case SELECTSTMT
            
        /* while-statement case */
        // ITERSTMT is a while loop statement
        case ITERSTMT: if(debug) printf("emitAST: IterStmt (WHILE)\n");
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
                  
                case CALL: emit_funct(fp, p->s1); // a call node is sent to emit_funct
                  fprintf(fp, "\t;About to call the function, each param should be set up in the new activation record\n");
                  fprintf(fp, "\tCALL %s\t\t;Call to function %s\n", p->name, p->name);
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


        /* Function call case */
        case CALL : if(debug) printf("emitAST: CALL\n");
            emit_funct(fp, p); // a call node is sent to emit_funct
  
            fprintf(fp, "\t;About to call the function, each param should be set up in the new activation record\n");
            
            fprintf(fp, "\tCALL %s\t\t;Call to function %s\n", p->name, p->name);

            break;

        default: printf("*** Unknown type (%d) in emitAST\n", p->type);
            fprintf(fp,"\t;*** Unknown type (%d) in emitAST\n", p->type);
            break;


       } /* end large switch */
       
       // move to the next node in the AST
       emitAST( fp, p -> next);

} /* end function emitAST */
