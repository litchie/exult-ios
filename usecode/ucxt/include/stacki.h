
#ifndef STACKI_H
#define STACKI_H

#include <string>
#include "ucc.h"

#define RETVAL    "retval"
#define VARPREFIX "var"
#define STRREG    "strreg"

enum { STACKI_VAR=1,
       STACKI_CMPGT, STACKI_CMPLT, STACKI_CMPGE,
       STACKI_CMPLE, STACKI_CMPNE, STACKI_CMPEQ,
       STACKI_ADD,   STACKI_SUB,   STACKI_DIV,
       STACKI_MUL,   STACKI_MOD,   STACKI_AND,
       STACKI_OR,
       STACKI_NOT,
       STACKI_ISIN_ARR,
       STACKI_STRING,
       STACKI_ARRA
       };

class StackI
{
  public:
    StackI() : _type(0) {};
    StackI(const unsigned int ucc_type,
           const StackI &s1, const StackI &s2=StackI());
    StackI(const UCc &ucc);

    string str() const;
    string strbrace() const;

  unsigned int _type;
  string _string1;
  string _string2;

  private:
    void process_not(const StackI &s1);
};


#endif

