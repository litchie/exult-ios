
#ifndef NEWOPCODE_H
#define NEWOPCODE_H

#include "opcodec.h"

class GCStack
{
  public:
    GCStack() {};
    ~GCStack()
    {
      for(vector<Opcode *>::iterator i=_gcstack.begin(); i!=_gcstack.end(); i++)
      {
        delete *i;
        *i=NULL; // paranoia
      }
    }
    Opcode *operator()(Opcode *op) { _gcstack.push_back(op); return op; };

  private:
    vector<Opcode *> _gcstack;
};

static GCStack gcstack;

/* returns a new opcode determined by the type of "id" it is, see opcodes.h */
Opcode *NewOpcode(const unsigned int offset, const unsigned int id, const vector<unsigned int> &params)
{
  switch(id)
  {
    case Opcode::PUSH_TRUE:    return gcstack(new PushTrueOpcode(offset, id, params));
    case Opcode::PUSH_FALSE:   return gcstack(new PushFalseOpcode(offset, id, params));
    case Opcode::PUSHI:        return gcstack(new PushIOpcode(offset, id, params));
    case Opcode::PUSH:         return gcstack(new PushOpcode(offset, id, params));
    case Opcode::PUSH_EVENTID: return gcstack(new PushEventIDOpcode(offset, id, params));
    case Opcode::PUSHS:        return gcstack(new PushSOpcode(offset, id, params));
    case Opcode::CMPEQ:        return gcstack(new CmpEQOpcode(offset, id, params));
    case Opcode::JNE:          return gcstack(new JNEOpcode(offset, id, params));
    case Opcode::JMP:          return gcstack(new JmpOpcode(offset, id, params));
    case Opcode::CALLIS:       return gcstack(new CallISOpcode(offset, id, params));
    case Opcode::POP:          return gcstack(new PopOpcode(offset, id, params));
    case Opcode::CMPGT:        return gcstack(new CmpGTOpcode(offset, id, params));
    case Opcode::CMPLT:        return gcstack(new CmpLTOpcode(offset, id, params));
    case Opcode::SUB:          return gcstack(new SubOpcode(offset, id, params));
    case Opcode::ADD:          return gcstack(new AddOpcode(offset, id, params));
    case Opcode::PUSH_ITEMREF: return gcstack(new PushItemRefOpcode(offset, id, params));
    case Opcode::RET:          return gcstack(new RetOpcode(offset, id, params));
    case Opcode::CALLI:        return gcstack(new CallIOpcode(offset, id, params));
    case Opcode::PUSHF:        return gcstack(new PushFOpcode(offset, id, params));
    case Opcode::POPF:         return gcstack(new PopFOpcode(offset, id, params));
    case Opcode::CALL_EXTERN:  return gcstack(new CallOpcode(offset, id, params));
    case Opcode::AND:          return gcstack(new AndOpcode(offset, id, params));
    case Opcode::OR:           return gcstack(new OrOpcode(offset, id, params));
    case Opcode::NOT:          return gcstack(new NotOpcode(offset, id, params));
    case Opcode::ARRC:         return gcstack(new ArrCOpcode(offset, id, params));
    case Opcode::AGET:         return gcstack(new AGetOpcode(offset, id, params));
    case Opcode::ENUM:         return gcstack(new ENumOpcode(offset, id, params));
    case Opcode::NEXT:         return gcstack(new NextOpcode(offset, id, params));
    case Opcode::CMPGE:        return gcstack(new CmpGEOpcode(offset, id, params));
    case Opcode::CMPLE:        return gcstack(new CmpLEOpcode(offset, id, params));
    case Opcode::CMPNE:        return gcstack(new CmpNEOpcode(offset, id, params));
    case Opcode::ARRA:         return gcstack(new ArrAOpcode(offset, id, params));
    case Opcode::POPR:         return gcstack(new PopROpcode(offset, id, params));
    case Opcode::RETR:         return gcstack(new RetROpcode(offset, id, params));
    case Opcode::ASK:          return gcstack(new AskOpcode(offset, id, params));
    case Opcode::JMPA:         return gcstack(new JmpAOpcode(offset, id, params));
    case Opcode::DIV:          return gcstack(new DivOpcode(offset, id, params));
    case Opcode::MUL:          return gcstack(new MulOpcode(offset, id, params));
    case Opcode::MOD:          return gcstack(new ModOpcode(offset, id, params));
    case Opcode::ADDSI:        return gcstack(new AddSIOpcode(offset, id, params));
    case Opcode::EXIT2:        return gcstack(new Exit2Opcode(offset, id, params));
    case Opcode::ADDSV:        return gcstack(new AddSVOpcode(offset, id, params));
    case Opcode::IN:           return gcstack(new InOpcode(offset, id, params));
    case Opcode::SAY:          return gcstack(new SayOpcode(offset, id, params));
    case Opcode::EXIT:         return gcstack(new ExitOpcode(offset, id, params));
    case Opcode::CLA:          return gcstack(new ClAOpcode(offset, id, params));
    case Opcode::PUSHBI:       return gcstack(new PushBIOpcode(offset, id, params));
    case Opcode::APUT:         return gcstack(new APutOpcode(offset, id, params));
    case Opcode::CALLE:        return gcstack(new CallEOpcode(offset, id, params));
    case Opcode::POP_EVENTID:  return gcstack(new PopEventIDOpcode(offset, id, params));
    default:                   return gcstack(new MiscOpcode(offset, id, params));
  }

  // can't happen
  return NULL;
};

#endif

