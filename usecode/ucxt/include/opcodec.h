
#ifndef OPCODEC_H
#define OPCODEC_H

#include <iosfwd>
#include <vector>
#include <iomanip>
#include "opcodec_.h"
#include "opcodecn.h"

class MiscOpcode : public Opcode
{
  public:
    MiscOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
          : Opcode(offset, id, params), _params(params) {};
    virtual ~MiscOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ":" << SPACER << setw(2) << _id << " -";
      for(unsigned int i=0; i<_params.size(); i++)
        o << " " << setw(2) << _params[i];
      o << endl;
    };
    virtual void print(ostream &o)
    {
      indent(o);
      o << "// ";
      print_asm(o);
    };
    virtual bool IsUnknown() const { return true; };

  private:
    vector<unsigned int> _params;
};

class NextOpcode : public Opcode
{
  public:
    NextOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==10);
      _counter     = (((unsigned int)params[1])<<8) + (unsigned int)params[0];
      _total_count = (((unsigned int)params[3])<<8) + (unsigned int)params[2];
      _current_val = (((unsigned int)params[5])<<8) + (unsigned int)params[4];
      _arrayvar    = (((unsigned int)params[7])<<8) + (unsigned int)params[6];
      _jumpto      = (((unsigned int)params[9])<<8) + (unsigned int)params[8];
      _jumpto = calc_rel_offset(offset, params.size(), _jumpto);

    };
    virtual ~NextOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << endl
        << SPACER << SPACER << SPACER << "next" << SPACER
          << "[" << setw(4) << _counter << "], "
          << "[" << setw(4) << _total_count << "], "
          << "[" << setw(4) << _current_val << "], "
          << "[" << setw(4) << _arrayvar << "], "
          << setw(4) << _jumpto << endl;
    };

  private:
    unsigned int _counter;
    unsigned int _total_count;
    unsigned int _current_val;
    unsigned int _arrayvar;
    unsigned int _jumpto;
};

class AskOpcode : public Opcode
{
  public:
    AskOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==2);
      _jumpto = (((unsigned int)params[1])<<8) + (unsigned int)params[0];
      _jumpto = calc_rel_offset(offset, params.size(), _jumpto);
    };
    virtual ~AskOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "ask" << SPACER << setw(4) << _jumpto << endl;
    };

  private:
    unsigned int _jumpto;

};

class JNEOpcode : public OpcodeGenericJump
{
  public:
    JNEOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : OpcodeGenericJump(offset, id, params)
    {
      assert(params.size()==2);
      _target_offset = calc_rel_offset(offset, params.size(), (int)(params[0] + (((signed char)params[1])<<8)));

    };
    virtual ~JNEOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "jne" << SPACER << setw(4) << (unsigned short)_target_offset << endl;
    };

    virtual bool IsaRelativeJump() const { return true; };

  private:
};

class JmpOpcode : public OpcodeGenericJump
{
  public:
    JmpOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : OpcodeGenericJump(offset, id, params)
    {
      assert(params.size()==2);
      _target_offset = calc_rel_offset(offset, params.size(), (int)(params[0] + (((signed char)params[1])<<8)));

    };
    virtual ~JmpOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "jmp" << SPACER << setw(4) << (unsigned short)_target_offset << endl;
    };

    virtual bool IsaRelativeJump() const { return true; };

  private:
};

class JmpAOpcode : public Opcode
{
  public:
    JmpAOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==4);
      _one    = (((unsigned int)params[1])<<8) + (unsigned int)params[0];
      _jumpto = (((unsigned int)params[3])<<8) + (unsigned int)params[2];
      _jumpto = calc_rel_offset(offset, params.size(), _jumpto);
    };
    virtual ~JmpAOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << "jmpa" << SPACER << setw(4) << _one << "H," << SPACER << setw(4) << _jumpto << SPACER << "; " << setbase(10) << _one << setbase(16) << endl;
    };

  private:
    unsigned int _one;
    unsigned int _jumpto;
};

class AddOpcode : public Opcode
{
  public:
    AddOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~AddOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "add" << endl;
    };
};

class SubOpcode : public Opcode
{
  public:
    SubOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~SubOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "sub" << endl;
    };
};

class DivOpcode : public Opcode
{
  public:
    DivOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~DivOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "div" << endl;
    };
};

class MulOpcode : public Opcode
{
  public:
    MulOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~MulOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "mul" << endl;
    };
};

class ModOpcode : public Opcode
{
  public:
    ModOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~ModOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "mod" << endl;
    };
};

class AndOpcode : public Opcode
{
  public:
    AndOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~AndOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "and" << endl;
    };
};

class OrOpcode : public Opcode
{
  public:
    OrOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~OrOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "or" << endl;
    };
};

class NotOpcode : public Opcode
{
  public:
    NotOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~NotOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "not" << endl;
    };
};

class PopOpcode : public Opcode
{
  public:
    PopOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==2);
      _variable = (((unsigned int)params[1])<<8) + (unsigned int)params[0];
    };
    virtual ~PopOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "pop" << SPACER << "[" << setw(4) << _variable << "]" << endl;
    };
    virtual bool I_Pop() const { return true; };

  private:
    unsigned int _variable;

};

class PushTrueOpcode : public Opcode
{
  public:
    PushTrueOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~PushTrueOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "push" << SPACER << "true" << endl;
    };
    virtual bool I_Push() const { return true; };
};

class PushFalseOpcode : public Opcode
{
  public:
    PushFalseOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~PushFalseOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "push" << SPACER << "false" << endl;
    };
    virtual bool I_Push() const { return true; };
};

class CmpGTOpcode : public Opcode
{
  public:
    CmpGTOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~CmpGTOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "cmpgt" << endl;
    };
};

class CmpLTOpcode : public Opcode
{
  public:
    CmpLTOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~CmpLTOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "cmplt" << endl;
    };
};

class CmpGEOpcode : public Opcode
{
  public:
    CmpGEOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~CmpGEOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "cmpge" << endl;
    };
};

class CmpLEOpcode : public Opcode
{
  public:
    CmpLEOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~CmpLEOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "cmple" << endl;
    };
};

class CmpNEOpcode : public Opcode
{
  public:
    CmpNEOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~CmpNEOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "cmpne" << endl;
    };
};

class AddSIOpcode : public Opcode
{
  public:
    AddSIOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==2);
      _value = (int)(params[0] + (((int)((char)params[1]))<<8));
    };
    virtual ~AddSIOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "addsi" << SPACER << setw(4) << (unsigned short)_value << "H" << SPACER << SPACER << SPACER << "; " << "*** STRING GOES HERE ***" << endl;
    };
    virtual bool I_Push() const { return true; };

  private:
    int _value;
};

class PushSOpcode : public Opcode
{
  public:
    PushSOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==2);
      _value = (int)(params[0] + (((int)((char)params[1]))<<8));
    };
    virtual ~PushSOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "pushs" << SPACER << setw(4) << (unsigned short)_value << "H" << SPACER << SPACER << SPACER << "; " << "*** STRING GOES HERE ***" << endl;
    };
    virtual bool I_Push() const { return true; };

  private:
    int _value;
};

class ArrCOpcode : public Opcode
{
  public:
    ArrCOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==2);
      _numvalues = (((unsigned int)params[1])<<8) + (unsigned int)params[0];
    };
    virtual ~ArrCOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "arrc" << SPACER << setw(4) << _numvalues << "H" << SPACER << SPACER << SPACER << "; " << setbase(10) << _numvalues << setbase(16) << endl;
    };

  private:
    unsigned int _numvalues;
};

class PushIOpcode : public Opcode
{
  public:
    PushIOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==2);
      _value = (int)(params[0] + (((int)((char)params[1]))<<8));
    };
    virtual ~PushIOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "pushi" << SPACER << setw(4) << (unsigned short)_value << "H" << SPACER << "; " << setbase(10) << _value << setbase(16) << endl;
    };
    virtual bool I_Push() const { return true; };

  private:
    int _value;
};

class PushOpcode : public Opcode
{
  public:
    PushOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==2);
      _variable = (((unsigned int)params[1])<<8) + (unsigned int)params[0];
    };
    virtual ~PushOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "push" << SPACER << "[" << setw(4) << _variable << "]" << endl;
    };
    virtual bool I_Push() const { return true; };

  private:
    unsigned int _variable;

};

class CmpEQOpcode : public Opcode
{
  public:
    CmpEQOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~CmpEQOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "cmpeq" << endl;
    };
};

class CallOpcode : public Opcode
{
  public:
    CallOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==2);
      _externref = (((unsigned int)params[1])<<8) + (unsigned int)params[0];
    };
    virtual ~CallOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "call" << SPACER << "extern:[" << setw(4) << _externref << "]" << SPACER << SPACER << "; *** EXTERN NAME ***" << endl;
    };

  private:
    unsigned int _externref;
};

class RetOpcode : public Opcode
{
  public:
    RetOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~RetOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "ret" << endl;
    };
};

class AGetOpcode : public Opcode
{
  public:
    AGetOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==2);
      _arrayvar = (((unsigned int)params[1])<<8) + (unsigned int)params[0];
    };
    virtual ~AGetOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "aget" << SPACER << "[" << setw(4) << _arrayvar << "]" << endl;
    };

  private:
    unsigned int _arrayvar;
};

class Exit2Opcode : public Opcode
{
  public:
    Exit2Opcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~Exit2Opcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "exit2" << endl;
    };
};

class PopROpcode : public Opcode
{
  public:
    PopROpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~PopROpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "popr" << endl;
    };
};

class ENumOpcode : public Opcode
{
  public:
    ENumOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~ENumOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "enum" << endl;
    };
};

class AddSVOpcode : public Opcode
{
  public:
    AddSVOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==2);
      _stringref = (((unsigned int)params[1])<<8) + (unsigned int)params[0];
    };
    virtual ~AddSVOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "addsv" << SPACER << "[" << setw(4) << _stringref << "]" << endl;
    };
    virtual bool I_Push() const { return true; };

  private:
    unsigned int _stringref;

};

class InOpcode : public Opcode
{
  public:
    InOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~InOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "in" << endl;
    };
};

class RetROpcode : public Opcode
{
  public:
    RetROpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~RetROpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "retr" << endl;
    };
};

class SayOpcode : public Opcode
{
  public:
    SayOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~SayOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "say" << endl;
    };
};

class CallISOpcode : public Opcode
{
  public:
    CallISOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==3);
      _function = (((unsigned int)params[1])<<8) + (unsigned int)params[0];
      _num_params = params[2];

    };
    virtual ~CallISOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << "callis" << SPACER << setw(4) << (unsigned short)_function << ", " << _num_params << endl;
    };

  private:
    unsigned int _function;
    unsigned int _num_params;
};

class CallIOpcode : public Opcode
{
  public:
    CallIOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==3);
      _function = (((unsigned int)params[1])<<8) + (unsigned int)params[0];
      _num_params = params[2];

    };
    virtual ~CallIOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "calli" << SPACER << setw(4) << (unsigned short)_function << ", " << _num_params << endl;
    };

  private:
    unsigned int _function;
    unsigned int _num_params;
};

class PushItemRefOpcode : public Opcode
{
  public:
    PushItemRefOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~PushItemRefOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "push" << SPACER << "itemref" << endl;
    };
    virtual bool I_Push() const { return true; };
};

class ExitOpcode : public Opcode
{
  public:
    ExitOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~ExitOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "exit" << endl;
    };
};

class ClAOpcode : public Opcode
{
  public:
    ClAOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~ClAOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "cla" << endl;
    };
};

class PushFOpcode : public OpcodeGenericFlag
{
  public:
    PushFOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : OpcodeGenericFlag(offset, id, params)
    {
      assert(params.size()==2);
      _flag = (int)(params[0] + (((int)((char)params[1]))<<8));
    };
    virtual ~PushFOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "pushf" << SPACER << setw(4) << (unsigned short)_flag << "H" << SPACER << "; " << setbase(10) << _flag << setbase(16) << endl;
    };
    virtual bool I_Push() const { return true; };
};

class PopFOpcode : public OpcodeGenericFlag
{
  public:
    PopFOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : OpcodeGenericFlag(offset, id, params)
    {
      assert(params.size()==2);
      _flag = (int)(params[0] + (((int)((char)params[1]))<<8));
    };
    virtual ~PopFOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "popf" << SPACER << setw(4) << (unsigned short)_flag << "H" << SPACER << "; " << setbase(10) << _flag << setbase(16) << endl;
    };
    virtual bool I_Pop() const { return true; };
};

class PushBIOpcode : public Opcode
{
  public:
    PushBIOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==1);
      _value = (unsigned int)(params[0]);
    };
    virtual ~PushBIOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "pushbi" << SPACER << setw(2) << (unsigned short)_value << "H" << SPACER << SPACER << SPACER << "; " << setbase(10) << _value << setbase(16) << endl;
    };
    virtual bool I_Push() const { return true; };

  private:
    unsigned int _value;
};

class APutOpcode : public Opcode
{
  public:
    APutOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==2);
      _variable = (((unsigned int)params[1])<<8) + (unsigned int)params[0];
    };
    virtual ~APutOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "aput" << SPACER << "[" << setw(4) << _variable << "]" << endl;
    };
    virtual bool I_Pop() const { return true; };

  private:
    unsigned int _variable;

};

class CallEOpcode : public Opcode
{
  public:
    CallEOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==2);
      _externref = (((unsigned int)params[1])<<8) + (unsigned int)params[0];
    };
    virtual ~CallEOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "calle" << SPACER << "extern:[" << setw(4) << _externref << "]" << endl;
    };

  private:
    unsigned int _externref;
};

class PushEventIDOpcode : public Opcode
{
  public:
    PushEventIDOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~PushEventIDOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "push" << SPACER << "eventid" << endl;
    };
    virtual bool I_Push() const { return true; };
};

class ArrAOpcode : public Opcode
{
  public:
    ArrAOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~ArrAOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "arra" << endl;
    };
};

class PopEventIDOpcode : public Opcode
{
  public:
    PopEventIDOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
      assert(params.size()==0);
    };
    virtual ~PopEventIDOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ": " << *this << SPACER << SPACER << "pop" << SPACER << "eventid" << endl;
    };
    virtual bool I_Pop() const { return true; };
};











#endif

