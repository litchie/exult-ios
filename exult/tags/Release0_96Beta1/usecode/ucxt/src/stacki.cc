#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "stacki.h"
#include <strstream>
#include "opcodes.h"
#include <iomanip>

StackI::StackI(const UCc &ucc)
{
  strstream ss;
  switch(ucc._id)
  {
    case UCC_ADDSV:
    case UCC_PUSH:
    case UCC_POP:
      assert(ucc._params.size()>=2);
      _type=STACKI_VAR;
      ss << setfill('0') << setbase(16) << VARPREFIX << setw(2) << ucc._params[1] << setw(2) << ucc._params[0] << ends;
      break;

    case UCC_PUSHI:
      assert(ucc._params.size()>=2);
      _type=STACKI_VAR;
      ss << setbase(10) << (int)(ucc._params[0] + (((int)((char)ucc._params[1]))<<8)) << setbase(16) << ends;
      break;

    case UCC_PUSHBI:
      assert(ucc._params.size()>=1);
      _type=STACKI_VAR;
      ss << setbase(10) << (unsigned int)(ucc._params[0]) << setbase(16) << ends;
      break;

    case UCC_PUSHF:
    case UCC_POPF:
      assert(ucc._params.size()>=2);
      _type=STACKI_VAR;
      ss << "flag[" << (unsigned int)((ucc._params[1]<<8)+ ucc._params[0]) << "]" << ends;
      break;

    case UCC_PUSH_EVENTID:
      _type=STACKI_VAR;
      ss << "eventid" << ends;
      break;
    case UCC_PUSH_ITEMREF:
      _type=STACKI_VAR;
      ss << "itemref" << ends;
      break;
    case UCC_PUSH_TRUE:
      _type=STACKI_VAR;
      ss << "true" << ends;
      break;
    case UCC_PUSH_FALSE:
      _type=STACKI_VAR;
      ss << "false" << ends;
      break;


    case UCC_RET:
      _type = STACKI_VAR;
      ss << "return" << ends;
      break;

    case UCC_RETR:
      _type = STACKI_VAR;
      ss << "return " << RETVAL << ends;
      break;

    case UCC_EXIT2:
      _type = STACKI_VAR;
      ss << "exit2" << ends;
      break;

    case UCC_EXIT:
      _type = STACKI_VAR;
      ss << "exit" << ends;
      break;

    case UCC_FUNC:
    case UCC_ARR:
      _type = STACKI_VAR;
      ss << ucc._miscstr << ends;
      break;

    case UCC_STRING:
      _type = STACKI_STRING;
      for(unsigned int i=0; i<ucc._miscstr.size(); i++)
      {
        if(ucc._miscstr[i]=='\"')
          ss << "\\\"";
        else
          ss << ucc._miscstr[i];
      }
      ss << ends;
      break;

    default:
      cout << "U: Pushed the unpushable.(" << ucc._id << ")" << endl;
      cerr << "U: Pushed the unpushable.(" << ucc._id << ")" << endl;
      exit(2);
  }
  _string1 = ss.str();
  _string2 = "";
}

StackI::StackI(const unsigned int ucc_type,
               const StackI &s1, const StackI &s2)
{
  switch(ucc_type)
  {
    case UCC_CMPGT:
      _type = STACKI_CMPGT;
      _string1 = s1.str();
      _string2 = s2.str();
      break;
    case UCC_CMPLT:
      _type = STACKI_CMPLT;
      _string1 = s1.str();
      _string2 = s2.str();
      break;
    case UCC_CMPGE:
      _type = STACKI_CMPGE;
      _string1 = s1.str();
      _string2 = s2.str();
      break;
    case UCC_CMPLE:
      _type = STACKI_CMPLE;
      _string1 = s1.str();
      _string2 = s2.str();
      break;
    case UCC_CMPNE:
      _type = STACKI_CMPNE;
      _string1 = s1.str();
      _string2 = s2.str();
      break;
    case UCC_CMPEQ:
      _type = STACKI_CMPEQ;
      _string1 = s1.str();
      _string2 = s2.str();
      break;
    case UCC_ADD:
      _type = STACKI_ADD;
      _string1 = s1.str();
      _string2 = s2.str();
      break;
    case UCC_SUB:
      _type = STACKI_SUB;
      _string1 = s1.str();
      _string2 = s2.str();
      break;
    case UCC_DIV:
      _type = STACKI_DIV;
      _string1 = s1.str();
      _string2 = s2.str();
      break;
    case UCC_MUL:
      _type = STACKI_MUL;
      _string1 = s1.str();
      _string2 = s2.str();
      break;
    case UCC_MOD:
      _type = STACKI_MOD;
      _string1 = s1.str();
      _string2 = s2.str();
      break;
    case UCC_AND:
      _type = STACKI_AND;
      _string1 = s1.str();
      _string2 = s2.str();
      break;
    case UCC_OR:
      _type = STACKI_OR;
      _string1 = s1.str();
      _string2 = s2.str();
      break;
    case UCC_NOT:
      process_not(s1);
      break;
    case UCC_IN:
      _type = STACKI_ISIN_ARR;
      _string1 = s1.str();
      _string2 = s2.str();
      break;
    case UCC_ARRA:
      _type = STACKI_ARRA;
      _string1 = s1.str();
      _string2 = s2.str();
      break;
    default:
      cout << "iSS: Pushed the unpushable." << endl;
      cerr << "iSS: Pushed the unpushable." << endl;
      exit(3);
  }

}

string StackI::str() const
{
  string tmpstr;

  switch(_type)
  {
    case STACKI_VAR:
      return _string1;
      break;
    case STACKI_CMPGT:
      tmpstr = " > ";
      return string("(") + _string1 + tmpstr + _string2 + string(")");
      break;
    case STACKI_CMPLT:
      tmpstr = " < ";
      return string("(") + _string1 + tmpstr + _string2 + string(")");
      break;
    case STACKI_CMPGE:
      tmpstr = " >= ";
      return string("(") + _string1 + tmpstr + _string2 + string(")");
      break;
    case STACKI_CMPLE:
      tmpstr = " <= ";
      return string("(") + _string1 + tmpstr + _string2 + string(")");
      break;
    case STACKI_CMPNE:
      tmpstr = " != ";
      return string("(") + _string1 + tmpstr + _string2 + string(")");
      break;
    case STACKI_CMPEQ:
      tmpstr = " == ";
      return string("(") + _string1 + tmpstr + _string2 + string(")");
      break;
    case STACKI_ADD:
      tmpstr = " + ";
      return /*string("(") +*/ _string1 + tmpstr + _string2 /*+ string(")")*/;
      break;
    case STACKI_SUB:
      tmpstr = " - ";
      return string("(") + _string1 + tmpstr + _string2 + string(")");
      break;
    case STACKI_DIV:
      tmpstr = " / ";
      return string("(") + _string1 + tmpstr + _string2 + string(")");
      break;
    case STACKI_MUL:
      tmpstr = " * ";
      return string("(") + _string1 + tmpstr + _string2 + string(")");
      break;
    case STACKI_MOD:
      tmpstr = " % ";
      return string("(") + _string1 + tmpstr + _string2 + string(")");
      break;
    case STACKI_AND:
      tmpstr = " && ";
      return string("(") + _string1 + tmpstr + _string2 + string(")");
      break;
    case STACKI_OR:
      tmpstr = " || ";
      return string("(") + _string1 + tmpstr + _string2 + string(")");
      break;
    case STACKI_NOT:
      return string("(!") + _string1 + string(")");
      break;
    case STACKI_ISIN_ARR:
      return string ("xisiny(") + _string1 + string(", ") + _string2 + string(")");
      break;
    case STACKI_STRING:
      return string("\"") + _string1 + string("\"");
      break;
    case STACKI_ARRA:
      return _string1 + string(".append(") + _string2 + string(")");
      break;
    default:
      cout << "HUH?!?" << endl;
      exit(1);
  }
}

string StackI::strbrace() const
{
  string s = str();
  if(s[0]!='(')
    s = string("(") + s;
  if(s[s.size()-1]!=')')
    s+=")";
  return s;
}

void StackI::process_not(const StackI &s1)
{
  switch(s1._type)
  {
    case STACKI_CMPGT:
      _type = STACKI_CMPLE;
      _string1 = s1._string1;
      _string2 = s1._string2;
      break;
    case STACKI_CMPLT:
      _type = STACKI_CMPGE;
      _string1 = s1._string1;
      _string2 = s1._string2;
      break;
    case STACKI_CMPGE:
      _type = STACKI_CMPLT;
      _string1 = s1._string1;
      _string2 = s1._string2;
      break;
    case STACKI_CMPLE:
      _type = STACKI_CMPGT;
      _string1 = s1._string1;
      _string2 = s1._string2;
      break;
    case STACKI_CMPNE:
      _type = STACKI_CMPEQ;
      _string1 = s1._string1;
      _string2 = s1._string2;
      break;
    case STACKI_CMPEQ:
      _type = STACKI_CMPNE;
      _string1 = s1._string1;
      _string2 = s1._string2;
      break;
    case STACKI_AND:
      _type = STACKI_OR;
      _string1 = s1._string1;
      _string2 = s1._string2;
      break;
    case STACKI_OR:
      _type = STACKI_AND;
      _string1 = s1._string1;
      _string2 = s1._string2;
      break;
    case STACKI_NOT:
      _type = STACKI_VAR;
      _string1 = s1._string1;
      break;
    default:
      _type = STACKI_NOT;
      _string1 = s1.str();
      break;
  }

}

