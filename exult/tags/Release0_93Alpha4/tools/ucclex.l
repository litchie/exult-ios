%{
int linenum;
%}

NUMBER		[0-9]+
IDENT		[A-Za-z][A-Za-z0-9]*
BLANK		[ \t\r]
NL		\n
%%
{BLANK}		;
"//"[^\n]	;
void		{ return VOID; }
uvar		{ return UVAR; }
"="		{ return EQUALS; }
"+"		{ return PLUS; }
"-"		{ return MINUS; }
"{"		{ return OBRACE; }
"}"		{ return CBRACE; }
"("		{ return OBRACKET; }
")"		{ return CBRACKET; }
","		{ return COMMA; }
if		{ return IF; }
then		{ return THEN; }
else		{ return ELSE; }
{NL}		{ linenum++; }
{IDENT}		{ return ID; }
%%
