
#ifndef UCC_H
#define UCC_H

#include <vector>
#include <string>

class UCc
{
  public:
  UCc(const unsigned int offset=0, const unsigned int id=0, const vector<unsigned char> &params=vector<unsigned char>())
     : _id(id), _offset(offset), _params(params), _tagged(false) {};
  UCc(const unsigned int id, const string &miscstr)
     : _id(id), _miscstr(miscstr) {};

  unsigned int          _id;
  string                _miscstr;
  unsigned int          _offset;
  vector<unsigned char> _params;
	vector<unsigned int>  _params_parsed;
  bool                  _tagged;
	vector<unsigned int>  _jump_offsets;

};

#endif

