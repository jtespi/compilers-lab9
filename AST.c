/*   Abstract syntax tree code

     This code is used to define an AST node, 
    routine for printing out the AST
    defining an enumerated type so we can figure out what we need to
    do with this.  The ENUM is basically going to be every non-terminal
    and terminal in our language.

    Shaun Cooper February 2015

*/

/* CS 370 Compilers
 * Lab 9 - Create NASM from AST
 * Jacob Espinoza
 * 2018 May 04
 */

#include "AST.h"
#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <string.h>
static int mydebug;

/* Enumerated types for the AST are defined in the header file.
 *  They are used to tell us what production rule we came across */

/* define a type AST node which will hold pointers to AST structs that will
   allow us to represent the parsed code 
*/
int GTEMPS=0;  /* Global Temp counter */

int GTEMPS2=0;

char * CreateTemp2()
{    char hold[100];
     char *s;
     sprintf(hold,"_STR%d",GTEMPS++);
     s=strdup(hold);
     return (s);
}

/* Creates a temporary label */
char * CreateTempLbl() {
     char hold[100];
     char * s;
     sprintf(hold, "_L%d", GTEMPS2++);
     s = strdup( hold );
     return (s);
}

/* uses malloc to create an ASTnode and passes back the heap address of the newly created node */
ASTnode * ASTCreateNode(enum ASTtype mytype)
{
    ASTnode *p;
    if (mydebug) fprintf(stderr,"Creating AST Node \n");
    p = (ASTnode *) malloc( sizeof(ASTnode) );
    p -> type = mytype;
    p -> next = NULL;
    p -> s1 = NULL;
    p -> s2 = NULL;
    p -> s3 = NULL;
    p -> strlabel = NULL;
    p -> name = NULL;
    p -> value = 0;
    return(p);
}

/* attach q to the next part of p */
void ASTAttachNext(ASTnode *p,ASTnode *q)
{
 /* ... missing */
    if ( p -> next != NULL )
        p -> next;
    else
        p -> next = q;
    
    printf("ASTAttachNext finished\n");
}

void PT(int level)
{ int i;
   for ( i = 0; i < level; i++) printf(" ");
}

/*  Print out the abstract syntax tree */
void ASTprint(int level,ASTnode *p)
{
   int i;
   if (p == NULL ) return;
   else
     { 
     for (i = 0;i < level; i++) printf(" "); /* print spacing blanks for indentation */
      
         /* **Start of large switch structure for printing different node types** */
       switch (p -> type) {
           /* Variable declarations can either be of type int or void and can be an array */
           case VARDEC :  printf("Variable declaration: ");
                     if ( p-> operator == INTTYPE )
                         printf("INT");
                     else if ( p-> operator == VOIDTYPE )
                         printf("VOID");
                     else 
                         printf("*unknown*");
                     printf(" %s",p -> name);
                     if ( p-> value > 0)
                        printf("[%d]",p -> value);
                     printf("\n");
                     break;
                     
        /* A function has a type, 0 or more perameters, and a compound statement */
        case FUNCTDEC : printf("Function: ");
                     if ( p -> operator == INTTYPE )
                         printf("INT %s", p->name );
                     if ( p -> operator == VOIDTYPE )
                         printf("VOID %s", p->name );
                     if ( p -> s1 == NULL ) {
                         printf("\n  ( VOID )");
                     }
                     else {
                         printf("\n  ( perams:\n");
                         ASTprint( level + 5, p -> s1 );
                         printf("  )\n");
                     }
                     printf("\n    {\n");
                     /* finally, print the compound statement */
                     ASTprint( level + 5, p -> s2 );
                     printf("    }\n");
                     break;
                     
        case COMPSTMT : printf("Compound statement:\n");
                    if ( p -> s1 != NULL ) {
                        // There are variable declarations to print
                        ASTprint( level, p -> s1 );
                    }
                    // else, don't print s1, there are no varDecs
                    // Next, check s2 for statement list
                    if ( p -> s2 == NULL ) {
                        /* print spacing blanks for indentation */
                        for (i = 0;i < level; i++) printf(" ");
                        printf("<statement list is empty>\n");                                                
                    }
                    /* else call ASTprint for the statement list */
                    else
                        ASTprint( level + 1, p -> s2);
                    
                    break;
                    
        case PARAM : printf("peram ");
                     if ( p -> operator == INTTYPE )
                        printf("INT %s", p->name);
                     if ( p -> operator == VOIDTYPE )
                         printf("VOID %s", p->name);
                     if ( p -> value == -1 )
                         printf("[ ] ");
                     printf("\n");
                     break;
                     
        /**** The following cases are for the 8 different types of statements ****/
        case EXPRSTMT : printf("expression statement\n");
                     if ( p -> s1 == NULL )
                         printf("<null expr. stmt.>");
                     /* else print the expression/simple expression */
                     else 
                         ASTprint( level + 2, p -> s1 );
                     printf("\n");
                     break;
        
        /* if and if-else statement */
        case SELECTSTMT : 
                    if( p -> s3 == NULL ) {
                        /* printf("if statement\n"); */
                        printf(" IF ( \n");
                        /* print expression */
                        ASTprint( level + 2, p -> s1 );
			
			for ( i = 0; i < level; i++) printf(" ");
                        printf(" ) { \n"); 
                        
                        /* print statement */
                        ASTprint( level + 4, p -> s2 );

			for( i = 0; i < level; i++ ) printf(" ");
                        printf(" } \n");
                    }
                    else {
                        /* printf("if-else statement\n"); */
                        printf(" IF ( \n");
                        /* print expression */
                        ASTprint( level + 2, p -> s1 );
                        
                        /* print spacing blanks for indentation */
                        for (i = 0;i < level; i++) printf(" ");
                        printf(" ) { \n"); 
                        
                        /* print statement */
                        ASTprint( level + 4, p -> s2 );
                        
                        /* print spacing blanks for indentation */
                        for (i = 0;i < level; i++) printf(" ");
                        printf(" } \n");
                        /* print spacing blanks for indentation */
                        for (i = 0;i < level; i++) printf(" ");
                        printf(" else { \n"); 
                        
                        /* print second statement */
                        ASTprint( level + 4, p -> s3);

			for ( i = 0; i < level; i++) printf(" ");
                        printf("} \n");
                    }
                    break;

        case ITERSTMT : 
                    printf(" WHILE ( \n");
                    ASTprint( level + 2, p -> s1 ); /* print expression */
                    
                    /* print spacing blanks for indentation */
                    for (i = 0;i < level; i++) printf(" ");
                    printf(" ) { \n");
                    ASTprint( level + 4, p -> s2); /* print statement */
                    
                    /* print spacing blanks for indentation */
                        for (i = 0;i < level; i++) printf(" ");
                    printf(" }\n");
                    
                    break;
                    
        case ASSNSTMT : printf("ASSIGNMENT statement\n");
                    /* print the identifier (variable) */
                    ASTprint( level + 2, p -> s1 );
                
                    /* print the expression statement */
                    ASTprint( level + 2, p -> s2 );
                    
                    break;
                    
        case RETURNSTMT : printf("RETURN statement\n");
                    if ( p -> s1 == NULL ) {
                        /* empty return statement */
                        /* print spacing blanks for indentation */
                        for (i = 0;i < level; i++) printf(" ");
                        printf("<empty>");
                    }
                    else  {
                        /* return statement with expression */
                        /* print the expression */
                        ASTprint( level + 2, p -> s1 );
                    }
                    printf("\n");
                    
                    break;
                    
                    
        case READSTMT : printf("READ statement\n");
                    /* print the identifier (variable) */
                    ASTprint( level + 2 , p -> s1 );
                    printf("\n");
                    
                    break;
                    
                    
                    
        case WRITESTMT : printf("WRITE statement\n");
                    /* print the expression */
                    ASTprint( level + 2 , p -> s1 );
                    printf("\n");
                    
                    break;
                    
                    
        case IDENTIFIER : printf("Identifier ");
                    printf("%s \n", p -> name);
                    if ( p -> s1 != NULL ) {
                        /* print spacing blanks for indentation */
                        for (i = 0;i < level; i++) printf(" ");
                        
                        /* the variable is an array */
                        printf("Array reference [ \n");
                        /* print the expression, the array size */
                        ASTprint( level + 2, p -> s1 );
                        
                        /* print spacing blanks for indentation */
                        for (i = 0;i < level; i++) printf(" "); 
                        printf(" ] end array\n");
                    }
                    
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
                    ASTprint( level + 2, p -> s1 );
                    
                    /* print the term to the right of the operator */
                    ASTprint( level + 2, p -> s2 );
        
                    break;
        
        case NUMBER: printf("NUMBER with value %d", p -> value);
		//printf(" (level = %d)", level);
                    printf("\n");
                    break;
                    
        case CALL : printf("function CALL\n");
                    /* print spacing blanks for indentation */
                    PT(level);
        
                    /* print the identifier */
                    printf("identifier %s\n", p -> name);

		            PT(level);
                    printf("(\n");
		

                    /* print the arguments */
                    if ( p -> s1 != NULL ) {
                        ASTprint( level +1 , p -> s1);
                    }

		             PT(level);
                    printf(" ) ");
                    printf("\n");
                    
                    break;
                    
        case ARGLIST: 
                    /* print the expression */
                    printf("ARGLIST\n");
                    ASTprint( level + 2, p -> s1);
                    printf("\n");
                    
                    break;
                     
        default: printf("*** Unknown type in ASTprint\n");


       } /* end switch */
       
       ASTprint( level , p -> next);
     }

} /* end function ASTprint */



/* dummy main program so I can compile for syntax error independently  
main()
{
}
/* */
