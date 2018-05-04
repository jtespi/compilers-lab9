%{
/*****
* Lab 9 - Create NASM code from AST
* CS-370 Compilers
* Jacob Espinoza
* 2018 May 04
*****/

/* begin specs */
/* Include symbol table before AST */
#include "symtable.h"
#include "AST.h"

#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

extern int lineN;
int debug = 0; /* also used in the lex file */

int level = 0; /* Used for ID names and calls */
int offset = 0; /* analogous to a memory index */

/* global offset to be saved before a compound statement and restored after*/
int gOffset = 0; 

int maxOffset;

void yyerror (s)  /* Called by yyparse on error */
     char *s;
{
  fprintf (stderr, "***ISSUE: %s, input line number %d\n", s, lineN);
} /* Print the line number after the error message */

%}
/*  defines the start symbol, what values come back from LEX and how the operators are associated  */

%start program

%union {
	int value;
	char * string;
	ASTnode * node;
	enum OPERATORS operator;
}
/* Some tokens have extra attributes attached to them:
     NUM has an integer value attached to it
     ID, INT, and VOID have string values attached to each */
%token <value> NUM
%token <string> ID
%token <string> INT
%token <string> VOID
%token <string> STRING

/* The rest of the tokens are just regular tokens */
%token IF ELSE WHILE RETURN READ WRITE FOR LE GE EQ NE L G
%nonassoc IFX
%nonassoc ELSE

%type <node> program decList dec varDec functDec localDec stmtList statement expr simExpr addExpr
%type <node> exprStmt compStmt selectStmt iterStmt assnStmt returnStmt readStmt writeStmt
%type <node> term args argList param params paramList factor var call
%type <operator> typeSpec relop addOp multOp

%%	/* end specs, begin rules */

program : decList { program = $1; }
		;

/* declaration list can be one or more declarations */
decList	: dec  { $$ = $1; }
		| dec decList  { $1 -> next = $2; 
                         $$ = $1; }
		;
		
/* a declaration can be a variable declaration or a function declaration */
dec     : varDec { $$ = $1; }
        | functDec { $$ = $1; }
        ;

/* variable declaration is a type spec with an ID and optional "[ NUM ]" followed by a semicolon 
   Allows for regular and array variables to be declared. */
varDec : typeSpec ID ';'
        { if (debug) printf("Found a variable \"%s\".\n", $2);
        
          if (debug) Display();

 /* If not already definded at this level, insert into symbol table. */
          if ( Search($2, level + 1, 0 ) == NULL ) {
            $$ = ASTCreateNode( VARDEC ); 
            $$ -> name = $2;
            $$ -> operator = $1;
            $$ -> isType = $1;
            $$ -> symbol = Insert( $2, $1, 0, level, 1, offset, NULL);
            if (debug) Display();
            $$ -> value = 0; /* not an array */
            offset += 1;
          }
          else {
            yyerror($2);
            yyerror("Symbol already exists!");
            exit(1);
            }
        }
        
        | typeSpec ID '[' NUM ']' ';' 
        { if (debug) printf( "Found an array \"%s\", size %d.\n", $2, $4); 
        
         /* Search the symbol table for the ID, if it is not present, insert into the table. */
         
          if( Search($2, level, 0) == NULL ) {
            $$ = ASTCreateNode( VARDEC );
            $$ -> name = $2;
            $$ -> value = $4;
            $$ -> operator = $1;
            $$ -> isType = $1;
            $$ -> symbol = Insert( $2, $1, 2, level, $4, offset,NULL);
         /* The offset is incremented by the size of the array */
            offset += $4;
          }
          else {
            yyerror($2);
            yyerror("Symbol already exists!");
            exit(1);
            }
        }
        ;

/* A type specifier is of the type int or void */
typeSpec : INT { $$ = INTDEC; }
        | VOID { $$ = VOIDDEC; }
        ;
        

/* a functional declaration is a type specifier followed by an ID,
   left parenthesis, parameters, right parenthesis and a compound statement */
functDec : typeSpec ID '('

        /* Before coming across the parameters, check the symbol table */
        {  if( Search( $2, level, 0) == NULL ) {
             gOffset=offset;
             offset=2;
             maxOffset = offset;
             Insert( $2, $1, 1, level, 1, offset, NULL );
             if(debug) printf("function declaration inserted\n");
           } else {
              yyerror($2);
              yyerror("Name already exists for function declaration");
              exit(1);
              }
        
         }
         
         params 
         {  /* set fparams in the symbol table entry for ID ($2) to params ($5) */
              /* fparams stands for formal parameters */
            Search( $2, 0, 0 ) -> fparams = $5;
         }

        ')' compStmt
        
         /* After compound statement is parsed, create the AST node for Funct. Dec. */
        { if(debug) printf("Found a function declaration\n"); 
          $$ = ASTCreateNode( FUNCTDEC );
          $$ -> name = $2;
          $$ -> operator = $1; /* typeSpec */
          $$ -> isType = $1;
          $$ -> s1 = $5; /*  params  */
          $$ -> s2 = $8; /* compStmt */
          $$ -> symbol = Search($2, 0, 0); /* Get the symbol table entry */
          $$ -> value = maxOffset;
          $$ -> symbol -> mySize = maxOffset;
          
          offset -= Delete(1); /* Remove all symbols from what was put into function call */
          level = 0; /* reset the level back to 0 */
          offset = gOffset; /* Restore the global offset */
        }
        ;
        
/* parameters are either void or a parameter list */
params  : VOID { $$ = NULL; }
        | paramList { $$ = $1; }
        ;
      
/* a parameter list is one or more parameters (separated by commas) */         
paramList : param
        { $$ = $1;
          if (debug) printf("  found a parameter\n"); 
          }
        | param ',' paramList
        { $$ = $1;
          $1 -> next = $3;
        }
        ;
        
/* a parameter is similar to a variable declaration:
    either a type-Spec and ID or type-Spec ID and a pair of square brackets 
    no semicolon is required and the array size need not be defined. */

/* Check the symbol table and insert in a similar manner to vardec functions. */
        
param   : typeSpec ID
        { if(debug) printf(" Found a parameter \"%s\".\n", $2 );
          if (debug) Display();
          if ( Search($2, level + 1, 0) == NULL ) {
            $$ = ASTCreateNode( PARAM ); 
            $$ -> name = $2;
            $$ -> operator = $1; 
            $$ -> isType = $1;
            $$ -> symbol = Insert($2, $1, 0, level + 1, 1, offset,NULL);
            $$ -> value = 0;  /* not an array parameter */
            if (debug) Display();
            offset += 1;
          } else {
             yyerror($2);
             yyerror("Symbol already exists");
             exit(1);
          }
        }
        
	/* Array parameter of unspecified size */
        | typeSpec ID '[' ']'
        { if (debug) printf(" Found an array parameter \"%s\".\n", $2 );
          if (debug) Display();
          if ( Search($2, level + 1, 0) == NULL ) {
            $$ = ASTCreateNode( PARAM ); 
            $$ -> name = $2;
            $$ -> operator = $1;
            $$ -> isType = $1;
            $$ -> symbol = Insert($2, $1, 2, level + 1, 1, offset,NULL);
            $$ -> value = -1; /* value -1 signifies an array with no specified size */
            offset += 1;
          } else {
             yyerror($2);
             yyerror("Symbol already exists!");
             exit (1);
             }
        }
        ;

/* a compound statement is enclosed by curly braces and consists of local declarations
    and a statement list. */          
compStmt : '{' 
	 /* Increment the level and print it if debugging is turned on */
         { level++;
	          if (debug) printf("Block level is %d\n", level);
	      }

           localDec stmtList '}'
	
        { $$ = ASTCreateNode( COMPSTMT );
          $$->s1 = $3; /* local declarations */
          $$->s2 = $4; /* statement list */
	    
           if(debug)Display();
           if ( offset > maxOffset ) maxOffset = offset;
           offset -= Delete( level );
           level--;
        }
        ; /* End compound statement */

    
/* local declarations consist of 0 or more variable declarations */ 
localDec : /* can be empty */ { $$ = NULL; } 
        | varDec localDec 
        { $1 -> next = $2; 
          $$ = $1;
          }
        ;
       
/* a statement list consists of 0 or more statements */         
stmtList : { $$ = NULL; } /* can be empty */ 
        | statement stmtList
        { $1 -> next = $2;
          $$ = $1;
          }
        ;
        
/* a statement can be any of 8 different types of statements*/        
statement : exprStmt { $$ = $1; }
        | compStmt { $$ = $1; }
        | selectStmt { $$ = $1; }
        | iterStmt { $$ = $1; }
        | assnStmt { $$ = $1; }
        | returnStmt { $$ = $1; }
        | readStmt { $$ = $1; }
        | writeStmt { $$ = $1; }
        ;
        
/* an expression statement is either an expression with a semicolon or just a semicolon */         
exprStmt    : expr ';' 
            { $$ = ASTCreateNode( EXPRSTMT );
              $$ -> s1 = $1;
              $$->isType = $1->isType;  // below, added 3 lines
              $$->name = CreateTemp();
              $$->symbol = Insert( $$->name, $$->isType, 0, level, 1, offset, NULL);
              offset++;}
              /* have s1 point to the expression */
            | ';' 
            { $$ = ASTCreateNode( EXPRSTMT );
              $$ -> s1 = NULL; }
              /* s1 points to null, there is no expression */
            ;
            
/* a selection statement is an IF with an optional ELSE */         
selectStmt : IF  '(' expr ')' statement %prec IFX
            { $$ = ASTCreateNode ( SELECTSTMT );
              $$ -> s1 = $3; /* expression */
              $$ -> s2 = $5; /* statement */
              $$ -> s3 = NULL;
              if(debug)printf("--found an if statement\n"); 
              }
            | IF  '(' expr ')' statement ELSE statement 
            { $$ = ASTCreateNode( SELECTSTMT );
              $$ -> s1 = $3; /* expression */
              $$ -> s2 = $5; /* (first) statement */
              $$ -> s3 = $7; /* (second, else) statement */
              if(debug)printf("--found an if/else statement\n"); }
            ;
            
/* an iteration statement is used for a while loop */             
iterStmt : WHILE '(' expr ')' statement
        { $$ = ASTCreateNode( ITERSTMT );
          $$ -> s1 = $3; /* expression */
          $$ -> s2 = $5; /* statement */
          if(debug)printf("--found a while loop\n"); 
          }
        ;

/* an assignment statement sets a variable to an expression statement */
/* **Type-checking was introduced in Lab 7 */
assnStmt : var '=' exprStmt
        { $$ = ASTCreateNode( ASSNSTMT );
	  /* Check to see if the types match */
     
	   if ( ($1 -> isType == VOIDDEC) || ($1 -> isType != $3 -> isType))
	     { yyerror("Type mismatch in assignment statement!");
	       exit(1);
	     }

          $$ -> s1 = $1;
          $$ -> s2 = $3;

	  /* Temporarily store the RHS in case it is modified */
          $$ -> name = CreateTemp();
          $$ -> symbol = Insert( $$->name, INTDEC, 0, level, 1, offset, NULL);
          offset++;
          }
        ;

/* a return statement can either be empty or with an expression 
    'empty' return statements must have RETURN and ; */ 
returnStmt : RETURN ';'
            { $$ = ASTCreateNode( RETURNSTMT );
              $$ -> s1 = NULL;
              if (debug) printf("Empty return statement\n");
              }
            | RETURN expr ';'
            { $$ = ASTCreateNode( RETURNSTMT );
              $$ -> s1 = $2;            
              if (debug) printf("Return statement with expression\n"); 
              }
            ;

/* a read statement has the token READ along with a variable and a semicolon */ 
readStmt : READ var ';'
        { $$ = ASTCreateNode( READSTMT );
          $$ -> s1 = $2;
         if(debug)printf("Read statement\n"); 
         }
        ;
        
/* a write statement has the token WRITE and an expression with a semicolon */
writeStmt : WRITE expr ';'
        { $$ = ASTCreateNode( WRITESTMT );
          $$ -> s1 = $2;
          if (debug) printf("Write statement\n"); 
          }
          
          | WRITE STRING  ';'
        { $$ = ASTCreateNode( WRITESTMT );
          /* Set name to the string value */
          $$ -> name = $2;
          /* String label is created using the CreateTemp2 function in AST.c */
          $$ -> strlabel = CreateTemp2();
          if (debug) printf("Write statement - string\n");
          }
        ;
        
/* an expression consists of a simple expression */ 
expr : simExpr { $$ = $1; }
    ;

/* a variable is either a variable name 
  or an array variable name (with size) */
/* Check the symbol table to make sure the ID was previously defined. */
var : ID { 

	  /* include a reference/pointer to the symbol table structure */
	  struct SymbTab * p;
	  p= Search($1, level, 1);
	  
	  if (p==NULL) {
	     yyerror($1);
		yyerror("Not found");
		exit(1);
	  }
	  
	   if ( p -> isAFunct != 0)
	     {
	         yyerror($1);
		    yyerror("Not a Scalar");
            exit(1);
	     }

	    $$ = ASTCreateNode( IDENTIFIER );
	    $$ -> name = $1; /* Might need to be pointer to symbol table */
	    $$ -> isType = p -> Type; /* get the type from the symbol table */
	    $$ -> symbol = p;

	    }
          


    | ID '[' expr ']' { 

	  /* include a reference/pointer to the symbol table structure */
	  struct SymbTab * p;
	  p= Search($1, level, 1);
	  
	  if (p==NULL) {
	     yyerror($1);
		yyerror("Not found");
		exit(1);
	  }
	  
	   if ( p -> isAFunct != 2)
	     {
	         yyerror($1);
		    yyerror("Not an array");
            exit(1);
	     }

	    $$ = ASTCreateNode( IDENTIFIER );
	    $$ -> name = $1; /* Might need to be pointer to symbol table */
	    $$ -> isType = p -> Type; /* get the type from the symbol table */
	    $$ -> symbol = p;
    }
	   
    ;

/* a simple expression is one additive expression
    or one add expr., a relation op, and an add expr.*/ 
simExpr   : addExpr { $$ = $1; }
        | simExpr relop addExpr {
          $$ = ASTCreateNode( EXPR );
          $$ -> operator = $2;
	      $$ -> isType = $1->isType;
          $$ -> s1 = $1;
          $$ -> s2 = $3;
          $$ -> name = CreateTemp(); /* Name is a temporary variable */
	      $$ -> symbol = Insert( $$->name, INTDEC, 0, level, 1, offset, NULL);
	      offset++;
        }
        ;
        
/* a relation operator can be either a token from LEX or single characters '>' and '<' */         
relop   : LE
        { $$ = LESSEQ;
          if(debug) printf("Less than or equal to OP found\n"); }
        | L
        { $$ = LESS;
          if(debug) printf("Less than OP found\n"); }
        | G
        { $$ = GREATER;
          if(debug) printf("Greater than OP found\n"); }
        | GE
        { $$ = GREATEREQ;
          if(debug) printf("Greater than or equal to OP found\n"); }
        | EQ
        { $$ = EQUAL;
          if(debug) printf("Equal to OP found\n"); }
        | NE
        { $$ = NOTEQ;
          if(debug) printf("Not equal to OP found\n"); }
        ;

/* an additive expression is a term followed by 0 or more sets of { addOp term }*/ 
/* Check the types before creating the node */
addExpr : term { $$ = $1; }
        | addExpr addOp term {
	     if ( $1 -> isType != $3 -> isType ) {
	        yyerror("Type mismatch in additive expression!");
	        exit(1);
	     }

          $$ = ASTCreateNode( EXPR );
          $$ -> operator = $2;
          $$ -> s1 = $1;
          $$ -> s2 = $3;
	      $$ -> isType = $1->isType;
	      $$ -> name = CreateTemp(); /* Name is a temporary variable */
	      $$ -> symbol = Insert( $$->name, INTDEC, 0, level, 1, offset, NULL);
	      offset++;
          }
        ;
        
/* an addition operator is either character '+' (plus) or '-' (minus) */ 
addOp   : '+' { $$ = PLUS; }
        | '-' { $$ = MINUS; }
        ;
       
/* a term is a factor followed by 0 or more sets of {multiplicative operator and factor} */         
term    : factor { $$ = $1; }
        | term multOp factor {
          if ( $1 -> isType != $3 -> isType ) {
             yyerror("Type mismatch in term.");
             exit(1);
          }
          $$ = ASTCreateNode( EXPR );
          $$ -> operator = $2;
          $$ -> s1 = $1;
          $$ -> s2 = $3;
          $$ -> isType = $1->isType;
          $$ -> name = CreateTemp(); /* Name is a temporary variable */
          $$ -> symbol = Insert( $$->name, INTDEC, 0, level, 1, offset, NULL);
          offset++;
          }
        ;

/* a multiplicative operator is either character '*' (multiply) or '/' (divide) */ 
multOp  : '*' { $$ = TIMES; }
        | '/' { $$ = DIVIDE; }
        ;
        
/* a factor is either an expression enclosed by parentheses, a NUM token,
     a variable, or a call */ 
factor  : '(' expr ')' { $$ = $2; } 
        | NUM { $$ = ASTCreateNode( NUMBER );
                $$ -> isType = INTDEC;
                $$ -> value = $1; }
        | var { $$ = $1; }
        | call { $$ = $1; }
        ; 

/* a call is an ID followed by arguments which are enclosed by parentheses*/

/* Check the formal parameters of the function call*/         
call    : ID '(' args ')'
        { /* include a reference/pointer to the symbol table structure */
	      struct SymbTab * p;
	      
	      /* If not already defined at this level, insert into symbol table. */
          if ( (p = Search($1, 0, 0 )) != NULL ) {
	      
              if ( CompareFormals( p-> fparams, $3 ) != 1 ) {
                  yyerror( $1 );
                  yyerror( "Parameter type or length mismatch between formals and argument list" );
                  exit( 1 );
               }
        
              $$ = ASTCreateNode( CALL );
              $$ -> s1 = $3;
              $$ -> isType = p -> Type;
              $$ -> name = $1;
              $$ -> symbol = p;
              }
              else {
              /* The symbol was not found */
              yyerror($1);
              yyerror("Function name was not defined");
              exit(1);
              }
          }
        ;

/* arguments are an argument list or empty */ 
args    : { $$ = NULL; }/* empty */ 
        | argList { $$ = $1; }
        ;
        
/* an argument list is an expression followed by 0 or more sets of { comma and expression } */        
argList : expr { 
          $$ = ASTCreateNode( ARGLIST );
          $$ -> isType = $1 -> isType;
          $$ -> s1 = $1; 
          }

        | argList ',' expr {
          $$ = ASTCreateNode( ARGLIST );
          $$ -> isType = $3 -> isType;
          $$ -> next = $1;
          $$ -> s1 = $3;
          }
        ;
        

%%	/* end of rules, start of program */

main(int argc, char * argv[] )
{ 
  bool debug_flag = false;
  bool output_flag = false;
  bool custom_output = false;
  char * output_name;
  int count = 1;
  
  while( count < argc ) {
     if( strcmp( argv[count], "-d") == 0 ) {
        debug_flag = true;
        printf(" debugging enabled\n");
        debug = 1;
     }
     if( strcmp( argv[count], "-o") == 0 ) {
        output_flag = true;
        printf(" output set\n");
     }
     if ( (output_flag == true) && (count+1 < argc) && (strcmp(argv[count+1],"-d") != 0) ) {
         output_name = strcat( strdup( argv[ count+1 ]), ".asm");
         custom_output = true;
     }
     count++;
  }
  if ((custom_output == false) && (output_flag == true)) output_name = "output.asm";
  
  if (output_flag == true) {
    printf(" set to output to file %s\n", output_name);
    fp = fopen( output_name, "w");
  } else 
    {
     fp = stdout;
    }
  
  /* Next call yyparse */
  yyparse();
  
  printf("---- Finished parsing input/file ---\n");
  
  emitASTmaster( fp, program );
  
  fclose( fp );
  
  printf("Done!\n");
}
