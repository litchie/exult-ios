%{
#include <stdio.h>
#include "ucclex.c"
%}

%token VOID UVAR OBRACE CBRACE OBRACKET CBRACKET COMMA ID IF THEN ELSE 
%token EQUALS PLUS MINUS
%%
function_decl:	type ID OBRACKET parameter_list CBRACKET
		OBRACE CBRACE;

type:		VOID
|		UVAR
;

parameter_list:	/* empty */
|		parameter
|		parameter COMMA parameter_list
;

parameter:	type ID;
%%

int yywrap() 
{
	return 1;
}

int yyerror(char *s)
{
	fprintf(stderr, "UCC Error: %s at line %d\n", s, linenum);
	exit(1);
}

int main(int argc, char **argv)
{
	yyparse();
	return 0;	
}
