
#ifndef OPCODEC__H
#define OPCODEC__H

#include <string>
#include <vector>
#include <iomanip>
#include <map>

inline unsigned int calc_rel_offset(const unsigned int offset,
                                    const unsigned int params_size,
                                    const int reloffset)
{
      // relative offset calculation =
      //   1           (size of id)
      // + offset      (offset from start of procedure)
      // + params_size (number of bytes until end of function)
      // + reloffset
      return 1 + offset + params_size + reloffset;
}

class Opcode
{
  public:
    Opcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
          : _offset(offset), _id(id), _params(params) {};
    virtual ~Opcode() {};
    virtual void print_asm(ostream &o)=0;
    virtual void print_c(ostream &o)
    {
      indent(o);
      o << "// ";
      print_asm(o);
    };
    virtual void print(ostream &o) { print_asm(o); };
    virtual void pass1(const vector<unsigned int> &externs) { externs[0]; };
    virtual void pass2(const map<unsigned int, string, less<unsigned int> > &data) { data.find(0); };
    friend ostream &operator<<(ostream &o, const Opcode &op);

    unsigned int offset() const { return _offset; };

    enum
    {
      UNKNOWN_00   = 0x00,
      UNKNOWN_01   = 0x01,

      NEXT         = 0x02,

      UNKNOWN_03   = 0x03,

      ASK          = 0x04,
      JNE          = 0x05,
      JMP          = 0x06,
      JMPA         = 0x07,

      UNKNOWN_08   = 0x08,

      ADD          = 0x09,
      SUB          = 0x0A,
      DIV          = 0x0B,
      MUL          = 0x0C,
      MOD          = 0x0D,
      AND          = 0x0E,
      OR           = 0x0F,
      NOT          = 0x10,

      UNKNOWN_11   = 0x11,

      POP          = 0x12,
      PUSH_TRUE    = 0x13,
      PUSH_FALSE   = 0x14,

      UNKNOWN_15   = 0x15,

      CMPGT        = 0x16,
      CMPLT        = 0x17,
      CMPGE        = 0x18,
      CMPLE        = 0x19,
      CMPNE        = 0x1A,

      UNKNOWN_1B   = 0x1B,

      ADDSI        = 0x1C,
      PUSHS        = 0x1D,
      ARRC         = 0x1E,
      PUSHI        = 0x1F,

      UNKNOWN_20   = 0x20,

      PUSH         = 0x21,
      CMPEQ        = 0x22,

      UNKNOWN_23   = 0x23,

      CALL_EXTERN  = 0x24,
      RET          = 0x25,
      AGET         = 0x26,

      UNKNOWN_27   = 0x27,
      UNKNOWN_28   = 0x28,
      UNKNOWN_29   = 0x29,
      UNKNOWN_2A   = 0x2A,
      UNKNOWN_2B   = 0x2B,

      EXIT2        = 0x2C,
      POPR         = 0x2D,
      ENUM         = 0x2E,
      ADDSV        = 0x2F,
      IN           = 0x30,

      UNKNOWN_31   = 0x31,
      RETR         = 0x32,
      SAY          = 0x33,

      UNKNOWN_34   = 0x34,
      UNKNOWN_35   = 0x35,
      UNKNOWN_36   = 0x36,
      UNKNOWN_37   = 0x37,

      CALLIS       = 0x38,
      CALLI        = 0x39,

      UNKNOWN_3A   = 0x3A,
      UNKNOWN_3B   = 0x3B,
      UNKNOWN_3C   = 0x3C,
      UNKNOWN_3D   = 0x3D,

      PUSH_ITEMREF = 0x3E,
      EXIT         = 0x3F,
      CLA          = 0x40,

      UNKNOWN_41   = 0x41,

      PUSHF        = 0x42,
      POPF         = 0x43,
      PUSHBI       = 0x44,

      UNKNOWN_45   = 0x45,

      APUT         = 0x46,
      CALLE        = 0x47,
      PUSH_EVENTID = 0x48,

      UNKNOWN_49   = 0x49,

      ARRA         = 0x4A,
      POP_EVENTID  = 0x4B,

      OPEND
    };

    // Iama's (I am a...) always false unless overridden
    virtual bool IsaRelativeJump() const { return false; };
    virtual bool IsaFlag() const { return false; };
    virtual bool IsUnknown() const { return false; };
    // I's are always false unless overridden
    virtual bool I_Pop() const { return false; };
    virtual bool I_Push() const { return false; };

    // amusing... ;)
//    static const char * const SPACER = " ";
    static const char * const SPACER;

    void indent_inc() { indent_level++; };
    void indent_dec() { indent_level--; };
    void indent(ostream &o)
    {
      for(unsigned int i=0; i<indent_level; i++)
        o << "  ";
    };

  protected:

    unsigned int _offset;
    unsigned int _id;

  private:
    static unsigned int indent_level;
    vector<unsigned int> _params;

};

//const char * const Opcode::SPACER = "\t";

inline ostream &operator<<(ostream &o, const Opcode &op)
{
  o << setw(2) << op._id;

  for(unsigned int i=0; i<op._params.size(); i++)
    o << ' ' << setw(2) << op._params[i];

  return o;
}

class OpcodeGenericJump : public Opcode
{
  public:
    OpcodeGenericJump(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
    };
    virtual ~OpcodeGenericJump() {};

    unsigned int target_offset() const { return _target_offset; };

  protected:
    unsigned int _target_offset;
};

class OpcodeGenericFlag : public Opcode
{
  public:
    OpcodeGenericFlag(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id, params)
    {
    };
    virtual ~OpcodeGenericFlag() {};
    virtual bool IsaFlag() const { return true; };

    unsigned int flag() const { return _flag; };

  protected:
    unsigned int _flag;
};

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

#endif

