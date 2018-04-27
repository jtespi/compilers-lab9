/* LEX routine for Lab 9
*****
* Lab 9 - Create NASM code from AST
* CS-370 Compilers
* Jacob Espinoza
* 2018 May 04
**** */

%{ 

#include "AST.h"
#include <stdio.h>
#include <ctype.h>
#include <malloc.h> 

/* external debug, set in the yacc file */
extern int debug;

/* Line number counter */
int lineN = 1;

#include "y.tab.h"
%}

string \".*\"

/* Any other characters found are simply printed on the screen */

%%
 /*** Found the reserved words "int" or "void", 
   also has a yylval to be able to print the name of the token */
int  { yylval.string = "INT" ; return(INT); }
void { yylval.string = "VOID" ; return(VOID);}

 /* other reserved words, just return a token  */
if { return(IF);}
else {return(ELSE);}
while {return(WHILE);}
read { return(READ); }
return { return(RETURN);}
write { return(WRITE); }
for { return(FOR); }

 /* two character tokens for relational operators */
[<][=] { return(LE); }
[>][=] { return(GE); }
[=][=] { return(EQ); }
[!][=] { return(NE); }

 /* single character relational ops */
[<] { return(L); }
[>] { return(G); }

 /* Accepts variables of any length */
 /* Returns a string */
[a-zA-Z][a-zA-Z0-9]*    { if (debug) fprintf(stderr,"ID found\n"); 
                       yylval.string = strdup( yytext ); return(ID);}
                       
 /* Found a number (integer) */                      
[0-9][0-9]*	{ if(debug) fprintf(stderr,"NUM found, "); 
                yylval.value = atoi((const char *)yytext);
                if(debug) fprintf(stderr,"value is %d\n", yylval ); 
                return(NUM);}
                
                
 /* Ignore regular white spaces (other than new line) */                      
[ \t]		{ }

 /* Return a token for each 'special' character */
[=\-+*/(),;\[\]{}] 	{ if (debug) fprintf(stderr,"return token \'%c\'\n",*yytext); 
                       return (*yytext);}

 /* Count every new line */
\n 		{ lineN++; // increment the new line counter
            if (debug) printf("carriage return, line #%d\n\n", lineN );
        }
        
{string}      { if(debug) fprintf( stderr,"String found\n");
              yylval.string = strdup( yytext ); return( STRING ); }
%%

int yywrap(void)
{ return 1;}
