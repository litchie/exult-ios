#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "printucc.h"

#include <stack>
#include "opcodes.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <strstream>
#include "stacki.h"
#include "gotoset.h"

//#define DEBUG(x) x
#define DEBUG(x)

#define VARNAME "uvar"

unsigned int uc_indentlvl=0;

string ucc_calli_tostr(const UCc &ucc);

bool print_uccs(ostream &o, const vector<UCc> &uccs, const unsigned int &uc_funcid,
                const vector<unsigned short> &uc_externs,
                const map<unsigned int, string> &uc_data);
void print_ucc(ostream &o, const UCc &ucc, const unsigned int &uc_funcid,
               const vector<unsigned short> &uc_externs,
               const map<unsigned int, string> &uc_data, bool &return_val);
void indent(ostream &o);
unsigned int calc_reljump_offset(const UCc &ucc);
string itos(unsigned int i);
void populate_gotoset(vector<GotoSet_old> &gotoset, const vector<UCc> &uccs);

stack<StackI> stackp;

void output_ucfunc(const unsigned int uc_funcid,
                   const map<unsigned int, string, less<unsigned int> > &uc_data,
                   const unsigned int &uc_argc, const unsigned int &uc_localc,
                   const vector<unsigned short> &uc_externs, const vector<UCc> &uc_codes)
  { //PATRICK
    strstream ss_externs;

    ss_externs << setbase(16) << setfill('0');
    ss_externs << "// externs" << endl;

    for(unsigned int i=0; i<uc_externs.size(); i++)
      ss_externs << "void " << extern_tostr(uc_externs[i]) << ";" << endl;
    ss_externs << endl << ends;

    strstream ss_func_head;
    //     << "void";
    ss_func_head << setbase(16) << setfill('0');
    ss_func_head << " Func" << setw(4) << uc_funcid << "(";

    for(unsigned int i=0; i<uc_argc; i++)
    {
      if(i!=0)
        ss_func_head << ", ";
      ss_func_head << VARNAME << " " << VARPREFIX << setw(4) << i;
    }
    ss_func_head << ")" << endl
                 << "{" << endl;
    uc_indentlvl+=2;

    /*for(map<unsigned int, string>::iterator i = uc_data.begin(); i!=uc_data.end(); i++)
    {
      indent(cout);
      cout << "string data" << setw(4) << i->first << " = \"" << i->second << "\";" << endl;
    }
    cout << endl;*/

    if(uc_localc)
    {
      for(unsigned int i=uc_argc; i<(uc_argc+uc_localc); i++)
      {
        indent(ss_func_head);
        ss_func_head << VARNAME << " " << VARPREFIX << setw(4) << i << ";" << endl;
      }
      ss_func_head << endl;
    }
    ss_func_head << ends;

    strstream ss_func_body;
    ss_func_body << setbase(16) << setfill('0');

    bool func_returns_val = print_uccs(ss_func_body, uc_codes, uc_funcid, uc_externs, uc_data);
    ss_func_body << ends;

    cout << ss_externs.str();

    if(func_returns_val)
      cout << VARNAME;
    else
      cout << "void";

    cout << ss_func_head.str();

    if(func_returns_val)
    {
      indent(cout);
      cout << VARNAME << " " << RETVAL << ";" << endl;
      cout << endl;
    }

    cout << ss_func_body.str();
    cout << endl;
    uc_indentlvl-=2;
    cout << "}" << endl << endl;
  }

bool print_uccs(ostream &o, const vector<UCc> &uccs, const unsigned int &uc_funcid,
                const vector<unsigned short> &uc_externs,
                const map<unsigned int, string> &uc_data)
{
  vector<GotoSet_old> uccs_gotoset;

  populate_gotoset(uccs_gotoset, uccs);

  bool return_val=false;

  for(unsigned int i=0; i<uccs_gotoset.size(); i++)
  {
    //o << "//" << uccs[i]._offset << endl;

    /* we don't want to output the first "jump"
       (the start of the function) */
    if(i!=0)
    {
      o << endl;
      uc_indentlvl--;
      indent(o);
      uc_indentlvl++;
      o << setbase(16) << "label" << setw(4) << uc_funcid << "_" << setw(4) << uccs_gotoset[i].offset() << ":" << endl;
    }

    for(unsigned int j=0; j<uccs_gotoset[i].size(); j++)
    {

      const UCc &ucc = uccs_gotoset[i][j];
      const UCOpcodeData &opcode = opcode_table_data[ucc._id];

      //if we've already done this
      if(ucc._tagged!=true)
      {
        if(opcode.name=="NULL")
        {
          indent(o);
          o << "// unknown function = ";
          print_ucc(o, ucc, uc_funcid, uc_externs, uc_data, return_val);
          o << endl;
        }
        else
        {
//          if(opcode.effect & EFF_PUSH)
//            stackp.push(ucc);
//          else if(opcode.effect & EFF_POP)
//            print_ucc(o, ucc, uc_funcid, uc_externs, uc_data, return_val);
//          else if(opcode.effect & EFF_CMP)
//            print_ucc(o, ucc, uc_funcid, uc_externs, uc_data, return_val);
//          else if(opcode.effect & EFF_RELATIVE_JUMP)
//            print_ucc(o, ucc, uc_funcid, uc_externs, uc_data, return_val);
//          else if(opcode.effect & EFF_BIMATH)
//            print_ucc(o, ucc, uc_funcid, uc_externs, uc_data, return_val);
//          else if(opcode.effect & EFF_UNIMATH)
//            print_ucc(o, ucc, uc_funcid, uc_externs, uc_data, return_val);
//          else if(opcode.effect & EFF_SINGLELINE)
//            print_ucc(o, ucc, uc_funcid, uc_externs, uc_data, return_val);
//          else if(opcode.effect & EFF_STUPIDEFF)
//            print_ucc(o, ucc, uc_funcid, uc_externs, uc_data, return_val);
//          else
//          {
//            indent(o);
            print_ucc(o, ucc, uc_funcid, uc_externs, uc_data, return_val);
//            o << endl;
//          }
        }
      }
    } // end for j
  } // end for i

  if(stackp.size())
  {
    o << endl << "  //Stack (Pushed Values) (size=" << stackp.size() << ")" << endl;
    while(!stackp.empty())
    {
      indent(o);
      //print_ucc(o, si);
      o << stackp.top().str() << endl;
      stackp.pop();
    }
  }

  o << endl << "  //Jumps" << endl;
  for(unsigned int i=1; i<uccs_gotoset.size(); i++)
    o << "  //" << setw(4) << uccs_gotoset[i].offset() << endl;

  return return_val;
}

void print_ucc(ostream &o, const UCc &ucc, const unsigned int &uc_funcid,
               const vector<unsigned short> &uc_externs,
               const map<unsigned int, string> &uc_data, bool &return_val)
{
  switch(ucc._id)
  {
		// pushes
		case 0x13:
		case 0x14:
		case 0x1F:
		case 0x21:
		case 0x3E:
		case 0x42:
		case 0x44:
		case 0x48:
			stackp.push(ucc);
			break;

    // comparisons
    case UCC_CMPGT:
    case UCC_CMPLT:
    case UCC_CMPGE:
    case UCC_CMPLE:
    case UCC_CMPNE:
    case UCC_CMPEQ:
    case UCC_IN: // is in array
      assert(stackp.size()>=2);
      {
        StackI second = stackp.top();
        stackp.pop();
        StackI first = stackp.top();
        stackp.pop();

        stackp.push(StackI(ucc._id, first, second));
      }
      break;

    // pops
    case UCC_POP:
    case UCC_POPF:
      assert(stackp.size()>=1);
      assert(ucc._params.size()>=2);
      {
        indent(o);
        StackI var(ucc);
        StackI val = stackp.top();
        stackp.pop();
        o << var.str() << " = " << val.str() << ";" << endl;
      }
      break;

    case UCC_POPR:
      assert(stackp.size()>=1);
      assert(ucc._params.size()==0);
      {
        indent(o);
        StackI val = stackp.top();
        stackp.pop();
        o << RETVAL << " = " << val.str() << ";" << endl;
      }
      break;

    // string register functions
    case UCC_ADDSI:
      assert(ucc._params.size()>=2);
      {
        indent(o);
        string s = uc_data.find((unsigned int)(((unsigned int)(ucc._params[1])<<8)+ucc._params[0]))->second;
        StackI val(UCc(UCC_STRING, s));
        //StackI val(ucc);
        o << STRREG << " += " << val.str() << ";" << endl;
      }
      break;

    case UCC_SAY:
      {
        indent(o);
        o << STRREG << ".display(); // SIDE EFFECT: clears strreg" << endl;
      }
      break;

    case UCC_ADDSV:
      assert(ucc._params.size()>=2);
      {
        indent(o);
        StackI val(ucc);
        o << STRREG << " += " << val.str() << ";" << endl;
      }
      break;

    // jumps
    case UCC_JNE:
      {
        indent(o);
        StackI si_cmp = stackp.top();
        stackp.pop();
        StackI si_not(UCC_NOT, si_cmp);
        o << "if" << si_not.strbrace() << endl;
        uc_indentlvl++;
        indent(o);
        o << setfill('0') << setbase(16) << "goto label" << setw(4) << uc_funcid << "_"
          << setw(4) << calc_reljump_offset(ucc) << ";" << endl;
        uc_indentlvl--;
      }
      break;
    case UCC_JMP:
      assert(ucc._params.size()>=2);
      {
        indent(o);
        o << setfill('0') << setbase(16) << "goto label" << setw(4) << uc_funcid << "_"
          << setw(4) << calc_reljump_offset(ucc) << ";" << endl;
      }
      break;

    // binary math
    case UCC_ADD:
    case UCC_SUB:
    case UCC_DIV:
    case UCC_MUL:
    case UCC_MOD:
    case UCC_AND:
    case UCC_OR:
      assert(stackp.size()>=2);
      {
        StackI second = stackp.top();
        stackp.pop();
        StackI first = stackp.top();
        stackp.pop();

        stackp.push(StackI(ucc._id, first, second));
      }
      break;

    case UCC_PUSHS:
      assert(ucc._params.size()>=2);
      {
        string s = uc_data.find((unsigned int)(((unsigned int)(ucc._params[1])<<8)+ucc._params[0]))->second;
        stackp.push(StackI(UCc(UCC_STRING, s)));
      }
      break;

    // uniary math
    case UCC_NOT:
      assert(stackp.size()>=1);
      {
        StackI first = stackp.top();
        stackp.pop();

        stackp.push(StackI(ucc._id, first));
      }
      break;

    // calls
    case UCC_CALLIS:
      {
        stackp.push(StackI(UCc(UCC_FUNC, ucc_calli_tostr(ucc))));
      }
      break;

    case UCC_CALLI:
      {
        indent(o);
        StackI si(UCc(UCC_FUNC, ucc_calli_tostr(ucc)));
        o << si.str() << ";" << endl;
      }
      break;

    case UCC_CALL:
      assert(ucc._params.size()>=2);
      {
        //indent(o);
        //o << extern_tostr(uc_externs[(ucc._params[1]<<8) + ucc._params[0]]) << endl;
        stackp.push(StackI(UCc(UCC_FUNC, extern_tostr(uc_externs[(ucc._params[1]<<8) + ucc._params[0]]))));
        //o << StackI(UCc(UCC_FUNC, extern_tostr(uc_externs[(ucc._params[1]<<8) + ucc._params[0]]))).str() << ";" << endl;
      }
      break;

    // returning
    case UCC_RET:
    case UCC_EXIT2:
    case UCC_EXIT:
      {
        StackI si(ucc);
        indent(o);
        o << si.str() << ";" << endl;
      }
      break;

    case UCC_RETR:
      {
        return_val=true;
        StackI si(ucc);
        indent(o);
        o << si.str() << ";" << endl;
      }
      break;

    // array maniuplation functions
    case UCC_ARRC:
      {
        unsigned int count = (ucc._params[1]<<8) + ucc._params[0];
        assert(stackp.size()>=count);
        strstream ss;
        ss << "{ ";
        for(unsigned int i=0; i<count; i++)
        {
          if(i!=0) ss << ", ";
          StackI si = stackp.top();
          stackp.pop();
          ss << si.str();
        }
        ss << " }" << ends;
        stackp.push(StackI(UCc(UCC_ARR, ss.str())));
      }
      break;

    case UCC_AGET:
      assert(stackp.size()>=1);
      assert(ucc._params.size()>=2);
      {
        StackI val = stackp.top();
        stackp.pop();
        strstream ss;
        ss << setfill('0') << setbase(16) << VARPREFIX
           << setw(2) << ucc._params[1] << setw(2) << ucc._params[0]
           << "[" << val.str() << "]" << ends;
        stackp.push(StackI(UCc(UCC_ARR, ss.str())));
      }
      break;
    case UCC_ARRA:
      assert(stackp.size()>=2);
      {
        StackI val = stackp.top();
        stackp.pop();
        StackI arr = stackp.top();
        stackp.pop();
        stackp.push(StackI(UCC_ARRA, arr, val));

      }
      break;
  /* Pops the value from the stack and adds it to array on the top of stack */
  /*  or pops both values from the stack, builds an array from them */
  /*  & pushes it on the stack if neither of them are arrays */

/*    case UCC_APUT:
      assert(stackp.size()>=2);
      {

      }*/
  /* Uses the top-of-stack value to index (1-based) the array variable, (top-of-stack - 1)  as
    the new value, and updates a value in the array slot (local variable specified
    in the operand) */
      break;
    // misc "temp" funcs
/*    case UCC_MATHCAT:
      o << ucc._miscstr;
      break;*/

    default:
      indent(o);
      o << setfill('0') << "/*" << setw(4) << ucc._offset << " "
        << setw(2) << ucc._id;
      for(unsigned int j=0; j<ucc._params.size(); j++)
        o << " " << setw(2) << ucc._params[j];
      o << "<" << ucc._miscstr << ">*/" << endl;
  }

}

void indent(ostream &o)
{
  for(unsigned int i=0; i<uc_indentlvl; i++)
    o << "  ";
}

unsigned int calc_reljump_offset(const UCc &ucc)
{
  return ucc._offset + 1 + ucc._params.size()
        + ucc._params[0] + (ucc._params[1] << 8);
}

extern UCData uc;

string ucc_calli_tostr(const UCc &ucc)
{
  strstream ss;
  assert(ucc._params.size()>=3);

  ss << setfill('0') << setbase(16);
  unsigned int func_no = (((unsigned int)ucc._params[1])<<8) + ucc._params[0];

  // if there is no "real" func name we make one up
	// WARNING: uber-assert in progress
	assert((uc.game()==GAME_BG) ? (func_no<bg_uc_intrinsics.size()) :
	       ((uc.game()==GAME_SI) ? (func_no<si_uc_intrinsics.size()) : false));

	string name;
  if(uc.game()==GAME_BG)
	  name = bg_uc_intrinsics.find(func_no)->second;
  if(uc.game()==GAME_SI)
	  name = si_uc_intrinsics.find(func_no)->second;
	
	if(name=="UNKNOWN") // if it's unknown we dump out a fake name
    ss << "IFunc" << setw(4) << func_no;
  else                // else we dump out the real one
    ss << name;

  ss << "(";

  for(unsigned int i=0; i<ucc._params[2]; i++)
  {
    if(i!=0)
      ss << ", ";
    StackI si = stackp.top();
    stackp.pop();
    ss << si.str();
  }
  ss << ")" << ends;
  return ss.str();
}

string extern_tostr(const unsigned int uc_extern)
{
  strstream ss;
  ss << setfill('0') << setbase(16) << "Func" << setw(4) << uc_extern << "()" << ends;
  return ss.str();
}

string itos(unsigned int i)
{
  strstream ss;
  ss << setfill('0') << setw(4) << i << ends;
  return ss.str();
}

/* FIXME: This is not going to work because of the lack of implemented .effect
          in the new UCOpcodeData
          Fixed the lack of .effect... now there obviously is other problems... */
void populate_gotoset(vector<GotoSet_old> &gotoset, const vector<UCc> &uccs)
{
  vector<unsigned int> jumps;

  // collect jump references
  for(unsigned int i=0; i<uccs.size(); i++)
  {
    //if(opcode_table_data[uccs[i]._id].effect & EFF_RELATIVE_JUMP)
		if(!uccs[i]._jump_offsets.empty())
    {
      //assert(uccs[i]._params.size()>=2);
      jumps.insert(jumps.end(), uccs[i]._jump_offsets.begin(), uccs[i]._jump_offsets.end());
    }
  }

  gotoset.push_back(GotoSet_old());

  for(unsigned int i=0; i<uccs.size(); i++)
  {
    //cout << "//" << uccs[i]._offset << endl;

    if(count(jumps.begin(), jumps.end(), uccs[i]._offset))
    {
      gotoset.push_back(uccs[i]);
      //cout << endl;
      //uc_indentlvl--;
      //indent(cout);
      //uc_indentlvl++;
      //cout << setbase(16) << "label" << setw(4) << uc_funcid << "_" << setw(4) << uccs[i]._offset << ":" << endl;
    }
    else
      gotoset.back().add(uccs[i]);

    //DEBUG({
    //  cout << ">>" << stackp.top()._id << endl;
    //})
  }
}

