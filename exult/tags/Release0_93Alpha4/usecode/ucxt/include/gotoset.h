
#ifndef GOTOSET_H
#define GOTOSET_H

#include "ucc.h"

class GotoSet_old
{
   public:
     GotoSet_old() : _offset(0) {};
     GotoSet_old(const unsigned int offset, const UCc &ucc)
            : _offset(offset)
     {
       add(ucc);
     };
     GotoSet_old(const UCc &ucc) : _offset(ucc._offset) { add(ucc); };

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
};

#endif


