%{
/**
 **	Uclex.ll - Usecode lexical scanner.
 **
 **	Written: 12/30/2000 - JSF
 **/

/*
Copyright (C) 2000 The Exult Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <string>
#include <vector>
#include "ucparse.h"
#include "ucloc.h"

using std::string;

extern std::vector<char *> include_dirs;	// -I directories.

/*
 *	Want a stack of files for implementing '#include'.
 */
std::vector<Uc_location *> locstack;
std::vector<YY_BUFFER_STATE> bufstack;

/*
 *	Parse out a name in quotes.
 *
 *	Output:	->name, with a null replacing the ending quote.
 *		0 if not found.
 */

static char *Find_name
	(
	char *name,
	char *& ename			// ->null at end of name returned.
	)
	{
	while (*name && *name != '"')	// Find start of filename.
		name++;
	if (!*name)
		return 0;
	name++;				// Point to name.
	ename = name;			// Find end.
	while (*ename && *ename != '"')
		ename++;
	if (!*ename)
		return 0;
	*ename = 0;
	return name;
	}

/*
 *	Set location from a preprocessor string.
 */

static void Set_location
	(
	char *text			// ->first digit of line #.
	)
	{
	char *name;
	int line = strtol(text, &name, 10);
	char *ename;
	name = Find_name(name, ename);
	if (!name)
		return;
//cout << "Setting location at line " << line - 1 << endl;
					// We're 0-based.
	Uc_location::set_cur(name, line - 1);
	*name = '"';			// Restore text.
	}

/*
 *	Include another source.
 */

static void Include
	(
	char *yytext			// ->text containing name.
	)
	{
	char msg[180];
	if (bufstack.size() > 20)
		{
		Uc_location::yyerror("#includes are nested too deeply");
		exit(1);
		}
	char *ename;
	char *name = Find_name(yytext, ename);
	if (!name)
		{
		Uc_location::yyerror("No file in #include");
		return;
		}
	locstack.push_back(new Uc_location());
	bufstack.push_back(YY_CURRENT_BUFFER);
	yyin = fopen(name, "r");
					// Look in -I list if not found here.
	for (std::vector<char *>::const_iterator it = include_dirs.begin();
				!yyin && it != include_dirs.end(); ++it)
		{
		string path(*it);
		path += '/';
		path += name;
		yyin = fopen(path.c_str(), "r");
		}
	if (!yyin)
		{
		sprintf(msg, "Can't open '%s'", name);
		Uc_location::yyerror(msg);
		exit(1);
		}
					// Set location to new file.
	Uc_location::set_cur(name, 0);
	yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
	}


extern "C" int yywrap() { return 1; }		/* Stop at EOF. */

%}

%x comment

%%

if		return IF;
else		return ELSE;
return		return RETURN;
while		return WHILE;
for		return FOR;
in		return IN;
with		return WITH;
to		return TO;
var		return VAR;
int		return INT;
const		return CONST;
string		return STRING;
extern		return EXTERN;
true		return UCTRUE;
false		return UCFALSE;
break		return BREAK;
case		return CASE;

converse	return CONVERSE;
say		return SAY;
add		return ADD;
hide		return HIDE;
message		return MESSAGE;
response	return RESPONSE;
script		return SCRIPT;
after		return AFTER;
ticks		return TICKS;
hours		return HOURS;
event		return EVENT;
gflags		return FLAG;
item		return ITEM;
goto		return GOTO;

					/* Script commands. */
continue	return CONTINUE;
repeat		return REPEAT;
nop		return NOP;
nohalt		return NOHALT;
wait		return WAIT;
remove		return REMOVE;
rise		return RISE;
descent		return DESCEND;
frame		return FRAME;
hatch		return HATCH;
next		return NEXT;
previous	return PREVIOUS;
cycle		return CYCLE;
step		return STEP;
music		return MUSIC;
call		return CALL;
speech		return SPEECH;
sfx		return SFX;
face		return FACE;
hit		return HIT;
actor		return ACTOR;

north		return NORTH;
south		return SOUTH;
east		return EAST;
west		return WEST;
nw		return NW;
ne		return NE;
sw		return SW;
se		return SE;

"&&"		return AND;
"||"		return OR;
"!"		return NOT;

[a-zA-Z][a-zA-Z0-9_]*	{
			yylval.strval = strdup(yytext);
			return IDENTIFIER;
			}
\"[^"]*\"		{
					// Remove ending quote.
			yylval.strval = strdup(yytext + 1);
			yylval.strval[strlen(yylval.strval) - 1] = 0;
			return STRING_LITERAL;
			}
\"[^"]*\"\*		{
					// Remove ending quote and asterisk.
			yylval.strval = strdup(yytext + 1);
			yylval.strval[strlen(yylval.strval) - 2] = 0;
			return STRING_PREFIX;
			}
[0-9]+			{
			yylval.intval = atoi(yytext);
			return INT_LITERAL;
			}
0x[0-9a-fA-F]+		{
			yylval.intval = strtol(yytext + 2, 0, 16);
			return INT_LITERAL;
			}

"=="			{ return EQUALS; }
"!="			{ return NEQUALS; }
"<="			{ return LTEQUALS; }
">="			{ return GTEQUALS; }
"->"			{ return POINTS; }

"# "[0-9]+\ \"[^"]*\".*\n	{ Set_location(yytext + 2); }
"#line "[0-9]+\ \"[^"]*\".*\n	{ Set_location(yytext + 6); }
"#include"[ \t]+.*\n		{ Include(yytext + 8); }

\#.*			/* Ignore other cpp directives. */

[ \t\r]+					/* Ignore spaces. */
"//".*						/* Comments. */
"/*"			BEGIN(comment);
<comment>[^*\n]*				/* All but '*'. */
<comment>"*"+[^*/\n]*				/* *'s not followed by '/'. */
<comment>\n		{ Uc_location::increment_cur_line(); }
<comment>"*/"		BEGIN(INITIAL);
<comment><<EOF>>	{ Uc_location::yyerror("Comment not terminated");
			yyterminate(); }
\n			{ Uc_location::increment_cur_line(); }
.			return *yytext;		/* Being lazy. */
<<EOF>>			{
			if (locstack.empty())
				yyterminate();
			else		// Restore buffer and location.
				{
				Uc_location *loc = locstack.back();
				locstack.pop_back();
				const char *nm = loc->get_source();
				loc->set_cur(nm, loc->get_line());
				delete loc;
				yy_delete_buffer(YY_CURRENT_BUFFER);
				yy_switch_to_buffer(bufstack.back());
				bufstack.pop_back();
				}
			}


%%

