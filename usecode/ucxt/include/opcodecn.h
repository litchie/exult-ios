
#ifndef OPCODECN_H
#define OPCODECN_H

#include "opcodec_.h"

class Label
{
  public:
    Label(const unsigned int offset) : _offset(offset) { };
    vector<Opcode *> &operator()() { return _opcodes; };
    void print_asm(ostream &o)
    {
      for(unsigned int i=0; i<_opcodes.size(); i++)
        _opcodes[i]->print_asm(o);
    };

  private:
    unsigned int _offset;
    vector<Opcode *> _opcodes;
};

/*class LabelOpcode : public Opcode
{
  public:
    LabelOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
              : Opcode(offset, id)
    {
      assert(params.size()==0);
    };
    virtual ~LabelOpcode() {};
    virtual void print_asm(ostream &o)
    {
      o << setw(4) << _offset << ":" << SPACER << "pushs" << SPACER << setw(4) << (unsigned short)_value << "H" << SPACER << "; " << "*** STRING GOES HERE ***" << endl;
    };

    // different
    void addOpcode(Opcode *opcode)
    {
       assert(opcode!=NULL);
       _opcode.push_back(opcode);
    };

  private:
    vector<Opcode *> _opcodes;

};*/


/*class LabelOpcode
{
   public:
     LabelOpcode() : _offset(0) {};
     LabelOpcode(const unsigned int offset, const UCc &ucc)
            : _offset(offset)
     {
       add(ucc);
     };
     LabelOpcode(const UCc &ucc) : _offset(ucc._offset) { add(ucc); };

     UCc &operator[](const unsigned int i) { return _uccs[i]; };
     unsigned int size() const { return _uccs.size(); };

     void add(const UCc &ucc)
     {
       _uccs.push_back(ucc);
       //_uccs.back()._offset=0;
     };

     unsigned int offset() const { return _offset; };

   private:
     unsigned int _offset;
     vector<UCc> _uccs;
};*/



#endif

