#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include "opcodes.h"
#include <cstdlib>
#include <iomanip>
#include <fstream>
#include <stack>

/* Opcode table - common to BG & SI */
const opcode_desc opcode_table[] =
{
  { NULL, 0, 0 },           /* 00 */
  { NULL, 0, 0 },           /* 01 */
  /* Next iteration of For Each-style loop
    Operands are 1,2, <current>, <array variable>, <jump>
    <current> is set to next value from <array variable>
    Jumps to <jump> if array ended
    ??? what are 1,2? Are they used?
  */
  { "next", 10, EFF_RELATIVE_JUMP },         /* 02 */
  { NULL, 0, 0 },           /* 03 */
  /* Asks user to select one the talk answers
    Jumps where specified if no answer available
  */
  { "ask", 2, EFF_RELATIVE_JUMP },        /* 04 */
  /* Pops a value from the top of stack, jump if false, zero or empty array */
  { "jne", 2, EFF_RELATIVE_JUMP },        /* 05 */
  /* jump */
  { "jmp", 2, EFF_RELATIVE_JUMP },        /* 06 */
  /* Pops the top-of-stack string & jumps if the string is NOT the last talk answer 
    ??? first operand seems to be always 1
  */
  { "jmpa", 4, EFF_RELATIVE_JUMP },      /* 07 */
  { NULL, 0, 0 },           /* 08 */
  /* Adds two values on the stack, popping them & pushing result
    Can be used to add integer to string - in this case integer is converted to string 
    and 2 strings are concatenated */
  { "add", 0, EFF_BIMATH },          /* 09 */
  /* Decrements second value of the stack by first value, popping them & pushing result */
  { "sub", 0, EFF_BIMATH },          /* 0a */
  /* Divides second value of the stack by first value, popping them & pushing result */
  { "div", 0, EFF_BIMATH },          /* 0b */
  /* Multiplies two values on the stack, popping them & pushing result */
  { "mul", 0, EFF_BIMATH },          /* 0c */
  /* Divides second value of the stack by first value, popping them & pushing reminder */
  { "mod", 0, EFF_BIMATH },          /* 0d */
  /* Boolean AND on two values on stack, popping them & pushing result
    Top-of-stack variable is the left one in case of string addition */
  { "and", 0, EFF_BIMATH },          /* 0e */
  /* Boolean OR on two values on stack, popping them & pushing result */
  { "or", 0, EFF_BIMATH },           /* 0f */
  /* Inverts a boolean value on top of stack */
  { "not", 0, EFF_UNIMATH },          /* 10 */
  { NULL, 0, 0 },           /* 11 */
  /* Pops a stack value to given local variable */
  { "pop", 2, EFF_POP },         /* 12 */
  /* Pushes a TRUE boolean value on the stack */
  { "push\ttrue", 0, EFF_PUSH },         /* 13 */
  /* Pushes a FALSE boolean value on the stack */
  { "push\tfalse", 0, EFF_PUSH },        /* 14 */
  { NULL, 0, 0 },           /* 15 */
  /* Pops 2 values from the stack, pushes boolean value -
    TRUE if second value "?" then first 
    (where "?" can be greater, greater or equal etc...)
  */
  { "cmpgt", 0, EFF_CMP },          /* 16 */
  { "cmplt", 0, EFF_CMP },          /* 17 */
  { "cmpge", 0, EFF_CMP },          /* 18 */
  { "cmple", 0, EFF_CMP },          /* 19 */
  { "cmpne", 0, EFF_CMP },          /* 1a */
  { NULL, 0, 0 },         /* 1b */
  /* Adds a string from data segment to string register current contents */
  { "addsi", 2, EFF_STUPIDEFF },        /* 1c */
  /* Pushes a string value given by 16bit string offset in data segment to stack */
  { "pushs", 2, EFF_STUPIDEFF },        /* 1d */
  /* Pops specified number of values from stack, builds an array from them
    & pushes it on the stack 
    Pushes the empty array to top of the stack if operand is 0
  */
  { "arrc", 2, EFF_STUPIDEFF },         /* 1e */
  /* Pushes immediate 16bit integer to stack */
  { "pushi", 2, EFF_PUSH },          /* 1f */
  { NULL, 0, 0 },           /* 20 */
  /* Pushes a local variable on stack */
  { "push", 2, EFF_PUSH },          /* 21 */
  /* Compares 2 values on the stack, pops them & pushes TRUE if they are equal */
  { "cmpeq", 0, EFF_CMP },          /* 22 */
  { NULL, 0, 0 },           /* 23 */
  /* Calls a usecode function - function number is 0-based index to externs array */
  { "call", 2, EFF_SINGLELINE },          /* 24 */
  /* Return from function without result returned on the stack */
  { "ret", 0, EFF_SINGLELINE },          /* 25 */
  /* Uses the top-of-stack value to index (1-based) the array variable, 
    pops index & pushes result - the value from the array */
  { "aget", 2, EFF_SINGLELINE },          /* 26 */
  { NULL, 0, 0 },           /* 27 */
  { NULL, 0, 0 },           /* 28 */
  { NULL, 0, 0 },           /* 29 */
  { NULL, 0, 0 },           /* 2a */
  { NULL, 0, 0 },           /* 2b */
  /* ??? Looks like to be the same as "exit"
    ??? Suggestion: maybe "exit" is for functions, while "exit2" is for event handlers? */
  { "exit2", 0, EFF_EXIT | EFF_SINGLELINE },          /* 2c */
  /* Pop the top-of-stack value & sets it as a return value */
  { "popr", 0, EFF_STUPIDEFF },         /* 2d */
  /* Opens a new For Each-style enumeration loop.
    Always followed by "next" opcode??? */
  { "enum", 0, 0 },           /* 2e */
  /* Adds local variable's string value to string register current contents */
  { "addsv", 2, EFF_STUPIDEFF },         /* 2f */
  /* If (top-of-stack - 1) value is in top-of-stack array, pushes true, 
    otherwise pushes false - after popping array & value */
  { "in", 0, EFF_STUPIDEFF },           /* 30 */
  /* Something strange with 2 varrefs (???) as operands
    Appears in BG only - in talk to Raymundo in the Theatre - trying to sing a song
    I -am- the Avatar (function: 08D1H)
  */
  { "???", 4, 0 },            /* 31 */
  /* Return with a result returned on the stack */
  { "retr", 0, EFF_STUPIDEFF },         /* 32 */
  /* Displays the string register value (to current talk, sign, scroll or book),
    string register emptied */
  { "say", 0, EFF_STUPIDEFF },          /* 33 */
  { NULL, 0, 0 },           /* 34 */
  { NULL, 0, 0 },           /* 35 */
  { NULL, 0, 0 },           /* 36 */
  { NULL, 0, 0 },           /* 37 */
  /* Calls engine's intrinsic function with specified number of parameters 
    (popping them). The return value remains on stack */
  { "callis", 3, EFF_STUPIDEFF },          /* 38 */
  /* Calls engine's intrinsic function with specified number of parameters 
    (popping them). No return value  */
  { "calli", 3, EFF_SINGLELINE },         /* 39 */
  { NULL, 0, 0 },           /* 3a */
  { NULL, 0, 0 },           /* 3b */
  { NULL, 0, 0 },           /* 3c */
  { NULL, 0, 0 },           /* 3d */
  /* Pushes identifier of the item ( for which the usecode event handler is called ) 
    on the stack */
  { "push\titemref", 0, EFF_PUSH },        /* 3e */
  /* Aborts the function & all usecode execution, returning to the engine */
  { "exit", 0, EFF_SINGLELINE },         /* 3f */
  /* Removes all answers from the current talk */
  { "cla", 0, 0 },            /* 40 */
  { NULL, 0, 0 },           /* 41 */
  /* Pushes game flag's value (boolean) on the stack */
  { "pushf", 2, EFF_PUSH },         /* 42 */
  /* Pops the stack value to the game flag (boolean) */
  { "popf", 2, EFF_POP },          /* 43 */
  /* Pushes an immediate byte to the stack */
  { "pushbi", 1, EFF_PUSH },        /* 44 */
  { NULL, 0, 0 },           /* 45 */
  /* Uses the top-of-stack value to index (1-based) the array variable, (top-of-stack - 1)  as
    the new value, and updates a value in the array slot (local variable specified
    in the operand) */
  { "aput", 2, EFF_ARRAY | EFF_SINGLELINE },          /* 46 */
  /* Call of usecode function - function # is the operand
    ???Suggestion: functions are divided into event handlers (called by external code
    and the usecode) and functions (called by usecode only). "calle" is used to
    call the event handler from the usecode, while "call" is used to call a function
  */
  { "calle", 2, 0 },          /* 47 */
  /* Pushes the cause of usecode event handler call on the stack */
  /*  (double-click on item is be 1, NPC death seems to be 7 in SI and 2 in BG ) */
  { "push\teventid", 0, EFF_PUSH },        /* 48 */
  { NULL, 0, 0 },           /* 49 */
  /* Pops the value from the stack and adds it to array on the top of stack */
  /*  or pops both values from the stack, builds an array from them */
  /*  & pushes it on the stack if neither of them are arrays */
  { "arra", 0, EFF_STUPIDEFF },         /* 4a */
  /* Pops the top-of-stack value & sets the cause of usecode event handler to it
    (double-click on item is be 1, NPC death seems to be 7 in SI and 2 in BG )
    Used to call event handlers from the usecode
  */
  { "pop\teventid", 0, 0 },         /* 4b */
};


vector<UCOpcodeData> opcode_table_data(MAX_NO_OPCODES);

map<unsigned int, string> bg_uc_intrinsics;
map<unsigned int, string> si_uc_intrinsics;

vector<string> str2vec(const string &s);

/* constructs the static usecode tables from other include files in the /exult hierachy,
   static by compilation.
*/
void init_static_usecodetables(const Configuration &config)
{
	#define	USECODE_INTRINSIC_PTR(NAME)	string(__STRING(NAME))
	string bgut[] = 
	{
	#include "bgintrinsics.h"
	};
	string siut[] =
	{
	#include "siintrinsics.h"
	};
	#undef USECODE_INTRINSIC_PTR
	
	for(unsigned int i=0; i<0x100; i++)
		bg_uc_intrinsics.insert(pair<unsigned int, string>(bg_uc_intrinsics.size(), bgut[i]));
	
	for(unsigned int i=0; i<0x100; i++)
		si_uc_intrinsics.insert(pair<unsigned int, string>(si_uc_intrinsics.size(), siut[i]));
}

/* constructs the usecode tables from datafiles in the /ucxt hierachy */
void init_usecodetables(const Configuration &config, const UCData &uc)
{
	string ucxtroot;
	if(uc.noconf() == false) config.value("config/ucxt/root", ucxtroot);
  if(uc.verbose()) cout << "ucxtroot: " << ucxtroot << endl;
  if(ucxtroot.size() && ucxtroot[ucxtroot.size()-1]!='/' && ucxtroot[ucxtroot.size()-1]!='\\') ucxtroot+='/';
  ucxtroot+= "Docs/opcodes.txt";

	ifstream file;

	file.open(ucxtroot.c_str(), ios::in);
	
	if(file.fail())
	{
		cout << "error. could not locate " << ucxtroot << ". exiting." << endl;
		exit(1);
	}
	
	string s;
	while(!file.eof())
	{
		getline(file, s);
		if(s.size() && s[0]=='>')
		{
			UCOpcodeData uco(str2vec(s));
			assert(uco.opcode<MAX_NO_OPCODES);
			opcode_table_data[uco.opcode] = uco;
		}	
	}
	#if 0 //test
	for(vector<UCOpcodeData>::iterator i=opcode_table_data.begin(); i!=opcode_table_data.end(); i++)
		if(i->asm_nmo!="" || i->ucs_nmo!="")
			cout << i->opcode << "\t" << i->asm_nmo << "\t" << i->ucs_nmo << "\t" << i->num_bytes << "\t" << i->param_types << endl;
	#endif ///test
}

/* To be depricated when I get the complex vector<string> splitter online */
vector<string> qnd_ocsplit(const string &s)
{
  assert((s[0]=='{') && (s[s.size()-1]=='}'));

	vector<string> vs;
  string tstr;

	for(string::const_iterator i=s.begin(); i!=s.end(); ++i)
	{
    if(*i==',')
		{
			vs.push_back(tstr);
			tstr="";
		}
		else if(*i=='{' || *i=='}')
		{ /* nothing */ }
		else
			tstr+=*i;
	}
	if(tstr.size())
		vs.push_back(tstr);

	return vs;
}

vector<string> str2vec(const string &s)
{
	vector<string> vs;
	unsigned int lasti=0;

	// if it's empty return null
	if(s.size()==0) return vs;

	bool indquote=false;
	for(unsigned int i=0; i<s.size(); i++)
	{
		if(s[i]=='"')
			indquote = !indquote;
		else if(isspace(s[i]) && (!indquote))
		{
			if(lasti!=i)
			{
				if((s[lasti]=='"') && (s[i-1]=='"'))
				{
					if((lasti+1)!=(lasti-1))
						vs.push_back(s.substr(lasti+1, i-lasti-2));
				}
				else
					vs.push_back(s.substr(lasti, i-lasti));
			}

			lasti=i+1;
		}
		if(i==s.size()-1)
			if(lasti!=i)
			{
				if((s[lasti]=='"') && (s[i]=='"'))
				{
					if((lasti+1)!=(lasti-1))
						vs.push_back(s.substr(lasti+1, i-lasti-2));
				}
				else
					vs.push_back(s.substr(lasti, i-lasti+1));
		}
	}

	#if 0 //test
	for(unsigned int i=0; i<vs.size(); i++)
		cout << "\t\"" << vs[i] << "\"" << endl;
	#endif ///test

	return vs;
}

/*vector<string> str2vec(const string &s)
{
	vector<string> vs; // the resulting strings
	stack<char> vbound; // the "bounding" chars used to deonte collections of characters
	unsigned int lasti=0;
  string currstr; // the current string, gets appended to vs

	// if it's empty return null
	if(s.size()==0) return vs;

	for(unsigned int i=0; i<s.size(); i++)
	{
		bool pushback=false; // do we push the currstr onto the vector now?
		char c = s[i];
		switch(c)
		{*/
			// let's start with the openings...
			/* the general pricipal, since we strip the outermost enclosures,
			   is to only append the "bounding" characters if they're NOT the
			   outer most.
			   NOTE: A subtle exception is the boundaries on the outermost set of
			   bounding chars has the same effect as isspace(), YHBW */
/*			case '{':  if(vs.size()) currstr+=c; vbound.push('}');  break;
			//case '[': if(vs.size()) currstr+=c; vbound.push(']'); break;
			//case '(': if(vs.size()) currstr+=c; vbound.push(')'); break;
			//case '<': if(vs.size()) currstr+=c; vbound.push('>'); break;

			// now the closures...
			case '}':
				if(vbound.top()=='}') vbound.pop();
				if(vbound.size()==0)  pushback=true;
				else                  currstr+=c;
				break;
			//case ']':
			//	break;
			//case ')':
			//	break;
			//case '>':
			//	break;

			// now the ones that have the pretentiousness of being both
			// opening and closing causes
			case '\"': if(vs.size()) currstr+=c; vbound.push('\"'); break;
			case '\'': if(vs.size()) currstr+=c; vbound.push('\''); break;
			case '\"':
				if(vbound.top()=='\"')    vbound.pop();
				else                   vbound.push('\"');
				if(vbound.size()==0) pushback=true;
				else                   currstr+=c;
				break;
			case '\'':
				if(vbound.top()=='\'') vbound.pop();
				if(vbound.size()==0)   pushback=true;
				else                   currstr+=c;
				break;
			
			// not to emulate isspace();
			case ' ':  // ze space
			case '\f': // form-feed
			case '\n': // newline
			case '\r': // carriage return
			case '\t': // horizontal tab
			case '\v': // vertical tab
				pushback=true;
				break;
		}

		if(pushback)
		{
			if(currstr.size())
				vs.push_back(currstr);
			currstr="";
		}
	}

	#if 1 //test
	for(unsigned int i=0; i<vs.size(); i++)
		cout << "\t\"" << vs[i] << "\"" << endl;
	#endif ///test

	return vs;
}*/
