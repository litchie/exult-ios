#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include "opcodes.h"
#include "files/utils.h"
#include <cstdlib>
#include <iomanip>
#include <fstream>
#include <stack>

#ifndef __STRING
	#if defined __STDC__ && __STDC__
		#define __STRING(x) #x
	#else
		#define __STRING(x) "x"
	#endif
#endif

#define MAX_NO_OPCODES 512
vector<UCOpcodeData> opcode_table_data(MAX_NO_OPCODES);
vector<pair<unsigned int, unsigned int> > opcode_jumps;

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
void init_usecodetables(const Configuration &config, bool noconf, bool verbose)
{
	string ucxtroot;
	// just to handle if people are going to compile with makefile.unix, unsupported, but occasionally useful
	#ifdef HAVE_CONFIG_H
	if(noconf == false) config.value("config/ucxt/root", ucxtroot, EXULT_DATADIR);
	#else
	if(noconf == false) config.value("config/ucxt/root", ucxtroot, "data/");
	#endif
	
	if(verbose) cout << "ucxtroot: " << ucxtroot << endl;
	if(ucxtroot.size() && ucxtroot[ucxtroot.size()-1]!='/' && ucxtroot[ucxtroot.size()-1]!='\\') ucxtroot+='/';
	ucxtroot+= "opcodes.txt";

	ifstream file;

	U7open(file, ucxtroot.c_str(), true);
	
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
	file.close();
	
	/* Create an {opcode, parameter_index} array of all opcodes that
		execute a 'jump' statement */
	for(vector<UCOpcodeData>::iterator op=opcode_table_data.begin(); op!=opcode_table_data.end(); op++)
	{
		for(unsigned int i=0; i<op->param_sizes.size(); i++)
		{
			if(op->param_sizes[i].second==true) // this is a calculated offset
			{
				opcode_jumps.push_back(pair<unsigned int, unsigned int>(op->opcode, i+1)); // parameters are stored as base 1
			}
		}
	}
	
	#if 0
	cout << "Calculated Opcode pairs:" << endl;
	for(vector<pair<unsigned int, unsigned int> >::iterator i=opcode_jumps.begin(); i!=opcode_jumps.end(); i++)
		cout << setw(4) << i->first << '\t' << setw(4) << i->second << endl;
	#endif
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

void map_type_size(const vector<string> &param_types, vector<pair<unsigned int, bool> > &param_sizes)
{
	for(vector<string>::const_iterator s=param_types.begin(); s!=param_types.end(); ++s)
	{
		unsigned int ssize=0;
		bool offset_munge=false;
		// all these are two bytes
		if(*s=="short")           ssize=2;
		else if(*s=="flag")       ssize=2;
		else if(*s=="extoffset")  ssize=2;
		else if(*s=="dataoffset") ssize=2;
		else if(*s=="varoffset")  ssize=2;
		else if(*s=="offset")     { ssize=2; offset_munge=true; }
		else if(*s=="long")       ssize=4;
		// and the single one byte type
		else if(*s=="byte")       ssize=1;
		else
		{
			cout << "error: data type '" << *s << "' is not defined. exiting." << endl;
			exit(1);
		}
		assert(ssize!=0);
		param_sizes.push_back(pair<unsigned int, bool>(ssize, offset_munge));
	}
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
